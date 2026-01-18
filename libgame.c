#include "game.h"

Player* create_player(unsigned int health, unsigned int damage) {
    Player* player=(Player*)malloc(sizeof(Player));
    (player->element).health=health;
    (player->element).original_health=health;
    (player->element).damage=damage;
    (player->element).skill=(p_skill*)calloc(1, sizeof(p_skill));
    (player->element).skill_count=0;
    player->level=1;
    player->nuclear=create_nuclear();
    return player;
}
Monster* create_monster(unsigned int health, unsigned int damage) {
    Monster* monster=(Monster*)malloc(sizeof(Monster));
    (monster->element).health=health;
    (monster->element).original_health=health;
    (monster->element).damage=damage;
    (monster->element).skill=(p_skill*)calloc(1, sizeof(p_skill));
    (monster->element).skill_count=0;
    return monster;
}
void monster_behavior(Monster* monster, Player* player) {
    if((monster->element).skill_count==0) return;
    ((monster->element).skill)[rand()%((monster->element).skill_count)](player, monster);

}
void player_behavior(Player* player, Monster* monster, unsigned char selected_value) {
    if((player->element).skill_count==0) return;
    ((player->element).skill)[selected_value-1](player, monster);
}
void add_monster_skill(Monster* monster, p_skill skill) {
    ((monster->element).skill)[(monster->element).skill_count]=skill;
    (monster->element).skill_count++;
    (monster->element).skill=(p_skill*)realloc((monster->element).skill, ((monster->element).skill_count+1)*sizeof(p_skill));
};
void add_player_skill(Player* player, p_skill skill) {
    ((player->element).skill)[(player->element).skill_count]=skill;
    (player->element).skill_count++;
    (player->element).skill=(p_skill*)realloc((player->element).skill, ((player->element).skill_count+1)*sizeof(p_skill));
};
void player_attack(Player* player, Monster* monster) {
    if(monster->element.health<(player->element).damage) {
        monster->element.health=0;
        return;
    }
    (monster->element).health-=(player->element).damage;
}
void player_ak_47(Player* player, Monster* monster) {
    if(monster->element.health<3*(player->element).damage/2) {
        monster->element.health=0;
        return;
    }
    (monster->element).health-=3*(player->element).damage/2;
    if(player->element.health<(player->element).damage/5) {
        player->element.health=0;
        return;
    }
    (player->element).health-=(player->element).damage/5;
}
void player_t_34(Player* player, Monster* monster) {
    if(monster->element.health<2*(player->element).damage) {
        monster->element.health=0;
        return;
    }
    (monster->element).health-=2*(player->element).damage;
    if(player->element.health<(player->element).damage/2) {
        player->element.health=0;
        return;
    }
    (player->element).health-=(player->element).damage/2;
}
void monster_attack(Player* player, Monster* monster) {
    if(player->element.health<(monster->element).damage) {
        player->element.health=0;
        return;
    }
    (player->element).health-=(monster->element).damage;
}
void player_heal(Player* player, Monster* monster) {
    if(player->element.original_health<(player->element).health+(player->element).damage) {
        player->element.health=player->element.original_health;
        return;
    }
    (player->element).health+=rand()%((player->element).damage/10)+(player->element).damage;
}
void monster_heal(Player* player, Monster* monster) {
    if(monster->element.original_health<(monster->element).health+(monster->element).damage) {
        monster->element.health=monster->element.original_health;
        return;
    }
    (monster->element).health+=rand()%((monster->element).damage/10)+(monster->element).damage;
}   
void delete_player(Player* player) {
    free((player->element).skill);
    free(player->nuclear);
    free(player);
}
void delete_monster(Monster* monster) {
    free((monster->element).skill);
    free(monster);
}
Nuclear* create_nuclear() {
    Nuclear* nuclear=(Nuclear*)malloc(sizeof(Nuclear));
    nuclear->gage=0;
    nuclear->concentrated_value=0;
    return nuclear;
}
void charge_nuclear(Nuclear* nuclear) {
    if (nuclear->gage==20 && nuclear->concentrated_value<20) {
        nuclear->concentrated_value++;
    } else if (nuclear->gage<20) {
        nuclear->gage++;
    }
}
void use_nuclear(Player* player, Monster* monster) {
    if(player->nuclear->gage<20) return;
    if(monster->element.health<(player->element).damage*4+player->nuclear->concentrated_value*5) {
        monster->element.health=0;
        player->nuclear->gage=0;
        player->nuclear->concentrated_value=0;
        return;
    }
    (monster->element).health-=(player->element).damage*4+player->nuclear->concentrated_value*5;
    player->nuclear->gage=0;
    player->nuclear->concentrated_value=0;
}
