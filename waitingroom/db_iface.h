#ifndef __OPENPKMN_DB_IFACE_WAITINGROOM__
#define __OPENPKMN_DB_IFACE_WAITINGROOM__ 1

#include "datatypes.h"
#include "../share/db_iface.h"

int get_random_species_numbers(int pkmn_restriction, int repeat,
			       Species_ll ** species_numbers);

int get_random_move_numbers(int species, int learning_restriction,
			    int move_restrictions,
			    Move_record_ll ** move_numbers);

int get_user_rules(char * user_name, int * user_rules);

int get_user_current_team_rules(char * user_name, int * team_rules);

int get_user_current_team(char * user_name, int * team_id);

int get_user_selected_starter(char * user_name, int * starter);

int get_user_status(char * user_name, int * user_status);

int insert_move(int owner_id, int pkmn_id, int number, int pp);

int insert_pkmn(int owner_id, int species, char * nickname,
		int level, int max_hp, int attack, int defense,
		int speed, int special, int rules, int * pkmn_id);

int insert_team_pkmn(int team_id, int pkmn_id);

int insert_team(int owner_id, int * team_id);

int insert_battle(int challenger_id, int defender_id,
		  int challenger_team_id, int challenger_starter,
		  int defender_team_id, int defender_starter, int * battle_id);

int set_user_status(char * user_name, int new_status);

int set_user_ruleset(char * user_name, int new_ruleset);

int set_user_selected_team(char * user_name, int team_id);

int set_user_selected_starter(char * user_name, int starter);

int set_round_end_state(int round_id, int end_state);

int get_team_rules(int team_id, int user_id, int * rules);

int get_user_name(int user_id, char * user_name);

int get_users_pkmn(int user_id, Pkmn_ll ** pkmn, int * num_pkmn,
		   int * num_moves);

int get_users_teams(int user_id, Team_ll ** teams, int * num_teams,
		    int * num_members);

int get_active_users(int user_id, User_ll ** users, int * num_users);

int get_battleable_users(int user_id, User_ll ** users, int * num_users);

int insert_ruleset(int team_id);

int set_pkmn_invisible(int pkmn_id);

int set_team_invisible(int team_id);

int get_max_hp(int species, int * max_hp);

int get_max_attack(int species, int * max_attack);

int get_max_defense(int species, int * max_defense);

int get_max_speed(int species, int * max_speed);

int get_max_special(int species, int * max_special);

#define NATURAL 0
#define TM_HM 1
#define GS_TRADEBACK 2

int move_learned_by_method(int pkmn_id, int move_num, int method,
			   int * found);

int get_move_pp(int move_id,  int * pp);

#endif
