#include <jni.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>
#include "zygisk.hpp"
#include "dobby.h"

#define LOG_TAG "DexUnpacker"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 备份原函数地址
void (*old_OpenMemory)(uint8_t* base, size_t size, void* location, void* location_checksum, void* oat_dex_file, void* container, void* error_msg);

// 脱壳回调函数
void new_OpenMemory(uint8_t* base, size_t size, void* location, void* location_checksum, void* oat_dex_file, void* container, void* error_msg) {
    if (base != nullptr && size > 0x30) {
        // 检测 Dex 魔数
        if (memcmp(base, "dex\n035", 7) == 0) {
            char path[128];
            // 导出到公用目录，文件名包含内存大小
            sprintf(path, "/data/local/tmp/dump_%zu.dex", size);
            int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd > 0) {
                write(fd, base, size);
                close(fd);
                LOGI("Dump Success: %s, Size: %zu", path, size);
            }
        }
    }
    // 回调原函数
    old_OpenMemory(base, size, location, location_checksum, oat_dex_file, container, error_msg);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs*) override {
        // Android 10-12 通用 OpenMemory 符号
        const char* sym = "_ZN3art7DexFile10OpenMemoryEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_10MemMapPtrEPKNS_10OatDexFileEPS9_";
        
        // 使用 Dobby 查找符号地址
        void* target = DobbySymbolResolver("libart.so", sym);
        
        if (target) {
            // 执行 Hook
            DobbyHook(target, (void*)new_OpenMemory, (void**)&old_OpenMemory);
            LOGI("Dobby Hook Installed Successfully.");
        } else {
            LOGI("Symbol Not Found. Check your Android Version.");
        }
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
