
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <execinfo.h>   // backtrace
#include <unistd.h>     // getpid


// 注意：绝对不要出现出现编译警告 [-Wimplicit-function-declaration]

/*
gcc -Wall -o lite lite.c -g ;./lite
=================>>>catch signal 11<<<=====================
[00] ./lite(+0x12fb) [0x55bccaa652fb]
[01] ./lite(+0x14f6) [0x55bccaa654f6]
[02] /lib/x86_64-linux-gnu/libc.so.6(+0x46210) [0x7f385d3e7210]
[03] ./lite(+0x1545) [0x55bccaa65545]
[04] /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7f385d3c80b3]
[05] ./lite(+0x120e) [0x55bccaa6520e]
55bccaa65000-55bccaa66000 r-xp 00001000 00:2c 36873221949326498          /mnt/d/codes/git/backtracedemo/lite
7f385d389000-7f385d39b000 r-xp 00003000 08:10 12081                      /usr/lib/x86_64-linux-gnu/libgcc_s.so.1
7f385d3c6000-7f385d53e000 r-xp 00025000 08:10 11971                      /usr/lib/x86_64-linux-gnu/libc-2.31.so
7f385d5a0000-7f385d5c3000 r-xp 00001000 08:10 11854                      /usr/lib/x86_64-linux-gnu/ld-2.31.so
7ffdca3cb000-7ffdca3cc000 r-xp 00000000 00:00 0                          [vdso]
Segmentation fault


addr2line -fe lite  +0x1545
main
/mnt/d/codes/git/backtracedemo/lite.c:99
*/

// 定义保存栈帧的最大深度 根据项目复杂度定
#define STACK_FRAME_BUFFER_SIZE (int)128

void dump_backtrace() {
    // 定义栈帧缓冲区
    void *stack_frame_buffer[STACK_FRAME_BUFFER_SIZE];

    // 获取当前线程的栈帧
    int stack_frames_size = backtrace(stack_frame_buffer, STACK_FRAME_BUFFER_SIZE);

    // 将栈帧信息转化为字符串
    char **stack_frame_string_buffer = backtrace_symbols(stack_frame_buffer, stack_frames_size);
    if (NULL == stack_frame_string_buffer) {
        printf("failed to backtrace_symbols\n");
        return;
    }

    // 遍历打印栈帧信息
    for (int i = 0; i < stack_frames_size; i++) {
        printf("[%02d] %s\n", i, stack_frame_string_buffer[i]);
    }

    // 释放栈帧信息字符串缓冲区
    free(stack_frame_string_buffer);
}

void dump_maps() {
    char cmd[128];
    //snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps", getpid());
    snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps | grep r-xp", getpid());

    FILE *stream = popen(cmd, "r");
    if (NULL == stream) {
        printf("popen [%s] failed\n", cmd);
        return;
    }

    ssize_t read;
    char *line = NULL;
    size_t len = 0;

    while ((read = getline(&line, &len, stream)) != -1) {
        printf("%s", line);
    }

    pclose(stream);
}

void signal_handler(int signo) {
    printf("\n=================>>>catch signal %d<<<=====================\n", signo);
    dump_backtrace();
    dump_maps();

#ifdef ENABLE_LOG
    zlog_fini();
    system("sync");
#endif

    // 恢复信号默认处理(SIG_DFL)并重新发送信号(raise)
    signal(signo, SIG_DFL);
    raise(signo);
}

int main()
{
    // 捕获段错误信号SIGSEGV
    signal(SIGSEGV, signal_handler);
    
    // 模拟段错误信号
    int *pTmp = NULL;
    *pTmp = 1;	//对未分配内存空间的指针进行赋值，模拟访问非法内存段错误
    
    return 0;
}