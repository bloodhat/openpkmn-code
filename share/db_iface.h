#ifndef __OPENPKMN_DB_IFACE_SHARE__
#define __OPENPKMN_DB_IFACE_SHARE__ 1

#include <mysql/mysql.h>

#include "db_conf.h"
#include "../share/datatypes.h"

enum ret_vals { DATABASE_ERROR = -1, NULL_RESULT = -2, NO_RESULTS = 3 };

typedef struct db_move {
  int move_id;
  int pp;
  struct db_move * next;
} Db_move;

typedef struct db_pkmn {
  char nickname[32];

  int id;
  
  int species;
  int level;

  int max_hp;
  int attack;
  int defense;
  int speed;
  int special;

  int num_moves;
  struct db_pkmn * next;
  Db_move * moves;
} Db_pkmn;


int get_user_key_type(char * user_name, int * key_type);

int get_user_key_data(char * user_name, unsigned char * key_data);

int get_user_id(char * user_name, int * user_id);

int get_base_hp(int pkmn_id,  int * base);

int get_base_attack(int pkmn_id,  int * base);

int get_base_defense(int pkmn_id,  int * base);

int get_base_speed(int pkmn_id,  int * base);

int get_base_special(int pkmn_id,  int * base);

int get_user_name_by_parameter(int id_param, char * statement_str,
			       char * user_name);

int get_integer_parameter(int id, char * statement_str, int * ret_val);

#endif
