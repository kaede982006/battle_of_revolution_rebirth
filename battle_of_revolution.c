#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PATH_MAX 4096

int main() {
    char buf[PATH_MAX];
    char command[2*PATH_MAX+23];
    readlink("/proc/self/exe", buf, PATH_MAX);
    char* p=(char*)(buf+strlen(buf));
    while (*p!='/') p--;
    p++; *p='\0';

    sprintf(command,"LD_LIBRARY_PATH=%s %s/core", buf, buf);
    system(command);
    return 0;
}
