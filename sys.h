#ifndef __SYS_H__
#define __SYS_H__
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <fmod/fmod.h>

#define MAX_BUF_LINE 64 

typedef enum { FALSE, TRUE } BOOL;

void clear();
void print(wchar_t* format, ...);
void print_with_delay(wchar_t* format, ...);
void print_per_line(wchar_t* buffer);
void console_update(const wchar_t* str);
wchar_t* get_text_from_file(wchar_t* buffer, FILE* file, const char* filename);
void xsleep(unsigned int second);
BOOL fmod_init();
BOOL fmod_close();
BOOL fmod_play(const char* filename, unsigned int start_ms, unsigned int end_ms);
BOOL fmod_result(FMOD_RESULT result);
BOOL fmod_loop(const char** files);
void fmod_stop();
void fmod_resume();
#endif
