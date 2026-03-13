#include <jni.h>
#include <string>
#include <android/log.h>
#include <cstdio>
#include "zygisk.hpp"
#include "il2cpp_dump.h"

#define LOG_TAG "Il2CppManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 静态变量存储目标包名
static std::string target_package = "";

extern "C" JNIEXPORT void JNICALL
Java_com_unpacker_helper_DumpUtils_setTargetAndDump(JNIEnv *env, jclass clazz, jstring packageName) {
    const char *pkg = env->GetStringUTFChars(packageName, nullptr);
    target_package = pkg;
    
    LOGI("已设置目标应用: %s，准备执行 Dump 逻辑", pkg);
    
    // 执行 il2cpp dump 核心逻辑
    il2cpp_dump(); 
    
    env->ReleaseStringUTFChars(packageName, pkg);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // Zygisk 注入时可以获取当前进程名进行校验
        LOGI("模块已注入进程空间");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
