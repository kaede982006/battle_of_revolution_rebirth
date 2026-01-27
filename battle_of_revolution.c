#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

extern char **environ;

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    char exe_path[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (n < 0) die("readlink");
    exe_path[n] = '\0';  // readlink는 널 종료를 안 함

    // exe_path에서 디렉토리 부분만 남기기
    char *slash = strrchr(exe_path, '/');
    if (!slash) {
        fprintf(stderr, "Unexpected exe path: %s\n", exe_path);
        return 1;
    }
    *slash = '\0'; // exe_path == 런처가 있는 폴더 경로

    // core 경로 만들기: <launcher_dir>/core
    char core_path[PATH_MAX];
    if (snprintf(core_path, sizeof(core_path), "%s/core", exe_path) >= (int)sizeof(core_path)) {
        fprintf(stderr, "core_path too long\n");
        return 1;
    }

    // 핵심: 작업 디렉토리를 런처 폴더로 변경 (core가 상대경로로 파일 읽기 가능)
    if (chdir(exe_path) != 0) die("chdir");

    // LD_LIBRARY_PATH 설정: <launcher_dir>[:기존값]
    const char *old_ld = getenv("LD_LIBRARY_PATH");
    char new_ld[2 * PATH_MAX + 2];

    if (old_ld && old_ld[0] != '\0') {
        if (snprintf(new_ld, sizeof(new_ld), "%s:%s", exe_path, old_ld) >= (int)sizeof(new_ld)) {
            fprintf(stderr, "LD_LIBRARY_PATH too long\n");
            return 1;
        }
    } else {
        if (snprintf(new_ld, sizeof(new_ld), "%s", exe_path) >= (int)sizeof(new_ld)) {
            fprintf(stderr, "LD_LIBRARY_PATH too long\n");
            return 1;
        }
    }

    if (setenv("LD_LIBRARY_PATH", new_ld, 1) != 0) die("setenv");

    // 런처에 전달된 인자를 core로 그대로 전달
    // argv[0]만 core_path로 바꿔서 execv
    char **core_argv = (char **)calloc((size_t)argc + 1, sizeof(char *));
    if (!core_argv) die("calloc");

    core_argv[0] = core_path;
    for (int i = 1; i < argc; i++) core_argv[i] = argv[i];
    core_argv[argc] = NULL;

    execv(core_path, core_argv);
    // execv 실패 시 여기로 옴
    die("execv");
    return 1;
}

