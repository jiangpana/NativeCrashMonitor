#include <jni.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unwind.h>
#include <dlfcn.h>
#include <android/log.h>
#include <cxxabi.h>
#include <vector>
#include <inttypes.h>
#include "SignalHandler.h"


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello 安装完成";

    // 设置监听信号量回调处理
    installSignalHandlers();
    // 设置额外的栈空间，让信号处理在单独的栈中处理
    installAlternateStack();

    return env->NewStringUTF(hello.c_str());
}


void test3() {
    int a = 1;
    int b = 0;
    int c;
    c = a / b;
}

void test2() {
    test3();

}

void test1() {
    test2();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_testCrash(JNIEnv *env, jobject thiz) {
    test1();
}