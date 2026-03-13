#include <jni.h>
#include <android/log.h>
#include <string>
#include "zygisk.hpp"
#include "il2cpp_dump.h" // 仅保留 il2cpp 相关的头文件

#define LOG_TAG "IL2CPP_DUMPER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_unpacker_helper_DumpUtils_nativeTriggerDump(JNIEnv *env, jclass clazz, jstring save_path) {
    if (save_path == nullptr) return;

    const char *path = env->GetStringUTFChars(save_path, nullptr);
    if (path != nullptr) {
        LOGI("收到指令：开始 IL2CPP Dump，保存至 %s", path);
        
        // 调用核心 il2cpp 提取逻辑
        // 这个函数会寻找 libil2cpp.so 并导出 metadata 和 dummy dll
        il2cpp_dump(path); 
        
        env->ReleaseStringUTFChars(save_path, path);
    }
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }
    // postAppSpecialize 保持空逻辑，避免自动触发
};

REGISTER_ZYGISK_MODULE(MyModule)
