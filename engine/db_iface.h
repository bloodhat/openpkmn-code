#ifndef __OPENPKMN_DB_IFACE_ENGINE
#define __OPENPKMN_DB_IFACE_ENGINE__ 1

#include "datatypes.h"
#include "../share/db_iface.h"

int get_battle_challenger_pkmn(int battle_id, Db_pkmn ** pkmn);

int get_battle_defender_pkmn(int battle_id, Db_pkmn ** pkmn);

int get_battle_challenger_name(int battle_id, char * user_name);

int get_battle_defender_name(int battle_id, char * user_name);

int get_battle_challenger_starter(int battle_id, int * starter);

int get_battle_defender_starter(int battle_id, int * starter);

int insert_turn(int attacker_id, int defender_id,
		int attacker_pkmn_id, int defender_pkmn_id,
		char lead_turn, int round_id, int * turn_id);

int insert_round(int round_number, int battle_id, int end_status,
		 int * round_id);

int insert_battle_state(Active_data * state, int turn_id, int pkmn_id,
			int * state_id);

int insert_pkmn_data(Pkmn * pkmn);

int insert_event(int turn_id, int type, int details, int subject_id,
		 int subject_pkmn_id, int object_id, int object_pkmn_id);

int get_battle_rules(int battle_id, int * rules);

int get_first_type(int pkmn_id,  int * type);

int get_second_type(int pkmn_id,  int * type);
int get_move_parameter(int move_id,  int * parameter);

int get_move_type(int move_id,  int * type);

int get_move_secondary_effect(int move_id,  int * secondary_effect);

int get_move_secondary_chance(int move_id,  int * secondary_chance);

int get_move_power(int move_id,  int * power);

int get_move_accuracy(int move_id,  int * accuracy);

int insert_battle_winner_loser(int battle_id, int winner_id, int loser_id);

int insert_battle_score(int battle_id, int challenger_left, int defender_left);

#endif
