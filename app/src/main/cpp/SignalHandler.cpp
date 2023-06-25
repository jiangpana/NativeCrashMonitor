//
// Created by darrenzeng on 2021/2/21.
//
#include <unistd.h>
#include "SignalHandler.h"
#include "StackPrinter.h"

void installAlternateStack() {
    stack_t newStack;
    stack_t oldStack;
    memset(&newStack, 0, sizeof(newStack));
    memset(&oldStack, 0, sizeof(oldStack));
    static const unsigned sigaltstackSize = std::max(16384, SIGSTKSZ);
    // 要先把原来的拿出来，可能会有一些其他框架早已设置好了
    // 避免 bugly，qapm 冲突
    if (sigaltstack(NULL, &oldStack) == -1
        || !oldStack.ss_sp
        || oldStack.ss_size < sigaltstackSize) {
        newStack.ss_sp = calloc(1, sigaltstackSize);
        newStack.ss_size = sigaltstackSize;
        if (sigaltstack(&newStack, NULL) == -1) {
            // 也可以打印一个警告
            free(newStack.ss_sp);
        }
    }
}

void signalPass(int code, siginfo_t *si, void *sc) {
    LOGE("监听到了 native 的崩溃");
    // 这里要考虑非信号方式防止死锁
    signal(code, SIG_DFL);
    signal(SIGALRM, SIG_DFL);
    (void)alarm(8);
    // 解析栈信息，回调给 java 层，上报到后台或者保存本地文件，下次课讲
    // 给系统原来默认的处理，否则就会进入死循环
    print_stack();
    oldHandlers[code].sa_sigaction(code, si, sc);
}

// signum：代表信号编码，可以是除SIGKILL及SIGSTOP外的任何一个特定有效的信号，如果为这两个信号定义自己的处理函数，将导致信号安装错误。
// act：指向结构体sigaction的一个实例的指针，该实例指定了对特定信号的处理，如果设置为空，进程会执行默认处理。
// oldact：和参数act类似，只不过保存的是原来对相应信号的处理，也可设置为NULL。
// int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact));
bool installSignalHandlers() {
    // 需要保存原来的处理，获取系统的或者其他第三方已经设置的
    for (int i = 0; i < exceptionSignalsNumber; ++i) {
        if (sigaction(exceptionSignals[i], NULL, &oldHandlers[exceptionSignals[i]]) == -1) {
            // 可以输出一个警告
            return false;
        }
    }
    // 初始化赋值
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    // 指定信号处理的回调函数
    sa.sa_sigaction = signalPass;
    sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
    // 处理当前信号量的时候不关心其他的
    for (int i = 0; i < exceptionSignalsNumber; ++i) {
        sigaddset(&sa.sa_mask, exceptionSignals[i]);
    }
    // 1. 调用 sigaction 来处理信号回调
    for (int i = 0; i < exceptionSignalsNumber; ++i) {
        if (sigaction(exceptionSignals[i], &sa, NULL) == -1) {
            // 可以输出一个警告
        }
    }
    return true;
}