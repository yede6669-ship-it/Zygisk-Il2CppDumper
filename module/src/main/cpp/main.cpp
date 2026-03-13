#include <jni.h>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <android/log.h> // 必须包含此头文件以解决 ANDROID_LOG_INFO 未定义问题

#include "zygisk.hpp"
#include "il2cpp_dump.h"

#define LOG_TAG "IL2CPP_WATCHER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 信号文件路径
#define SIGNAL_PATH "/data/local/tmp/dump.json"

void perform_timed_dump(const char* current_pkg) {
    FILE* f = fopen(SIGNAL_PATH, "r");
    if (!f) return;

    char buffer[512]; // 适当增大缓冲区防止溢出
    if (fgets(buffer, sizeof(buffer), f)) {
        char target_pkg[256] = {0};
        long signal_time = 0;
        
        // 匹配 JSON 格式: {"pkg":"xxx","time":123456789}
        if (sscanf(buffer, "{\"pkg\":\"%[^\"]\",\"time\":%ld}", target_pkg, &signal_time) == 2) {
            if (std::string(target_pkg) == current_pkg) {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                long now = tv.tv_sec;

                // 15 秒时效逻辑
                if (now - signal_time > 15) {
                    LOGI("信号已过期（距指令发出已过去 %ld 秒），放弃 Dump", now - signal_time);
                } else {
                    LOGI("信号有效，开始执行脱壳...");
                    // 注意：请确保你的 il2cpp_dump 函数接收保存路径作为参数
                    il2cpp_dump("/sdcard/Download");
                }
                // 处理完毕后清理信号文件
                unlink(SIGNAL_PATH);
            }
        }
    }
    fclose(f);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        if (proc) {
            perform_timed_dump(proc);
            env->ReleaseStringUTFChars(args->nice_name, proc);
        }
    }
private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
