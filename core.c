#include "sys.h"
#include "game.h"
#include "lo.h"
#include "level.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

extern int width;
extern int height;
extern wchar_t** buffer;
extern wchar_t** out_buffer;
extern int line;
Player* player;
Monster* monster;

int main() {
    /* initialization */
    {
        setlocale(LC_ALL, "ko_KR.UTF-8");
        srand((unsigned int)time(NULL));
        line=0;
        width=80;
        height=25;
        buffer=(wchar_t**)calloc(height, sizeof(wchar_t*));
        for(int i=0;i<height;i++) {
            buffer[i]=(wchar_t*)calloc(width, sizeof(wchar_t));
        }
        out_buffer=(wchar_t**)calloc(height, sizeof(wchar_t*));
        for(int i=0;i<height;i++) {
            out_buffer[i]=(wchar_t*)calloc(width, sizeof(wchar_t));
        }
        fmod_init();
    }
    
    /* prolog */
    {
        clear();
        fmod_play("data/intro.wav", 0, 11000);
        wchar_t* buffer = (wchar_t*)calloc(80*25, sizeof(wchar_t));
        get_text_from_file(buffer, NULL, "data/intro.txt");
        print_per_line(buffer);
        free(buffer);
    }

    /* game */
    {
        static const char* files[] = {
            "data/bg01.wav",
            "data/bg02.wav",
            "data/bg03.wav",
            NULL
        };

        player=create_player(100, 10);
        monster=create_monster(50, 5);
        add_player_skill(player, player_attack);
        add_player_skill(player, player_heal);
        add_monster_skill(monster, monster_attack);
        while(player->level<=10) {
            fmod_resume();
            fmod_loop(files);  // fmod_loop(const char** files) 형태여야 정상
            
            if(player->level==3) add_monster_skill(monster, monster_heal);
            if(player->level==P_AK_47_UNLOCK_LEVEL) add_player_skill(player, player_ak_47);
            if(player->level==P_T_34_UNLOCK_LEVEL) add_player_skill(player, player_t_34);
            if(player->level==P_NUCLEAR_UNLOCK_LEVEL) {
                add_player_skill(player, use_nuclear);
            }
            unsigned char selected_value=0;
            while((player->element).health>0 && (monster->element).health>0) {
                print_layout(player, monster);
                print(L"능력을 선택하세요: ");
                scanf("%hhu", &selected_value);
                if(selected_value<1 || selected_value>(player->element).skill_count) {
                    print(L"잘못된 입력입니다. 다시 선택하세요.");
                    xsleep(1);
                    continue;
                }
                else if(selected_value==5 && player->level>=P_NUCLEAR_UNLOCK_LEVEL && player->nuclear->gage<20) {
                    print(L"핵폭탄 게이지가 부족합니다. 다시 선택하세요.");
                    xsleep(1);
                    continue;
                }
                player_behavior(player, monster, selected_value);
                print_layout(player, monster);
                xsleep(1);
                if((monster->element).health==0) break;
                monster_behavior(monster, player);
                print_layout(player, monster);
                xsleep(1);
                if(player->level>=P_NUCLEAR_UNLOCK_LEVEL) {
                    charge_nuclear(player->nuclear);
                }
            }
            if((player->element).health==0) {
                fmod_stop();
                clear();
                print(L"당신은 패배하였습니다...");
                xsleep(2);
                goto EPILOG_FAILURE;
            }
            else {
                fmod_stop();
                clear();
                print(L"당신은 승리하였습니다!");
                xsleep(2);
            }
            player->level++;
            
            player->element.original_health+=20;
            player->element.health=player->element.original_health;
            player->element.damage+=5;

            monster->element.original_health+=20;
            monster->element.health=monster->element.original_health;
            monster->element.damage+=8;
    
        }
    }
    goto EPILOG_SUCCESS;

    EPILOG_FAILURE:
    /* epilog failure */
    {
        fmod_resume();
        fmod_play("data/failure.wav", 0, 11000);
        clear();
        wchar_t* buffer = (wchar_t*)calloc(80*25, sizeof(wchar_t));
        get_text_from_file(buffer, NULL, "data/fail.txt");
        print_per_line(buffer);
        free(buffer);
        goto CLEANUP;
    }
    EPILOG_SUCCESS:
    /* epilog 2 */
    {
        fmod_resume();
        fmod_play("data/victory.wav", 58000, 58000+11000);
        clear();
        wchar_t* buffer = (wchar_t*)calloc(80*25, sizeof(wchar_t));
        get_text_from_file(buffer, NULL, "data/outro.txt");
        print_per_line(buffer);
        free(buffer);
        goto CLEANUP;
    }
    CLEANUP:
    /* cleanup */
    {
        for(int i=0;i<height;i++) {
            free(buffer[i]);
        }
        free(buffer);
        for(int i=0;i<height;i++) {
            free(out_buffer[i]);
        }
        free(out_buffer);
        delete_player(player);
        delete_monster(monster);
        fmod_close();
        system("clear");
    }
    
    return 0;
}
