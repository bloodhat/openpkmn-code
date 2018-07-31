#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../share/preferences.h"

#include "db_iface.h"


#define RANDOM_SPECIES_STATEMENT "SELECT DISTINCT PD.id, PD.Rules, PD.Name " \
  "FROM PKMN_data PD WHERE (PD.rules & ?) = ? AND PD.final_stage = 1 " \
  "AND id != 0 ORDER BY RAND() LIMIT 6"
#define RANDOM_SPECIES_STATEMENT_REPEAT "SELECT PD.id, PD.Rules, PD.name " \
  "FROM PKMN_data PD WHERE (PD.rules & ?) = ? AND PD.final_stage = 1 " \
  "AND id != 0 ORDER BY RAND() LIMIT 6"

int get_random_species_numbers(int pkmn_restriction, int repeat,
			       Species_ll ** species_numbers)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[2];
  MYSQL_BIND out_bind[3];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;

  int species;
  int ruleset;
  unsigned long name_len;
  char name[32];

  Species_ll * current = NULL;
  Species_ll * root = NULL;
  char * query_str = (repeat) ? RANDOM_SPECIES_STATEMENT_REPEAT :
    RANDOM_SPECIES_STATEMENT;
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
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
  
  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&pkmn_restriction;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;
  
  in_bind[1].buffer_type = MYSQL_TYPE_LONG;
  in_bind[1].buffer = (char *)&pkmn_restriction;
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
  out_bind[0].buffer= (char *)&species;
  out_bind[0].is_null= &is_null;
  out_bind[0].error= &error;
  
  out_bind[1].buffer_type= MYSQL_TYPE_LONG;
  out_bind[1].buffer= (char *)&ruleset;
  out_bind[1].is_null= &is_null;
  out_bind[1].error= &error;
  
  out_bind[2].buffer_type= MYSQL_TYPE_STRING;
  out_bind[2].buffer= (char *)name;
  out_bind[2].buffer_length= 32;
  out_bind[2].is_null= &is_null;
  out_bind[2].length= &name_len;
  out_bind[2].error = &error;

  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_users_teams: mysql_stmt_bind_result: %s\n",
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
     
      /* some fairly standard link list generation code here */
      if(root == NULL)
	{
	  current = root = malloc(sizeof(Species_ll));
	}
      else
	{
	  current->next = malloc(sizeof(Species_ll));
	  current = current->next;
	}
      current->species_num = species;
      current->ruleset = ruleset;
      strncpy(current->name, name, 32);
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  mysql_close(&mysql);
  *species_numbers = root;
  return 1;
}

#define RANDOM_MOVES_STATEMENT_NO_TM "SELECT DISTINCT MD.id, MD.Rules, MD.PP " \
  "FROM Move_data MD, Learnabilities L WHERE L.pkmn_id = ? AND " \
  "L.move_id = MD.id AND (MD.rules & ?) = ? AND L.method != 1 ORDER BY RAND() " \
  "LIMIT 4"

#define RANDOM_MOVES_STATEMENT "SELECT DISTINCT MD.id, MD.Rules, MD.PP " \
  "FROM Move_data MD, Learnabilities L WHERE L.pkmn_id = ? AND " \
  "L.move_id = MD.id AND (MD.rules & ?) = ? ORDER BY RAND() LIMIT 4"

#define RANDOM_MOVES_STATEMENT_ILLEGAL "SELECT DISTINCT MD.id, MD.Rules, " \
  "MD.PP FROM Move_data MD WHERE (? | 1) AND (MD.rules & ?) = ? " \
  "ORDER BY RAND() LIMIT 4"

int get_random_move_numbers(int species, int learning_restrictions,
			    int move_restrictions,
			    Move_record_ll ** move_numbers)
{
  MYSQL mysql;
  MYSQL_STMT * statement;
  MYSQL_BIND in_bind[3];
  MYSQL_BIND out_bind[3];
  
  char *server = DB_SERVER;
  char *user = DB_USER;
  char *password = DB_PASS;
  char *database = DB_DBASE;

  my_bool       is_null;
  my_bool       error;

  int move;
  int rules;
  int pp;

  Move_record_ll * current = NULL;
  Move_record_ll * root = NULL;
  char * query_str;
  char * query_str_illegal = RANDOM_MOVES_STATEMENT_ILLEGAL;
  char * query_str_no_tm = RANDOM_MOVES_STATEMENT_NO_TM;
  char * query_str_typical = RANDOM_MOVES_STATEMENT;

  if(!(learning_restrictions & ILLEGAL_MOVES_DISALLOWED))
    {
      query_str = query_str_illegal;
    }
  else if(learning_restrictions & TM_MOVES_DISALLOWED)
    {
      query_str = query_str_no_tm;
    }
  else
    {
      query_str = query_str_typical;
    }
  
  mysql_init(&mysql);

  if(!mysql_real_connect(&mysql, server, user, password,
			 database, 0, NULL, 0))
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

  in_bind[0].buffer_type = MYSQL_TYPE_LONG;
  in_bind[0].buffer = (char *)&species;
  in_bind[0].is_null = 0;
  in_bind[0].length = 0;
  
  in_bind[1].buffer_type = MYSQL_TYPE_LONG;
  in_bind[1].buffer = (char *)&move_restrictions;
  in_bind[1].is_null = 0;
  in_bind[1].length = 0;
  
  in_bind[2].buffer_type = MYSQL_TYPE_LONG;
  in_bind[2].buffer = (char *)&move_restrictions;
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

  memset(out_bind, 0, sizeof(out_bind));
  out_bind[0].buffer_type= MYSQL_TYPE_LONG;
  out_bind[0].buffer= (char *)&move;
  out_bind[0].is_null= &is_null;
  out_bind[0].error= &error;

  out_bind[1].buffer_type= MYSQL_TYPE_LONG;
  out_bind[1].buffer= (char *)&rules;
  out_bind[1].is_null= &is_null;
  out_bind[1].error= &error;

  out_bind[2].buffer_type= MYSQL_TYPE_LONG;
  out_bind[2].buffer= (char *)&pp;
  out_bind[2].is_null= &is_null;
  out_bind[2].error= &error;

  if(mysql_stmt_bind_result(statement, out_bind))
    {
      fprintf(stderr, "get_users_teams: mysql_stmt_bind_result: %s\n",
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
     
      /* some fairly standard link list generation code here */
      if(root == NULL)
	{
	  current = root = malloc(sizeof(Move_record_ll));
	}
      else
	{
	  current->next = malloc(sizeof(Move_record_ll));
	  current = current->next;
	}
      current->move_num = move;
      current->ruleset = rules;
      current->pp = pp;
    }
  
  if(mysql_stmt_close(statement) != 0)
    {
      fprintf(stderr, " failed while closing the statement\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(statement));
      return DATABASE_ERROR;
    }
  mysql_close(&mysql);
  *move_numbers = root;
  return 1;
}
