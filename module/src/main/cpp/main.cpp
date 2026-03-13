#include <jni.h>
#include <string>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include "zygisk.hpp"
#include "il2cpp_dump.h"
#include <android/log.h>

#define LOG_TAG "IL2CPP_HOOK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 信号文件路径
#define SIGNAL_PATH "/data/local/tmp/il2cpp_dump_pkg"

// 原始点击事件的回调（我们要 Hook 的目标）
void hooked_onItemClick(JNIEnv* env, jobject thiz, jobject parent, jobject view, jint position, jlong id) {
    LOGI("Hook 成功：检测到列表点击，位置: %d", position);

    // 1. 反射获取 MainActivity 里的 packageNames 列表
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldId = env->GetFieldID(clazz, "packageNames", "Ljava/util/List;");
    jobject listObj = env->GetObjectField(thiz, fieldId);

    // 2. 从 List 中取出对应位置的包名
    jclass listClazz = env->FindClass("java/util/List");
    jmethodID getMethod = env->GetMethodID(listClazz, "get", "(I)Ljava/lang/Object;");
    jstring pkgName = (jstring)env->CallObjectMethod(listObj, getMethod, position);

    const char* pkg = env->GetStringUTFChars(pkgName, nullptr);
    LOGI("识别到目标应用包名: %s", pkg);

    // 3. 自动触发：创建信号文件，并写入包名
    FILE* f = fopen(SIGNAL_PATH, "w");
    if (f) {
        fprintf(f, "%s", pkg);
        fclose(f);
        chmod(SIGNAL_PATH, 0777);
        LOGI("信号已发送，准备 Dump...");
    }

    env->ReleaseStringUTFChars(pkgName, pkg);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        
        // 逻辑 A：如果进入了辅助 App 进程
        if (proc && std::string(proc) == "com.unpacker.helper") {
            LOGI("正在对接辅助 App...");
            
            // 动态注册 Hook，劫持原本的点击逻辑
            jclass mainActivity = env->FindClass("com/unpacker/helper/MainActivity$1");
            if (mainActivity) {
                JNINativeMethod methods[] = {
                    {"onItemClick", "(Landroid/widget/AdapterView;Landroid/view/View;IJ)V", (void*)hooked_onItemClick}
                };
                env->RegisterNatives(mainActivity, methods, 1);
                LOGI("点击事件已成功托管至 C++ 层");
            }
        }

        // 逻辑 B：如果是游戏进程
        // 这里启动线程监控 SIGNAL_PATH，如果内容匹配当前包名，直接执行 il2cpp_dump
        
        env->ReleaseStringUTFChars(args->nice_name, proc);
    }
};

REGISTER_ZYGISK_MODULE(MyModule)