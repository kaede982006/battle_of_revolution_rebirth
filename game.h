#ifndef __GAME_H__
#define __GAME_H__

#include <stdbool.h>
#include <stdlib.h>

typedef struct _Player  Player;
typedef struct _Monster Monster;
typedef void (*p_skill)(Player*, Monster*);
typedef struct _Element{
    unsigned int health;
    unsigned int damage;
    unsigned int original_health;
    p_skill* skill;
    unsigned int skill_count;
} Element;

typedef struct _Nuclear {
    unsigned short gage;
    unsigned short concentrated_value;
} Nuclear;
typedef struct _Player {
    Element element;
    Nuclear* nuclear;
    unsigned short level;
} Player;
typedef struct _Monster {
    Element element;
} Monster;

Player* create_player(unsigned int health, unsigned int damage);
Monster* create_monster(unsigned int health, unsigned int damage);
void monster_behavior(Monster* monster, Player* player);
void player_behavior(Player* player, Monster* monster, unsigned char selected_value);
void add_player_skill(Player* player, p_skill skill);
void add_monster_skill(Monster* monster, p_skill skill);
void player_attack(Player* player, Monster* monster);
void player_ak_47(Player* player, Monster* monster);
void player_t_34(Player* player, Monster* monster);
void monster_attack(Player* player, Monster* monster);
void player_heal(Player* player, Monster* monster);
void monster_heal(Player* player, Monster* monster);
void delete_player(Player* player);
void delete_monster(Monster* monster);
Nuclear* create_nuclear();
void charge_nuclear(Nuclear* nuclear);
void use_nuclear(Player* player, Monster* monster);
#endif
