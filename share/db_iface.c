#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "db_iface.h"
#include "datatypes.h"
#include "encryption.h"
#include "preferences.h"

int get_team_rules(int user_id, int team_id, int * ruleset)
{
  
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[2];
  MYSQL_BIND out_bind[1];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;
  unsigned long length;
  int result_count = 0;

  char * statement_str = "SELECT ruleset FROM Teams WHERE id = ? AND "
    "owner_id = ?";
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  /* create binding to pull data */
  memset(in_bind, 0, sizeof(in_bind));
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&team_id;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;

  in_bind[1].buffer_type = MYSQL_TYPE_LONG;
  in_bind[1].buffer = (char *)&user_id;
  in_bind[1].is_null = 0;
  in_bind[1].length = 0;
  
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)ruleset;
  out_bind[0].is_null= &is_null;
  out_bind[0].length= &length;
  out_bind[0].error= &error;

  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_team_rules: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  fprintf(stdout, "get_team_rules: NULL id\n");
	  return NULL_RESULT;
	}
      if(error)
	{
	  fprintf(stdout, "get_team_rules: Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, "get_team_rules:  failed while closing the statement\n");
      fprintf(stderr, "get_team_rules: %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      fprintf(stderr, "found no matching teams with user %i id %i\n",
	      user_id, team_id);
      return NO_RESULTS;
    }

  mysql_close(&mysql);
  
  return 1;
}


#define USER_NAME_STATEMENT "SELECT name FROM openpkmn.Users WHERE ID = ?"

int get_user_param(char * user_name, char * statement_str, void * response)
{
  
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind;
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  unsigned long uname_len;

  my_bool       is_null;
  my_bool       error;
  unsigned long length;
  int result_count = 0;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "get_user_param: mysql_real_connect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str, strlen(statement_str)) != 0)
    {
      fprintf(stderr, "get_user_param: mysql_stmt_prepare: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  uname_len = strlen(user_name);
  /* create binding to pull data */
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_STRING;
  in_bind.buffer = user_name;
  in_bind.buffer_length = 32;
  in_bind.is_null = 0;
  in_bind.length = &uname_len;
  
  if(mysql_stmt_bind_param(statement, &in_bind))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_bind_param %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_execute %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(&out_bind, 0, sizeof(out_bind));
  out_bind.buffer_type= MYSQL_TYPE_LONG;
  out_bind.buffer= (char *)response;
  out_bind.is_null= &is_null;
  out_bind.length= &length;
  out_bind.error= &error;

  if(mysql_stmt_bind_result(statement, &out_bind))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  return NULL_RESULT;
	}
      if(error)
	{
	  fprintf(stdout, "get_user_param: Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, "get_user_param:  failed while closing the statement\n");
      fprintf(stderr, "get_user_param: %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      return NO_RESULTS;
    }

  mysql_close(&mysql);
  
  return 1;
}

int get_user_key_data(char * user_name, unsigned char * key_data)
{
  
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind;
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  unsigned long uname_len;

  my_bool       is_null;
  my_bool       error;
  unsigned long length;
  int result_count = 0;

  char hash_str[32];

  char * statement_str = "SELECT Key_data FROM Users WHERE Name = ?";
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "get_user_param: mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "get_user_param: mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  uname_len = strlen(user_name);
  /* create binding to pull data */
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_STRING;
  in_bind.buffer = user_name;
  in_bind.buffer_length = 32;
  in_bind.is_null = 0;
  in_bind.length = &uname_len;
  
  if(mysql_stmt_bind_param(statement, &in_bind))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(&out_bind, 0, sizeof(out_bind));
  out_bind.buffer_type= MYSQL_TYPE_STRING;
  out_bind.buffer= (char *)hash_str;
  out_bind.buffer_length= 32;
  out_bind.is_null= &is_null;
  out_bind.length= &length;
  out_bind.error = &error;

  if(mysql_stmt_bind_result(statement, &out_bind))
    {
      fprintf(stderr, "get_user_param: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  fprintf(stdout, "get_user_param: NULL id\n");
	  return NULL_RESULT;
	}
      if(error)
	{
	  fprintf(stdout, "get_user_param: Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, "get_user_param:  failed while closing the statement\n");
      fprintf(stderr, "get_user_param: %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      return INVALID_CREDENTIAL_ERROR;
    }
  
  md5_str_to_bytes(key_data, hash_str);
  
  mysql_close(&mysql);
  
  return 1;
}

int get_user_name_by_parameter(int id_param, char * statement_str,
			       char * user_name)
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
  unsigned long length;
  int result_count = 0;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "get_battle_participant_name: mysql_real_mysqlect: %s\n",
	      mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "get_battle_participant_name: mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.buffer = (char *)&id_param;
  in_bind.is_null = 0;
  in_bind.length = 0;
  
  if(mysql_stmt_bind_param(statement, &in_bind))
    {
      fprintf(stderr, "get_battle_participant_name: mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "get_battle_participant_name: mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  memset(&out_bind, 0, sizeof(out_bind));
  out_bind.buffer_type= MYSQL_TYPE_STRING;
  out_bind.buffer= (char *)user_name;
  out_bind.buffer_length = 32;
  out_bind.is_null= &is_null;
  out_bind.length= &length;
  out_bind.error= &error;

  if(mysql_stmt_bind_result(statement, &out_bind))
    {
      fprintf(stderr, "get_battle_participant_name: mysql_stmt_bind_result: "
	      "%s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  fprintf(stdout, "get_battle_participant_name: NULL id\n");
	  return NULL_RESULT;
	}
      if(error)
	{
	  fprintf(stdout, "get_battle_participant_name: Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, "get_battle_participant_name: failed while closing the statement\n");
      fprintf(stderr, "get_battle_participant_name: %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      fprintf(stderr, " no results %s\n", user_name);
      return NO_RESULTS;
    }

  mysql_close(&mysql);

  return 1;
}


int get_user_name(int user_id, char * user_name)
{
  return get_user_name_by_parameter(user_id, "SELECT name FROM "
				     "openpkmn.Users WHERE ID = ?", user_name);
}
#if 0
int get_battle_challenger_id(int battle_id, int * id)
{
  return get_battle_param(battle_id, "SELECT id FROM openpkmn.Users WHERE ID = "
			  "(SELECT challenger_id FROM openpkmn.Battles WHERE id"
			  " = ?)", id);
}

int get_battle_defender_id(int battle_id, int * id)
{
  return get_battle_param(battle_id, "SELECT id FROM openpkmn.Users WHERE ID = "
			  "(SELECT defender_id FROM openpkmn.Battles WHERE id ="
			  " ?)", id);
}
#endif

int get_user_id(char *user_name, int * user_id)
{
  return get_user_param(user_name, "SELECT ID FROM Users WHERE Name = ?",
			user_id);
}

int get_user_status(char *user_name, int * user_status)
{
  return get_user_param(user_name, "SELECT Status FROM Users WHERE Name = ?",
			user_status);
}

int get_user_current_team(char *user_name, int * user_team)
{
  return get_user_param(user_name, "SELECT Selected_team FROM Users WHERE "
			"Name = ?", user_team);
}

int get_user_selected_starter(char *user_name, int * starter)
{
  return get_user_param(user_name, "SELECT selected_starter FROM Users WHERE "
			"Name = ?", starter);
}

int get_user_rules(char *user_name, int * user_status)
{
  return get_user_param(user_name, "SELECT ruleset FROM Users WHERE Name = ?",
			user_status);
}


int get_user_current_team_rules(char *user_name, int * user_status)
{
  return get_user_param(user_name, "SELECT T.ruleset FROM Users U, Teams T "
			"WHERE U.Name = ? AND U.selected_team = T.id",
			user_status);
}

int get_user_key_type(char * user_name, int * key_type)
{
  return get_user_param(user_name, "SELECT Key_type FROM Users WHERE Name = ?",
			key_type);
}

#define USERS_PKMN_STATEMENT "SELECT  M.Pkmn_id, P.Species, M.Move_number, "\
" P.ruleset FROM Moves M, Users U, PKMN P " \
" WHERE U.id = ? AND U.id = P.Owner_id AND P.id = M.Pkmn_id AND P.visible = 1"

int get_users_pkmn(int user_id, Pkmn_ll ** pkmn, int * num_pkmn, int * num_moves)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind[4];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;
  int result_count = 0;

  int pkmn_id;
  int species;
  int move_num;
  int ruleset;

  int current_pkmn_id = -1;
  Pkmn_ll * current = NULL;
  Pkmn_ll * root = NULL;
  Move_ll * move_current;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, USERS_PKMN_STATEMENT,
			strlen(USERS_PKMN_STATEMENT)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.buffer = (char *)&user_id;
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

  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)&pkmn_id;
  out_bind[0].is_null= &is_null;
  out_bind[0].error= &error;

  out_bind[1].buffer_type= MYSQL_TYPE_LONG;
  out_bind[1].buffer= (char *)&species;
  out_bind[1].is_null= &is_null;
  out_bind[1].error= &error;

  
  out_bind[2].buffer_type= MYSQL_TYPE_LONG;
  out_bind[2].buffer= (char *)&move_num;
  out_bind[2].is_null= &is_null;
  out_bind[2].error= &error;

  out_bind[3].buffer_type= MYSQL_TYPE_LONG;
  out_bind[3].buffer= (char *)&ruleset;
  out_bind[3].is_null= &is_null;
  out_bind[3].error= &error;
  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_users_pkmn: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  *num_pkmn = 0;
  *num_moves = 0;
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
     
      /* some fairly standard link list generation code here */
      /* new pkmn here */
      if(pkmn_id != current_pkmn_id)
	{
	  (*num_pkmn)++;
	  current_pkmn_id = pkmn_id;
	  if(root == NULL)
	    {
	      current = root = malloc(sizeof(Pkmn_ll));
	    }
	  else
	    {
	      current->next = malloc(sizeof(Pkmn_ll));
	      current = current->next;
	    }
	  current->species = species;
	  current->index = pkmn_id;
	  current->ruleset = ruleset;
	  current->moves = NULL;
	  current->next = NULL;
	}
      if(current->moves == NULL)
	{
	  move_current = current->moves
	    = malloc(sizeof(Move_ll));
	}
      else
	{
	  move_current->next = malloc(sizeof(Move_ll));
	  move_current = move_current->next;
	}
      (*num_moves)++;
      move_current->move_num = move_num;
      move_current->next = NULL;
      
      result_count++;
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


#define USERS_TEAMS_STATEMENT "SELECT  T.id, TP.pkmn_id, "\
" T.ruleset FROM Teams T, Users U, Team_PKMN TP  WHERE U.id = ? AND " \
"U.id = T.owner_id AND TP.team_id = T.id AND T.visible = 1"

int get_users_teams(int user_id, Team_ll ** team, int * num_teams,
		    int * num_members)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind[3];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;
  int result_count = 0;

  int team_id;
  int member_id;
  int ruleset;

  int current_team_id = -1;
  Team_ll * current = NULL;
  Team_ll * root = NULL;
  Member_ll * member_current;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, USERS_TEAMS_STATEMENT,
			strlen(USERS_TEAMS_STATEMENT)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.buffer = (char *)&user_id;
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

  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)&team_id;
  out_bind[0].is_null= &is_null;
  out_bind[0].error= &error;

  out_bind[1].buffer_type= MYSQL_TYPE_LONG;
  out_bind[1].buffer= (char *)&member_id;
  out_bind[1].is_null= &is_null;
  out_bind[1].error= &error;

  out_bind[2].buffer_type= MYSQL_TYPE_LONG;
  out_bind[2].buffer= (char *)&ruleset;
  out_bind[2].is_null= &is_null;
  out_bind[2].error= &error;

  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_users_teams: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  *num_teams = 0;
  *num_members = 0;
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
     
      /* some fairly standard link list generation code here */
      
      if(team_id != current_team_id)
	{
	  (*num_teams)++;
	  current_team_id = team_id;
	  if(root == NULL)
	    {
	      current = root = malloc(sizeof(Team_ll));
	    }
	  else
	    {
	      current->next = malloc(sizeof(Team_ll));
	      current = current->next;
	    }
	  current->index = team_id;
	  current->ruleset = ruleset;
	  current->members = NULL;
	  current->next = NULL;
	}
      if(current->members == NULL)
	{
	  member_current = current->members
	    = malloc(sizeof(Member_ll));
	}
      else
	{
	  member_current->next = malloc(sizeof(Member_ll));
	  member_current = member_current->next;
	}
      (*num_members)++;
      member_current->index = member_id;
      member_current->next = NULL;
      
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    
}
  mysql_close(&mysql);
  *team = root;
  return 1;
}


int get_users(char * query_str, int user_id, User_ll ** users,
	      int * num_users)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind;
  MYSQL_BIND out_bind[5];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null[5];
  my_bool       error[5];
  int result_count = 0;

  int active_user_id;
  int active_user_rules;
  int active_user_status;
  int active_user_team_rules;
  
  int current_user_id = -1;
  char user_name[32];
  unsigned long user_name_len;
  User_ll * current = NULL;
  User_ll * root = NULL;
    
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, query_str, strlen(query_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(&in_bind, 0, sizeof(in_bind));
  
  in_bind.buffer_type = MYSQL_TYPE_LONG;
  in_bind.buffer = (char *)&user_id;
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

  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)&active_user_id;
  out_bind[0].is_null= &(is_null[0]);
  out_bind[0].error= &(error[0]);

  out_bind[1].buffer_type= MYSQL_TYPE_STRING;
  out_bind[1].buffer= (char *)&user_name;
  out_bind[1].buffer_length = 32;
  out_bind[1].is_null= &(is_null[1]);
  out_bind[1].length = &user_name_len;
  out_bind[1].error= &(error[1]);
  
  out_bind[2].buffer_type= MYSQL_TYPE_LONG;
  out_bind[2].buffer= (char *)&active_user_rules;
  out_bind[2].is_null= &(is_null[2]);
  out_bind[2].error= &(error[2]);

  out_bind[3].buffer_type= MYSQL_TYPE_LONG;
  out_bind[3].buffer= (char *)&active_user_status;
  out_bind[3].is_null= &(is_null[3]);
  out_bind[3].error= &(error[3]);

  out_bind[4].buffer_type= MYSQL_TYPE_LONG;
  out_bind[4].buffer= (char *)&active_user_team_rules;
  out_bind[4].is_null= &(is_null[4]);
  out_bind[4].error= &(error[4]);
  
  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_active_users: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  *num_users = 0;
  while(!mysql_stmt_fetch(statement))
    {
      /* we don't care about null team rules */
      if(is_null[0] || is_null[1] || is_null[2] || is_null[3])
	{
	  fprintf(stdout, "NULL id\n");
	  return NULL_RESULT;
	}
      if(error[0] || error[1] || error[2] || error[3] || error[4])
	{
	  fprintf(stdout, "Error\n");
	  return DATABASE_ERROR;
	}
     
      /* some fairly standard link list generation code here */
      
      /* new user here (this is vacuous since user ids are keys, but
	 just in case there are db problems) */
      if(active_user_id != current_user_id)
	{
	  (*num_users)++;
	  current_user_id = active_user_id;
	  if(root == NULL)
	    {
	      current = root = malloc(sizeof(User_ll));
	    }
	  else
	    {
	      current->next = malloc(sizeof(User_ll));
	      current = current->next;
	    }
	  current->id = active_user_id;
	  current->ruleset = active_user_rules;
	  memset(current->user_name, 0, 32);
	  strncpy(current->user_name, user_name, 32);
	  current->status = active_user_status;
	  if(!is_null[4])
	    {
	      current->team_rules = active_user_team_rules;
	    }
	  else
	    {
	      current->team_rules = -1;
	    }
	  current->next = NULL;
	}      
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }

  mysql_close(&mysql);
  *users = root;
  return 1;
}


#define BATTLEABLE_USERS_STATEMENT "SELECT  Them.id, Them.name, Them.ruleset"\
  " FROM Users Me, Users Them, Teams TheirTeam WHERE status = '1'"\
  " AND Them.id <> ? AND Me.id = ? AND (0x3 & Me.ruleset) = " \
  "(0x3 & Them.ruleset) AND TheirTeam.id = Them.selected_team AND" \
  " (0xFFFFF3FC & Me.ruleset) = (0xFFFFF3FC & TheirTeam.ruleset)"

#define ACTIVE_USERS_STATEMENT "SELECT  U.id, U.name, U.ruleset, U.status, " \
  " T.ruleset FROM Users U LEFT JOIN Teams T ON U.selected_team = T.id " \
  " WHERE U.status <> '0' "

int get_active_users(int user_id, User_ll ** users, int * num_users)
{
  return get_users(ACTIVE_USERS_STATEMENT, user_id, users, num_users);
}

int get_battleable_users(int user_id, User_ll ** users, int * num_users)
{
  return get_users(BATTLEABLE_USERS_STATEMENT, user_id, users, num_users);
}

int set_boolean_parameter(int id, char *statement_str, int value, int how_many)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND * in_bind;
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  int i;
  
  in_bind = malloc(sizeof(*in_bind) * (1 + how_many));

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  /* create binding to pull data */
  memset(in_bind, 0, sizeof(in_bind));

  for(i = 0; i < how_many; i++)
    {
      in_bind[i].buffer_type = MYSQL_TYPE_TINY;
      in_bind[i].buffer = (char *)&value;
      in_bind[i].is_null = 0;
      in_bind[i].length = 0;
    }
  
  in_bind[i].buffer_type = MYSQL_TYPE_LONG;
  in_bind[i].buffer = (char *)&id;
  in_bind[i].is_null = 0;
  in_bind[i].length = 0;
  
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if (mysql_stmt_close(statement))
    {
      fprintf(stderr, " failed while closing the statement %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  free(in_bind);
  return 1;
}

int set_pkmn_invisible(int pkmn_id)
{
  /* Turn a pkmn and any of the teams that it may be on invisible.
     The left outer join will make the pkmn invisible even if it is on no
     team */
#define PKMN_INVISIBLE_STATEMENT_STRING  \
  "UPDATE (PKMN P LEFT OUTER JOIN (Teams T INNER JOIN Team_PKMN TP ON " \
  "T.id = TP.team_id) ON TP.pkmn_id = P.id) SET P.visible = ?, T.visible = ? "\
  " WHERE P.id = ?"
  
  return set_boolean_parameter(pkmn_id, PKMN_INVISIBLE_STATEMENT_STRING, 0, 2);
}

int set_team_invisible(int team_id)
{
#define TEAM_INVISIBLE_STATEMENT_STRING  \
  "UPDATE Teams SET visible = ? WHERE ID = ?" 
  return set_boolean_parameter(team_id, TEAM_INVISIBLE_STATEMENT_STRING, 0, 1);
}

int set_user_param(char * user_name, char * statement_str,
		   int status)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[2];
  
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  unsigned long uname_len;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password, database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str, strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  uname_len = strlen(user_name);

  /* create binding to pull data */
  memset(in_bind, 0, sizeof(in_bind));
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&status;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;
  
  in_bind[1].buffer_type = MYSQL_TYPE_STRING;
  in_bind[1].buffer = user_name;
  in_bind[1].buffer_length = 32;
  in_bind[1].is_null = 0;
  in_bind[1].length = &uname_len;
  
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if (mysql_stmt_close(statement))
    {
      fprintf(stderr, " failed while closing the statement %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  mysql_close(&mysql);
  
  return 1;
}


int insert_pkmn(int owner_id, int species, char * nickname,
		int level, int max_hp, int attack, int defense,
		int speed, int special, int rules, int * pkmn_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[10];
  unsigned long nick_len;
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO openpkmn.PKMN(species, owner_id, "
    "nickname, level, max_hp, attack, defense, speed, special, ruleset, "
    "visible)  VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1)";

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
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&species;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&owner_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  /* string needs to pass length */
  nick_len = strlen(nickname);
  
  /* STRING PARAM */
  bind[2].buffer_type= MYSQL_TYPE_STRING;
  bind[2].buffer= (char *)nickname;
  bind[2].buffer_length= 32;
  bind[2].is_null= 0;
  bind[2].length= &nick_len;

  /* more int params */
  bind[3].buffer_type= MYSQL_TYPE_LONG;
  bind[3].buffer= (char *)&level;
  bind[3].is_null= 0;
  bind[3].length= 0;

  bind[4].buffer_type= MYSQL_TYPE_LONG;
  bind[4].buffer= (char *)&max_hp;
  bind[4].is_null= 0;
  bind[4].length= 0;

  bind[5].buffer_type= MYSQL_TYPE_LONG;
  bind[5].buffer= (char *)&attack;
  bind[5].is_null= 0;
  bind[5].length= 0;

  bind[6].buffer_type= MYSQL_TYPE_LONG;
  bind[6].buffer= (char *)&defense;
  bind[6].is_null= 0;
  bind[6].length= 0;

  bind[7].buffer_type= MYSQL_TYPE_LONG;
  bind[7].buffer= (char *)&speed;
  bind[7].is_null= 0;
  bind[7].length= 0;

  bind[8].buffer_type= MYSQL_TYPE_LONG;
  bind[8].buffer= (char *)&special;
  bind[8].is_null= 0;
  bind[8].length= 0;

  bind[9].buffer_type= MYSQL_TYPE_LONG;
  bind[9].buffer= (char *)&rules;
  bind[9].is_null= 0;
  bind[9].length= 0;

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

  *pkmn_id = mysql_stmt_insert_id(stmt);

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


int set_user_selected_team(char * user_name, int team_id)
{
  return set_user_param(user_name," UPDATE Users SET selected_team = ? WHERE "
			"Name = ?", team_id);
}

int set_user_selected_starter(char * user_name, int starter)
{
  return set_user_param(user_name, "UPDATE Users SET selected_starter = ? "
			"WHERE Name = ?", starter);
}

int set_user_status(char * user_name, int new_status)
{
  return set_user_param(user_name, "UPDATE Users SET Status = ? WHERE Name = ?",
			new_status);
}

int set_user_ruleset(char * user_name, int new_ruleset)
{
  return set_user_param(user_name, "UPDATE Users SET Ruleset = ? WHERE "
			"Name = ?", new_ruleset);
}


int insert_move(int owner_id, int pkmn_id, int number, int pp)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[4];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO openpkmn.Moves(move_number, max_pp, \
owner_id, pkmn_id) VALUES(?, ?, ?, ?)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,  database, 0, NULL, 0))
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
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&number;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&pp;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&owner_id;
  bind[2].is_null= 0;
  bind[2].length= 0;

  bind[3].buffer_type= MYSQL_TYPE_LONG;
  bind[3].buffer= (char *)&pkmn_id;
  bind[3].is_null= 0;
  bind[3].length= 0;

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

int insert_team(int owner_id, int * team_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[1];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO openpkmn.Teams(owner_id, visible) "
    "VALUES(?, 1)";

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,  database, 0, NULL, 0))
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
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&owner_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

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

  *team_id = mysql_stmt_insert_id(stmt);

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

int insert_team_pkmn(int team_id, int pkmn_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[2];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;
  
  char * query_string = "INSERT INTO "
    "openpkmn.Team_PKMN(team_id, pkmn_id) VALUES(?, ?)";

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
      fprintf(stderr, " insert_team_pkmn -> mysql_stmt_prepare()"
	      ", INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }
  memset(bind, 0, sizeof(bind));

  /* INTEGER PARAM */
  /* This is a number type, so there is no need 
     to specify buffer_length */
  bind[0].buffer_type= MYSQL_TYPE_LONG;
  bind[0].buffer= (char *)&team_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&pkmn_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

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

int insert_battle(int challenger_id, int defender_id,
		  int challenger_team_id, int challenger_starter,
		  int defender_team_id, int defender_starter, int * battle_id)
{
  MYSQL mysql;
  MYSQL_STMT    *stmt;
  MYSQL_BIND    bind[9];
 
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  char * query_string;
  char * proto_query_string = "INSERT INTO openpkmn.Battles(challenger_id, "
    "defender_id, challenger_team_id, defender_team_id, challenger_starter, "
    "defender_starter, rules) VALUES(?, ?, ?, ?, ?, ?, "
    "(((SELECT ruleset FROM Teams WHERE ID = ?) & (SELECT ruleset "
    "FROM Teams WHERE ID = ?)) | ((SELECT ruleset FROM Users WHERE ID = ?) "
    "& (%i | %i))))";

  /* make a string large enough to hold our string with the clauses
     substituted in (assume we could have 64 bit insertions expressed
     in binary to be stupidly safe) */
  query_string = (char *)malloc(strlen(proto_query_string) + 128);

  sprintf(query_string, proto_query_string, SLEEP_CLAUSE, FREEZE_CLAUSE);

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
  bind[0].buffer= (char *)&challenger_id;
  bind[0].is_null= 0;
  bind[0].length= 0;

  bind[1].buffer_type= MYSQL_TYPE_LONG;
  bind[1].buffer= (char *)&defender_id;
  bind[1].is_null= 0;
  bind[1].length= 0;

  bind[2].buffer_type= MYSQL_TYPE_LONG;
  bind[2].buffer= (char *)&challenger_team_id;
  bind[2].is_null= 0;
  bind[2].length= 0;

  bind[3].buffer_type= MYSQL_TYPE_LONG;
  bind[3].buffer= (char *)&defender_team_id;
  bind[3].is_null= 0;
  bind[3].length= 0;

  bind[4].buffer_type= MYSQL_TYPE_LONG;
  bind[4].buffer= (char *)&challenger_starter;
  bind[4].is_null= 0;
  bind[4].length= 0;

  bind[5].buffer_type= MYSQL_TYPE_LONG;
  bind[5].buffer= (char *)&defender_starter;
  bind[5].is_null= 0;
  bind[5].length= 0;

  bind[6].buffer_type= MYSQL_TYPE_LONG;
  bind[6].buffer= (char *)&challenger_team_id;
  bind[6].is_null= 0;
  bind[6].length= 0;

  bind[7].buffer_type= MYSQL_TYPE_LONG;
  bind[7].buffer= (char *)&defender_team_id;
  bind[7].is_null= 0;
  bind[7].length= 0;

  bind[8].buffer_type= MYSQL_TYPE_LONG;
  bind[8].buffer= (char *)&challenger_id;
  bind[8].is_null= 0;
  bind[8].length= 0;


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

  *battle_id = mysql_stmt_insert_id(stmt);

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

int get_integer_parameter(int id, char * statement_str,
			   int * ret_val)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[1];
  MYSQL_BIND out_bind[1];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  int result_count = 0;

  my_bool is_null, error;

  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  /* create binding to pull data */
  memset(in_bind, 0, sizeof(in_bind));
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&id;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;
  
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)ret_val;
  out_bind[0].is_null= &is_null;
  out_bind[0].error= &error;
  
  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_integer_param: mysql_stmt_bind_result: %s\n",
	      mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  while(!mysql_stmt_fetch(statement))
    {
      if(is_null)
	{
	  fprintf(stderr, "get_base_stat: NULL stat\n");
	  return INVALID_CREDENTIAL_ERROR;
	}
      if(error)
	{
	  fprintf(stderr, "get_base_stat: Error\n");
	  return DATABASE_ERROR;
	}
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, "get_integer_param:  failed while closing the "
	      "statement\n");
      fprintf(stderr, "get_integer_param: %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count == 0)
    {
      fprintf(stderr, "found no base stat for id %i\n", id);
      return RECOVERABLE_ERROR;
    }

  mysql_close(&mysql);
  
  return 1;
}


#define BASE_HP_STATEMENT_STRING \
  "SELECT base_hp FROM PKMN_data WHERE id = ?" 
 int get_base_hp(int pkmn_id,  int * base)
{
  return get_integer_parameter(pkmn_id, BASE_HP_STATEMENT_STRING, base);
}

#define BASE_ATTACK_STATEMENT_STRING \
  "SELECT base_attack FROM PKMN_data WHERE id = ?" 
 int get_base_attack(int pkmn_id,  int * base)
{
  return get_integer_parameter(pkmn_id, BASE_ATTACK_STATEMENT_STRING, base);
}


#define BASE_DEFENSE_STATEMENT_STRING \
  "SELECT base_defense FROM PKMN_data WHERE id = ?" 
 int get_base_defense(int pkmn_id,  int * base)
{
  return get_integer_parameter(pkmn_id, BASE_DEFENSE_STATEMENT_STRING, base);
}

#define BASE_SPEED_STATEMENT_STRING \
  "SELECT base_speed FROM PKMN_data WHERE id = ?" 
 int get_base_speed(int pkmn_id,  int * base)
{
  return get_integer_parameter(pkmn_id, BASE_SPEED_STATEMENT_STRING, base);
}

#define BASE_SPECIAL_STATEMENT_STRING \
  "SELECT base_special FROM PKMN_data WHERE id = ?" 
 int get_base_special(int pkmn_id,  int * base)
{
  return get_integer_parameter(pkmn_id, BASE_SPECIAL_STATEMENT_STRING, base);
}

#define MOVE_PP_STRING \
  "SELECT PP FROM Move_data WHERE id = ?" 
 int get_move_pp(int move_id,  int * pp)
{
  return get_integer_parameter(move_id,
			       MOVE_PP_STRING,
			       pp);
}

#define MOVE_ACCURACY_STRING \
  "SELECT Accuracy FROM Move_data WHERE id = ?" 
 int get_move_accuracy(int move_id,  int * accuracy)
{
  return get_integer_parameter(move_id,
			       MOVE_ACCURACY_STRING,
			       accuracy);
}

int move_learned_by_method(int pkmn_id, int move_num, int method,
			   int * found)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[3];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  int result_count = 0;

  char * statement_str = "SELECT * FROM Learnabilities WHERE \
    pkmn_id = ? AND move_id = ? AND method = ?";
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
    {
      fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
   }

  statement = mysql_stmt_init(&mysql);

  if(mysql_stmt_prepare(statement, statement_str,
			strlen(statement_str)) != 0)
    {
      fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  
  /* create binding to pull data */
  memset(in_bind, 0, sizeof(in_bind));
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&pkmn_id;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;
  
  in_bind[1].buffer_type = MYSQL_TYPE_LONG;
  in_bind[1].buffer = (char *)&move_num;
  in_bind[1].is_null = 0;
  in_bind[1].length = 0;

  in_bind[2].buffer_type = MYSQL_TYPE_LONG;
  in_bind[2].buffer = (char *)&method;
  in_bind[2].is_null = 0;
  in_bind[2].length = 0;
    
  if(mysql_stmt_bind_param(statement, in_bind))
    {
      fprintf(stderr, "mysql_stmt_bind_param %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }

  if(mysql_stmt_execute(statement))
    {
      fprintf(stderr, "mysql_stmt_execute %s\n", mysql_error(&mysql));
      return DATABASE_ERROR;
    }
  
  while(!mysql_stmt_fetch(statement))
    {
      result_count++;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  
  if(result_count > 0)
    {
      *found = 1;
    }
  else
    {
      *found = 0;
    }
  
  mysql_close(&mysql);
  return 1;
  
}
 
int insert_ruleset(int team_id)
{
  MYSQL mysql;
  MYSQL_STMT * stmt;
  MYSQL_BIND in_bind[3];
  
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  char * query_string = "UPDATE Teams T SET ruleset = "
    "(((SELECT COUNT(*) = 0 FROM (SELECT species FROM Team_PKMN TP, PKMN P "
    "WHERE P.id = TP.pkmn_id AND TP.team_id = ? GROUP BY species "
    "HAVING (COUNT(species) > 1)) AS X) * 0x0800) | "
    "(SELECT BIT_AND(P.ruleset) FROM Team_PKMN TP, PKMN P "
    "WHERE P.id = TP.pkmn_id AND TP.team_id = ?)) WHERE T.id = ?";
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
  memset(in_bind, 0, sizeof(in_bind));

  /* INTEGER PARAM */
  /* This is  number type, so there is no need 
     to specify buffer_length */
  in_bind[0].buffer_type= MYSQL_TYPE_LONG;
  in_bind[0].buffer= (char *)&team_id;
  in_bind[0].is_null= 0;
  in_bind[0].length= 0;

  in_bind[1].buffer_type= MYSQL_TYPE_LONG;
  in_bind[1].buffer= (char *)&team_id;
  in_bind[1].is_null= 0;
  in_bind[1].length= 0;

  in_bind[2].buffer_type= MYSQL_TYPE_LONG;
  in_bind[2].buffer= (char *)&team_id;
  in_bind[2].is_null= 0;
  in_bind[2].length= 0;
  
  /* Bind the buffers */
  if (mysql_stmt_bind_param(stmt, in_bind))
    {
      fprintf(stderr, " mysql_stmt_bind_param() failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
      return DATABASE_ERROR;
    }

  /* Execute the update statement*/
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
