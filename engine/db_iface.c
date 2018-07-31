#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h> 

#include "db_iface.h"

int get_battle_param(int battle_id, char * statement_str, int * param)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind;
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;
  int result_count = 0;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_mysqlect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str, strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.buffer = (char *)&battle_id;
  in_bind.is_null = 0;
  in_bind.length = 0;
  
  if(mysql_stmt_bind_param(statement, &in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(&out_bind, 0, sizeof(out_bind));
  out_bind.buffer_type= MYSQL_TYPE_LONG;
  out_bind.buffer= (char *)param;
  out_bind.buffer_length = 32;
  out_bind.is_null= &is_null;
  out_bind.error= &error;

  if(mysql_stmt_bind_result(statement, &out_bind))
    {
      fprintf(stderr, "get_battle_participant_id: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  fprintf(stdout, "NULL id\n");
	  return NULL_RESULT;
	}
      if(error)
	{
	  fprintf(stdout, "Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      fprintf(stderr, " no results %i\n", battle_id);
      return NO_RESULTS;
    }

  mysql_close(&mysql);

  return 1;
}

int get_battle_pkmn(int battle_id, char * statement_str, Db_pkmn ** pkmn)
{
  
  MYSQL mysql;
  
  
  MYSQL_STMT * statement;
  
  MYSQL_BIND in_bind[1];
  MYSQL_BIND out_bind[11];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  Db_pkmn * root = NULL;
  Db_pkmn * current = NULL;
  Db_move * move_current = NULL;
  
  my_bool is_null[11];
  my_bool error[11];
  unsigned long length[11];

  int species;
  char nickname[32];
  unsigned long nick_len;
  int level;
  int max_hp;
  int attack;
  int defense;
  int speed;
  int special;
  int id;
  int move_num;
  int max_pp;
  
  int result_count = 0;
  int current_id = -1;

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "get_battle_pkmn: mysql_real_connect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)))
    {
      fprintf(stderr, "get_battle_pkmn: mysql_stmt_prepare: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }


  memset(in_bind, 0, sizeof(in_bind));
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&battle_id;
  in_bind[0].buffer_length = 0;
  in_bind[0].is_null = 0;
  
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "get_battle_pkmn: mysql_stmt_bind_param %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "get_battle_pkmn: mysql_stmt_execute %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(out_bind, 0, sizeof(out_bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)&species;
  out_bind[0].is_null= &is_null[0];
  out_bind[0].length= &length[0];
  out_bind[0].error = &error[0];
      
  /* STRING PARAM */
  out_bind[1].buffer_type= MYSQL_TYPE_STRING;
  out_bind[1].buffer= (char *)nickname;
  out_bind[1].buffer_length= 32;
  out_bind[1].is_null= &error[1];
  out_bind[1].length= &nick_len;
  out_bind[1].error = &error[1];
  
  /* more int params */
  out_bind[2].buffer_type= MYSQL_TYPE_LONG;
  out_bind[2].buffer= (char *)&level;
  out_bind[2].is_null= &is_null[2];
  out_bind[2].length= &length[2];
  out_bind[2].error = &error[2];
  
  out_bind[3].buffer_type= MYSQL_TYPE_LONG;
  out_bind[3].buffer= (char *)&max_hp;
  out_bind[3].is_null= &is_null[3];
  out_bind[3].length= &length[3];
  out_bind[3].error = &error[3];
      
  out_bind[4].buffer_type= MYSQL_TYPE_LONG;
  out_bind[4].buffer= (char *)&attack;
  out_bind[4].is_null= &is_null[4];
  out_bind[4].length= &length[4];
  out_bind[4].error = &error[4];
      
  out_bind[5].buffer_type= MYSQL_TYPE_LONG;
  out_bind[5].buffer= (char *)&defense;
  out_bind[5].is_null= &is_null[5];
  out_bind[5].length= &length[5];
  out_bind[5].error = &error[5];
      
  out_bind[6].buffer_type= MYSQL_TYPE_LONG;
  out_bind[6].buffer= (char *)&speed;
  out_bind[6].is_null= &is_null[6];
  out_bind[6].length= 0;
  out_bind[6].error = &error[6];
      
  out_bind[7].buffer_type= MYSQL_TYPE_LONG;
  out_bind[7].buffer= (char *)&special;
  out_bind[7].is_null= &is_null[7];
  out_bind[7].length= &length[7];
  out_bind[7].error = &error[7];
      
  out_bind[8].buffer_type= MYSQL_TYPE_LONG;
  out_bind[8].buffer= (char *)&id;
  out_bind[8].is_null= &is_null[8];
  out_bind[8].length= &length[8];
  out_bind[8].error = &error[8];

  
  out_bind[9].buffer_type= MYSQL_TYPE_LONG;
  out_bind[9].buffer= (char *)&move_num;
  out_bind[9].is_null= &is_null[8];
  out_bind[9].length= &length[8];
  out_bind[9].error = &error[8];
  
  out_bind[10].buffer_type= MYSQL_TYPE_LONG;
  out_bind[10].buffer= (char *)&max_pp;
  out_bind[10].is_null= &is_null[10];
  out_bind[10].length= &length[10];
  out_bind[10].error = &error[10];
      
  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

      
  /* go through the list of all the matching pkmn ids */
  while(!mysql_stmt_fetch(statement))
    {
      if(id != current_id)
	{
	  if(root == NULL)
	    {
	      current = root = malloc(sizeof(Db_pkmn));
	    }
	  else
	    {
	      current->next = malloc(sizeof(Db_pkmn));
	      current = current->next;
	    }
	  current_id = current->id = id;
	  strncpy(current->nickname, nickname, 32);
	  current->species = species;
	  current->level = level;
	  current->max_hp = max_hp;
	  current->attack = attack;
	  current->defense = defense;
	  current->speed = speed;
	  current->special = special;
	  current->next = NULL;
	  current->moves = NULL;
	}
      if(current->moves == NULL)
	{
	  move_current = current->moves
	    = malloc(sizeof(Db_move));
	}
      else
	{
	  move_current->next = malloc(sizeof(Db_move));
	  move_current = move_current->next;
	}
      move_current->move_id = move_num;
      move_current->pp = max_pp;
      move_current->next = NULL;
      
      result_count++;
    }

  
  if(result_count == 0)
    {
      return NO_RESULTS;
    }

  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);

  *pkmn = root; 
  
  return 1;
}

#define BATTLE_CHALLENGER_PKMN_STATEMENT "SELECT  P.Species, P.nickname, " \
    " P.level, P.max_hp, P.attack, P.defense, P.speed, P.special, "	\
    " P.id, M.Move_number, M.max_pp "					\
    " FROM Moves M, Team_PKMN TP, PKMN P, Battles B"			\
    " WHERE B.id = ? AND B.challenger_team_id = TP.team_id "		\
    " AND TP.pkmn_id = P.id AND P.id = M.Pkmn_id"

#define BATTLE_DEFENDER_PKMN_STATEMENT "SELECT  P.Species, P.nickname, " \
    " P.level, P.max_hp, P.attack, P.defense, P.speed, P.special, "	\
    " P.id, M.Move_number, M.max_pp "					\
    " FROM Moves M, Team_PKMN TP, PKMN P, Battles B"			\
    " WHERE B.id = ? AND B.defender_team_id = TP.team_id "		\
    " AND TP.pkmn_id = P.id AND P.id = M.Pkmn_id"

int get_battle_challenger_pkmn(int battle_id, Db_pkmn ** pkmn)
			      
{
  return get_battle_pkmn(battle_id, BATTLE_CHALLENGER_PKMN_STATEMENT, pkmn);
}

int get_battle_defender_pkmn(int battle_id, Db_pkmn ** pkmn)
{
  return get_battle_pkmn(battle_id, BATTLE_DEFENDER_PKMN_STATEMENT, pkmn);
}

int get_battle_challenger_name(int battle_id, char * user_name)
{
  return get_user_name_by_parameter(battle_id,
				     "SELECT name FROM "
				     "openpkmn.Users WHERE ID = "
				     "(SELECT challenger_id FROM "
				     "openpkmn.Battles WHERE id = ?)",
				     user_name);
}

int get_battle_defender_name(int battle_id, char * user_name)
{
  return get_user_name_by_parameter(battle_id,
				     "SELECT name FROM "
				     "openpkmn.Users WHERE ID = "
				     "(SELECT defender_id FROM "
				     "openpkmn.Battles WHERE id = ?)",
				     user_name);
}

int get_battle_challenger_starter(int battle_id, int * starter)
{
   return get_battle_param(battle_id, "SELECT challenger_starter FROM "
			   "Battles WHERE id = ?", starter);
}

int get_battle_defender_starter(int battle_id, int * starter)
{
   return get_battle_param(battle_id, "SELECT defender_starter FROM "
			   "Battles WHERE id = ?", starter);
}

int insert_turn(int attacker_id, int defender_id, int attacker_pkmn_id,
		int defender_pkmn_id, char lead_turn, int round_id,
		int * turn_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[6];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO "
    "openpkmn.Turns(round_id, attacker_id, defender_id, "
    "lead_turn, attacker_pkmn_id, defender_pkmn_id) "
    "VALUES(?, ?, ?, ?, ?, ?)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&round_id;
  bind[0].is_null= 0;
  bind[0].length= 0;
  
  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&attacker_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&defender_id;
  bind[2].is_null= 0;
  bind[2].length= 0;

  bind[3].buffer_type= MYSQL_TYPE_TINY;
  bind[3].buffer= (char *)&lead_turn;
  bind[3].is_null= 0;
  bind[3].length= 0;
 
  bind[4].buffer_type= MYSQL_TYPE_LONG;
  bind[4].buffer= (char *)&attacker_pkmn_id;
  bind[4].is_null= 0;
  bind[4].length= 0;
 
  bind[5].buffer_type= MYSQL_TYPE_LONG;
  bind[5].buffer= (char *)&defender_pkmn_id;
  bind[5].is_null= 0;
  bind[5].length= 0;
  
  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  *turn_id = mysql_stmt_insert_id(stmt);

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
}

int insert_event(int turn_id, int type, int details, int subject_id,
		 int subject_pkmn_id, int object_id, int object_pkmn_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[7];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO openpkmn.Events(turn_id, type, "
    " details, subject_id, subject_pkmn_id, object_id, object_pkmn_id) "
    " VALUES(?, ?, ?, ?, ?, ?, ?)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&turn_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&type;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&details;
  bind[2].is_null= 0;
  bind[2].length= 0;
  
  bind[3].buffer_type= MYSQL_TYPE_LONG;
  bind[3].buffer= (char *)&subject_id;
  bind[3].is_null= 0;
  bind[3].length= 0;
  
  bind[4].buffer_type= MYSQL_TYPE_LONG;
  bind[4].buffer= (char *)&subject_pkmn_id;
  bind[4].is_null= 0;
  bind[4].length= 0;
  
  bind[5].buffer_type= MYSQL_TYPE_LONG;
  bind[5].buffer= (char *)&object_id;
  bind[5].is_null= 0;
  bind[5].length= 0;
  
  bind[6].buffer_type= MYSQL_TYPE_LONG;
  bind[6].buffer= (char *)&object_pkmn_id;
  bind[6].is_null= 0;
  bind[6].length= 0;
  
  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  mysql_stmt_insert_id(stmt);

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
}

int insert_round(int round_number, int battle_id,
		 int end_status, int * round_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[3];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO "
    "openpkmn.Rounds(round_number, battle_id, "
    "round_end_state) VALUES(?, ?, ?)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&round_number;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&battle_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&end_status;
  bind[2].is_null= 0;
  bind[2].length= 0;
  
  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  *round_id = mysql_stmt_insert_id(stmt);

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
}

int insert_battle_state(Active_data * state, int turn_id, int pkmn_id,
			int * state_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[37];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO openpkmn.Battle_state(turn_id, pkmn_id, "
    "eff_type_1, eff_type_2, current_attack, current_defense, current_speed, "
    "current_special, attack_mod, defense_mod, speed_mod, special_mod, "
    "evade_mod, accuracy_mod, special_halve, physical_halve, critical_hit_up, "
    "mod_lock, substitute, toxic_turns, confusion_turns, seeded, enraged, "
    "flinched, recharging, disabled_selection, disabled_turns, "
    "trapping_damage, trapping_move, trapping_turns, "
    "original_trapping_victim, charging_move, temp_invul, rep_move_index, "
    "damage_taken, damage_is_counterable, last_move_used) "
    "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
    "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&turn_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&pkmn_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&state->type[0];
  bind[2].is_null= 0;
  bind[2].length= 0;

  bind[3].buffer_type= MYSQL_TYPE_LONG;
  bind[3].buffer= (char *)&state->type[1];
  bind[3].is_null= 0;
  bind[3].length= 0;

  bind[4].buffer_type= MYSQL_TYPE_LONG;
  bind[4].buffer= (char *)&state->current_stats.attack;
  bind[4].is_null= 0;
  bind[4].length= 0;

  bind[5].buffer_type= MYSQL_TYPE_LONG;
  bind[5].buffer= (char *)&state->current_stats.defense;
  bind[5].is_null= 0;
  bind[5].length= 0;

  bind[6].buffer_type= MYSQL_TYPE_LONG;
  bind[6].buffer= (char *)&state->current_stats.speed;
  bind[6].is_null= 0;
  bind[6].length= 0;

  bind[7].buffer_type= MYSQL_TYPE_LONG;
  bind[7].buffer= (char *)&state->current_stats.special;
  bind[7].is_null= 0;
  bind[7].length= 0;

  bind[8].buffer_type= MYSQL_TYPE_LONG;
  bind[8].buffer= (char *)&state->stat_mods.attack;
  bind[8].is_null= 0;
  bind[8].length= 0;

  bind[9].buffer_type= MYSQL_TYPE_LONG;
  bind[9].buffer= (char *)&state->stat_mods.defense;
  bind[9].is_null= 0;
  bind[9].length= 0;

  bind[10].buffer_type= MYSQL_TYPE_LONG;
  bind[10].buffer= (char *)&state->stat_mods.speed;
  bind[10].is_null= 0;
  bind[10].length= 0;

  bind[11].buffer_type= MYSQL_TYPE_LONG;
  bind[11].buffer= (char *)&state->stat_mods.special;
  bind[11].is_null= 0;
  bind[11].length= 0;

  bind[12].buffer_type= MYSQL_TYPE_LONG;
  bind[12].buffer= (char *)&state->evade_mod;
  bind[12].is_null= 0;
  bind[12].length= 0;

  bind[13].buffer_type= MYSQL_TYPE_LONG;
  bind[13].buffer= (char *)&state->accuracy_mod;
  bind[13].is_null= 0;
  bind[13].length= 0;

  bind[14].buffer_type= MYSQL_TYPE_LONG;
  bind[14].buffer= (char *)&state->special_halve;
  bind[14].is_null= 0;
  bind[14].length= 0;

  bind[15].buffer_type= MYSQL_TYPE_LONG;
  bind[15].buffer= (char *)&state->physical_halve;
  bind[15].is_null= 0;
  bind[15].length= 0;

  bind[16].buffer_type= MYSQL_TYPE_LONG;
  bind[16].buffer= (char *)&state->critical_hit_up;
  bind[16].is_null= 0;
  bind[16].length= 0;

  bind[17].buffer_type= MYSQL_TYPE_LONG;
  bind[17].buffer= (char *)&state->mod_lock;
  bind[17].is_null= 0;
  bind[17].length= 0;

  bind[18].buffer_type= MYSQL_TYPE_LONG;
  bind[18].buffer= (char *)&state->substitute;
  bind[18].is_null= 0;
  bind[18].length= 0;

  bind[19].buffer_type= MYSQL_TYPE_LONG;
  bind[19].buffer= (char *)&state->toxic_turns;
  bind[19].is_null= 0;
  bind[19].length= 0;

  bind[20].buffer_type= MYSQL_TYPE_LONG;
  bind[20].buffer= (char *)&state->confusion_turns;
  bind[20].is_null= 0;
  bind[20].length= 0;

  bind[21].buffer_type= MYSQL_TYPE_LONG;
  bind[21].buffer= (char *)&state->seeded;
  bind[21].is_null= 0;
  bind[21].length= 0;

  bind[22].buffer_type= MYSQL_TYPE_LONG;
  bind[22].buffer= (char *)&state->enraged;
  bind[22].is_null= 0;
  bind[22].length= 0;

  bind[23].buffer_type= MYSQL_TYPE_LONG;
  bind[23].buffer= (char *)&state->flinched;
  bind[23].is_null= 0;
  bind[23].length= 0;

  bind[24].buffer_type= MYSQL_TYPE_LONG;
  bind[24].buffer= (char *)&state->recharging;
  bind[24].is_null= 0;
  bind[24].length= 0;

  bind[25].buffer_type= MYSQL_TYPE_LONG;
  bind[25].buffer= (char *)&state->disabled_selection;
  bind[25].is_null= 0;
  bind[25].length= 0;

  bind[26].buffer_type= MYSQL_TYPE_LONG;
  bind[26].buffer= (char *)&state->disable_turns;
  bind[26].is_null= 0;
  bind[26].length= 0;

  bind[27].buffer_type= MYSQL_TYPE_LONG;
  bind[27].buffer= (char *)&state->trapping_damage;
  bind[27].is_null= 0;
  bind[27].length= 0;

  bind[28].buffer_type= MYSQL_TYPE_LONG;
  bind[28].buffer= (char *)&state->trapping_move;
  bind[28].is_null= 0;
  bind[28].length= 0;

  bind[29].buffer_type= MYSQL_TYPE_LONG;
  bind[29].buffer= (char *)&state->trapping_turns;
  bind[29].is_null= 0;
  bind[29].length= 0;

  bind[30].buffer_type= MYSQL_TYPE_LONG;
  bind[30].buffer= (char *)&state->original_trapping_victim;
  bind[30].is_null= 0;
  bind[30].length= 0;

  bind[31].buffer_type= MYSQL_TYPE_LONG;
  bind[31].buffer= (char *)&state->charging_move;
  bind[31].is_null= 0;
  bind[31].length= 0;

  bind[32].buffer_type= MYSQL_TYPE_LONG;
  bind[32].buffer= (char *)&state->temp_invulnerable;
  bind[32].is_null= 0;
  bind[32].length= 0;

  bind[33].buffer_type= MYSQL_TYPE_LONG;
  bind[33].buffer= (char *)&state->repeated_move_index;
  bind[33].is_null= 0;
  bind[33].length= 0;

  bind[34].buffer_type= MYSQL_TYPE_LONG;
  bind[34].buffer= (char *)&state->damage_taken;
  bind[34].is_null= 0;
  bind[34].length= 0;

  bind[35].buffer_type= MYSQL_TYPE_LONG;
  bind[35].buffer= (char *)&state->damage_is_counterable;
  bind[35].is_null= 0;
  bind[35].length= 0;

  bind[36].buffer_type= MYSQL_TYPE_LONG;
  bind[36].buffer= (char *)&state->last_move_used;
  bind[36].is_null= 0;
  bind[36].length= 0;
  
  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  *state_id = mysql_stmt_insert_id(stmt);

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
}

int get_battle_rules(int battle_id, int * rules)
{
  return get_battle_param(battle_id, "SELECT rules FROM openpkmn.Battles WHERE "
			  "ID = ?", rules);
}


#define FIRST_TYPE_STRING \
  "SELECT Type1 FROM PKMN_data WHERE id = ?" 
 int get_first_type(int pkmn_id,  int * type)
{
  return get_integer_parameter(pkmn_id, FIRST_TYPE_STRING, type);
}

#define SECOND_TYPE_STRING \
  "SELECT Type2 FROM PKMN_data WHERE id = ?" 
 int get_second_type(int pkmn_id,  int * type)
{
  return get_integer_parameter(pkmn_id, SECOND_TYPE_STRING, type);
}


#define MOVE_PARAMETER_STRING \
  "SELECT Parameter FROM Move_data WHERE id = ?" 
 int get_move_parameter(int move_id,  int * parameter)
{
  return get_integer_parameter(move_id, MOVE_PARAMETER_STRING, parameter);
}

#define MOVE_TYPE_STRING \
  "SELECT Type FROM Move_data WHERE id = ?" 
 int get_move_type(int move_id,  int * type)
{
  return get_integer_parameter(move_id, MOVE_TYPE_STRING, type);
}

#define MOVE_POWER_STRING \
  "SELECT Power FROM Move_data WHERE id = ?" 
 int get_move_power(int move_id,  int * power)
{
  return get_integer_parameter(move_id,
			       MOVE_POWER_STRING,
			       power);
}

#define MOVE_SECONDARY_EFFECT_STRING \
  "SELECT Secondary_effect FROM Move_data WHERE id = ?" 
 int get_move_secondary_effect(int move_id,  int * secondary_effect)
{
  return get_integer_parameter(move_id,
			       MOVE_SECONDARY_EFFECT_STRING,
			       secondary_effect);
}

#define MOVE_SECONDARY_CHANCE_STRING \
  "SELECT Secondary_chance FROM Move_data WHERE id = ?" 
 int get_move_secondary_chance(int move_id,  int * secondary_chance)
{
  return get_integer_parameter(move_id,
			       MOVE_SECONDARY_CHANCE_STRING,
			       secondary_chance);
}


int insert_battle_winner_loser(int battle_id, int winner_id, int loser_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[3];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  char * query_string = "UPDATE Battles SET winner_id = ?, "
    " loser_id = ? WHERE id = ? ";

  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare()\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&winner_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&loser_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&battle_id;
  bind[2].is_null= 0;
  bind[2].length= 0;

  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
  
}

int insert_battle_score(int battle_id, int challenger_left, int defender_left)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[3];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  char * query_string = "UPDATE Battles SET challenger_left = ?, "
    " defender_left = ? WHERE id = ? ";
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }
  
  stmt = mysql_stmt_init(&mysql);
  if (!stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      return DATABASE_ERROR;
    }
  if (mysql_stmt_prepare(stmt, query_string, strlen(query_string)))
    {
      fprintf(stderr, " mysql_stmt_prepare()\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&challenger_left;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&defender_left;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&battle_id;
  bind[2].is_null= 0;
  bind[2].length= 0;

  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the INSERT statement*/
  if (mysql_stmt_execute(stmt))
    {
      fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Close the statement */
  if (mysql_stmt_close(stmt))
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1; 
}
