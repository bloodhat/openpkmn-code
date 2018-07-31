#ifndef __OPENPKMN_WAITINGROOM_STATS__
#define __OPENPKMN_WAITINGROOM_STATS__

int hp_lower_bound(int base, int level);
  
int hp_upper_bound(int base, int level);

int stat_lower_bound(int base, int level);

int stat_upper_bound(int base, int level);

int hp_stat_valid(int test_hp, int id, int level);
  
int attack_stat_valid(int test_attack, int id, int level);

int defense_stat_valid(int test_defense, int id, int level);

int speed_stat_valid(int test_speed, int id, int level);

int special_stat_valid(int test_special, int id, int level);

int get_max_hp(int species, int * max_hp);

int get_max_attack(int species, int * max_attack);

int get_max_defense(int species, int * max_defense);

int get_max_speed(int species, int * max_speed);

int get_max_special(int species, int * max_special);
  
#endif
