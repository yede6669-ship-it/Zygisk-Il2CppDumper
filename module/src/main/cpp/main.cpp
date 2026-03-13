#include <jni.h>
#include <android/log.h>
#include <cstdio>
#include <string>
#include "zygisk.hpp"
#include "il2cpp_dump.h"

#define LOG_TAG "IL2CPP_DUMPER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_unpacker_helper_DumpUtils_nativeTriggerDump(JNIEnv *env, jclass clazz, jstring save_path) {
    if (save_path == nullptr) return;

    const char *path = env->GetStringUTFChars(save_path, nullptr);
    if (path != nullptr) {
        LOGI("收到指令：开始 IL2CPP Dump，保存至 %s", path);
        
        // 调用核心 il2cpp 提取逻辑
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

private:
    // 之前报错是因为少了这两行声明
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
