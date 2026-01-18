extern "C" {
    #include "sys.h"
}
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ctime>
#include <locale.h>
#include <sys/wait.h>
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <thread>

FMOD::System *fmod_system = NULL;
BOOL release_value=FALSE;
int line;
int width, height;
wchar_t** buffer;
wchar_t** out_buffer;
extern "C" BOOL fmod_result(FMOD_RESULT result) {
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        return FALSE;
    }
    return TRUE;
}
extern "C" void clear() {
    printf("\x1b[2J\x1b[H");
    for (int r = 0; r < height; r++) {
        for (int s = 0; s < width; s++) {
            printf("\x1b[41m \x1b[0m");
        }
        printf("\n");
    }
    fflush(stdout);
    line=0;
}
extern "C" void console_update(const wchar_t* str) {
    if (line==height-1) {
        for(int i=0;i<line-1;i++) {
            wcscpy(out_buffer[i], out_buffer[i+1]);
        }
        wcscpy(out_buffer[line], str);
    }
    else {
        wcscpy(out_buffer[line], str);
        line++;
    }
}
extern "C" void print(wchar_t* format, ...) {
    wchar_t out[width+1];
    wchar_t buf[width+16];

    va_list args;
    va_start(args, format);
    vswprintf(out, width+1, format, args);
    va_end(args);

    console_update(out);

    int temp=line; 
    for(int i=0;i<temp;i++) {
        // wchar_t를 char로 변환하여 출력
        char out_str[width+16];
        wcstombs(out_str, out_buffer[i], width+16);
        printf("\033[41;33m\033[%d;%dH%s\033[0m", i+1, 0, out_str);
    }
    line=temp;
    fflush(stdout);
}
extern "C" void print_with_delay(wchar_t* format, ...) {
    wchar_t out[width+1];
    wchar_t buf[width+16];

    va_list args;
    va_start(args, format);
    vswprintf(out, width+1, format, args);
    va_end(args);

    console_update(out);

    int temp=line; 
    for(int i=0;i<temp;i++) {
        // wchar_t를 char로 변환하여 출력
        char out_str[width+16];
        wcstombs(out_str, out_buffer[i], width+16);
        printf("\033[41;33m\033[%d;%dH%s\033[0m", i+1, 0, out_str);
    }
    line=temp;
    fflush(stdout);

    xsleep(wcslen(out)/8 +1);
}
extern "C" wchar_t* get_text_from_file(wchar_t* buffer, FILE* file, const char* filename) {
    setlocale(LC_ALL, "ko_KR.UTF-8");
    file = fopen(filename, "r, ccs=UTF-8");
    if(file == NULL) return NULL;
    
    int total_idx = 0;
    wchar_t in_buffer[81];
    
    while (fgetws(in_buffer, 81, file) != NULL && total_idx < 80*25-1) {
        size_t len = wcslen(in_buffer);
        if (total_idx + len < 80*25-1) {
            wmemcpy(&buffer[total_idx], in_buffer, len);
            total_idx += len;
        } else {
            break;
        }
    }
    buffer[total_idx] = L'\0';
    
    fclose(file);
    return buffer;
}
extern "C" void print_per_line(wchar_t* buffer) {
    wchar_t* line_start = buffer;
    wchar_t* line_end;
    while ((line_end = wcschr(line_start, L'\n')) != NULL) {
        *line_end = L'\0'; // 개행 문자를 널 문자로 대체
        print_with_delay(line_start);
        line_start = line_end + 1; // 다음 줄의 시작 위치로 이동
    }
}
extern "C" void xsleep(unsigned int second) {
    char command[32];
    sprintf(command, "sleep %u", second);
    system(command);
}
extern "C" BOOL fmod_init() {
    FMOD_RESULT result;

    result = FMOD::System_Create(&fmod_system);      // Create the main system object.
    if(!fmod_result(result)) return FALSE;

    result = fmod_system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
    if(!fmod_result(result)) return FALSE;
    return TRUE;
}
extern "C" BOOL fmod_close() {
    if (fmod_system) {
        FMOD_RESULT result = fmod_system->release();
        if(!fmod_result(result)) return FALSE;
        fmod_system = NULL;
        return TRUE;
    }
    return FALSE;
}
// 실제 스레드에서 실행될 작업 함수 (Internal)
void fmod_play_thread_worker(const char* filename, unsigned int start_ms, unsigned int end_ms) {
    FMOD::Sound* sound = nullptr;
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result;

    // 1. 사운드 생성
    result = fmod_system->createSound(filename, FMOD_DEFAULT, nullptr, &sound);
    if (result != FMOD_OK) return;

    // 2. 일시정지 상태로 재생 시작
    result = fmod_system->playSound(sound, nullptr, true, &channel);
    if (result == FMOD_OK && channel) {
        channel->setPosition(start_ms, FMOD_TIMEUNIT_MS);
        channel->setPaused(false);

        bool isPlaying = true;
        while (isPlaying) {
            unsigned int position = 0;
            channel->getPosition(&position, FMOD_TIMEUNIT_MS);
            
            // 재생 종료 조건 검사 (범위 초과 또는 사운드 종료)
            if (end_ms != 0 && position >= end_ms) {
                channel->stop();
                isPlaying = false;
            } else {
                channel->isPlaying(&isPlaying);
            }
            
            if(release_value==TRUE) {
                break;
            }
            // CPU 점유율 과다 방지를 위한 미세 대기 (컴퓨팅 사고: 효율성)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // 3. 리소스 해제 (스레드 종료 전 반드시 수행)
    if (sound) {
        sound->release();
    }
}

// 외부 호출을 위한 C 인터페이스
extern "C" BOOL fmod_play(const char* filename, unsigned int start_ms, unsigned int end_ms) {
    try {
        // 분리된 스레드(Detached Thread)에서 재생 실행
        // 메인 스레드는 기다리지 않고 즉시 TRUE 반환
        std::thread(fmod_play_thread_worker, filename, start_ms, end_ms).detach();
        return TRUE;
    } catch (...) {
        return FALSE;
    }
}
void fmod_loop_thread_worker(const char** files) {
    int file_count = 0;
    if(!files || !files[0]) return;
    while(files[file_count] != NULL) {
        file_count++;
    }
    BOOL quit=FALSE;
    while(!quit) {
        for(int i=0;i<file_count && quit==FALSE;i++) {
            unsigned int sound_length;
            FMOD::Sound* sound = nullptr;
            FMOD_RESULT result = fmod_system->createSound(files[i], FMOD_DEFAULT, nullptr, &sound);
            if (result != FMOD_OK) return;
            sound->getLength(&sound_length, FMOD_TIMEUNIT_MS);
            sound->release();
            std::thread t(fmod_play_thread_worker, files[i], 0, sound_length);
            time_t start, finish;
            start = time(NULL); 
            while(time(NULL)-start<=sound_length/1000) {
                if(release_value==TRUE) {
                    quit=TRUE; break;
                }
            }
            t.join();
        }
    }
}
// 외부 호출을 위한 C 인터페이스
extern "C" BOOL fmod_loop(const char** files) {
    try {
        // 분리된 스레드(Detached Thread)에서 재생 실행
        // 메인 스레드는 기다리지 않고 즉시 TRUE 반환
        std::thread(fmod_loop_thread_worker, files).detach();
        return TRUE;
    } catch (...) {
        return FALSE;
    }
}

extern "C" void fmod_stop() {
    release_value=TRUE;
}
extern "C" void fmod_resume() {
    release_value=FALSE;
}