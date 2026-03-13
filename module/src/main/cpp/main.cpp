#include <jni.h>
#include <android/log.h>
#include <cstdio>
#include <string>
#include "zygisk.hpp"
#include "il2cpp_dump.h"

#define LOG_TAG "Il2CppManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_unpacker_helper_DumpUtils_nativeTriggerDump(JNIEnv *env, jclass clazz, jstring save_path) {
    const char *path = env->GetStringUTFChars(save_path, nullptr);
    if (path != nullptr) {
        LOGI("API 触发：准备脱壳到目录 %s", path);
        // 传入路径参数，解决之前的编译错误
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
    void postAppSpecialize(const zygisk::AppSpecializeArgs*) override {
        LOGI("模块已加载到目标进程");
    }
private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
