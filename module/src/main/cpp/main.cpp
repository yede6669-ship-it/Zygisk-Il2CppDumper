#include <jni.h>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "zygisk.hpp"
#include "il2cpp_dump.h"

#define LOG_TAG "IL2CPP_WATCHER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 信号文件路径
#define SIGNAL_PATH "/data/local/tmp/dump.json"

void perform_timed_dump(const char* current_pkg) {
    FILE* f = fopen(SIGNAL_PATH, "r");
    if (!f) return;

    char buffer[256];
    fgets(buffer, sizeof(buffer), f);
    fclose(f);

    // 极简 JSON 解析：提取 pkg 和 time
    char target_pkg[128] = {0};
    long signal_time = 0;
    
    // 匹配格式: {"pkg":"xxx","time":123456}
    if (sscanf(buffer, "{\"pkg\":\"%[^\"]\",\"time\":%ld}", target_pkg, &signal_time) == 2) {
        if (std::string(target_pkg) == current_pkg) {
            // 获取当前系统时间（秒）
            struct timeval tv;
            gettimeofday(&tv, NULL);
            long now = tv.tv_sec;

            // 15 秒超时判定
            if (now - signal_time > 15) {
                LOGI("信号已过期（距指令发出已过去 %ld 秒），放弃 Dump", now - signal_time);
            } else {
                LOGI("信号有效，正在执行自动脱壳...");
                il2cpp_dump("/sdcard/Download");
            }
            // 处理完毕后删除信号文件，防止重复执行
            unlink(SIGNAL_PATH);
        }
    }
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
            // 每一个被注入的应用启动时都会检查一次该文件
            perform_timed_dump(proc);
            env->ReleaseStringUTFChars(args->nice_name, proc);
        }
    }
private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
