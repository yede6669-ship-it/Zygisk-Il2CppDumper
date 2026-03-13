#include <jni.h>
#include <android/log.h>
#include <cstdio>
#include "zygisk.hpp"
#include "il2cpp_dump.h" // 引用你项目中原有的脱壳头文件

#define LOG_TAG "Il2CppDumper-API"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ----------------------------------------------------------------
// JNI 导出函数
// 对应 Java 类: com.unpacker.helper.DumpUtils
// 方法名: nativeTriggerDump
// ----------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL
Java_com_unpacker_helper_DumpUtils_nativeTriggerDump(JNIEnv *env, jclass clazz, jstring save_path) {
    const char *path = env->GetStringUTFChars(save_path, nullptr);
    
    LOGI("API Triggered: Starting dump to %s", path);
    
    // 调用项目中原有的 il2cpp 脱壳核心逻辑
    // 假设你的 il2cpp_dump 支持传入路径，如果不支持，可不传参数
    il2cpp_dump(); 
    
    LOGI("Dump Task Finished.");
    
    env->ReleaseStringUTFChars(save_path, path);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    // 这里留空，不再自动触发，等待 API 调用
    void postAppSpecialize(const zygisk::AppSpecializeArgs*) override {
        LOGI("Module injected, waiting for API call...");
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
