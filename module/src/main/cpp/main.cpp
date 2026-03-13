#include <jni.h>
#include <string>
#include <cstdio>
#include <sys/stat.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOG_TAG "IL2CPP_HOOK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 信号文件：用于通知游戏进程开始脱壳
#define SIGNAL_FILE "/data/local/tmp/il2cpp_dump_pkg"

// 被 Hook 的 Java 回调函数
void hooked_onItemClick(JNIEnv* env, jobject thiz, jobject parent, jobject view, jint position, jlong id) {
    LOGI("Hook 命中：拦截到列表点击动作");

    // 1. 获取 MainActivity 的类定义
    jclass mainActivityClass = env->GetObjectClass(thiz);
    
    // 2. 通过反射找到 packageNames 成员变量
    jfieldID fieldId = env->GetFieldID(mainActivityClass, "packageNames", "Ljava/util/List;");
    jobject listObject = env->GetObjectField(thiz, fieldId);
    
    // 3. 调用 List.get(position) 提取字符串包名
    jclass listClass = env->FindClass("java/util/List");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jstring pkgName = (jstring)env->CallObjectMethod(listObject, getMethod, position);
    
    const char* pkgStr = env->GetStringUTFChars(pkgName, nullptr);
    LOGI("成功获取包名信号: %s", pkgStr);

    // 4. 将包名写入公共目录信号文件
    FILE* f = fopen(SIGNAL_FILE, "w");
    if (f) {
        fprintf(f, "%s", pkgStr);
        fclose(f);
        chmod(SIGNAL_FILE, 0777); // 确保游戏进程有权限读取
    }
    
    env->ReleaseStringUTFChars(pkgName, pkgStr);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        const char* proc = env->GetStringUTFChars(args->nice_name, nullptr);
        
        // 判定进入辅助 App 进程
        if (proc && std::string(proc) == "com.unpacker.helper") {
            LOGI("正在对辅助 App 实施 JNI 注入...");
            
            // 查找匿名内部类 MainActivity$1 (即点击监听器)
            jclass listenerClass = env->FindClass("com/unpacker/helper/MainActivity$1");
            if (listenerClass) {
                JNINativeMethod methods[] = {
                    {"onItemClick", "(Landroid/widget/AdapterView;Landroid/view/View;IJ)V", (void*)hooked_onItemClick}
                };
                // 使用 RegisterNatives 覆盖原有的 Java 方法
                env->RegisterNatives(listenerClass, methods, 1);
                LOGI("点击事件劫持完成");
            }
        }
        env->ReleaseStringUTFChars(args->nice_name, proc);
    }
private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
