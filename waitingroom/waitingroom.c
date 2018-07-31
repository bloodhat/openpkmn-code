#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../share/config.h"
#include "../share/entropy.h"
#include "../share/error.h"
#include "../share/netutil.h"
#include "../share/packet.h"
#include "../share/encryption.h"

#include "../share/preferences.h"

#include "datatypes.h"
#include "packet.h"
#include "db_iface.h"

void broadcast_user_change(char * updated_user_name,  int new)
{
  char recipient_name[32];
  int i;
  int ret_val;
  User * recipient;
  int updated_id;
  int updated_rules;
  int updated_status;
  int updated_team_rules;

  if(get_user_id(updated_user_name, &updated_id) > 0 &&
     get_user_rules(updated_user_name, &updated_rules) > 0 &&
     get_user_status(updated_user_name, &updated_status))
    {
      ret_val = get_user_current_team_rules(updated_user_name,
					    &updated_team_rules);
      if(ret_val == NO_RESULTS || ret_val == NULL_RESULT)
	{
	  updated_team_rules = -1;
	}

      for(i = 0; i < NUM_USERS; i++)
	{
	  
	  recipient = users[i];
	  if(recipient != NULL)
	    {
	      if(get_user_name(i, recipient_name) < 0)
		{
		}
	      else
		{
		  send_user_change_packet(recipient->chat_sock,
					  recipient_name, updated_id,
					  updated_user_name,
					  updated_rules, updated_status,
					  updated_team_rules, new);
		}
	    }
	}
    }
}


int create_random_pkmn(int user_id, int move_restrictions,
		       int learning_restrictions, int species, char  * nickname,
		       int species_rules, int * pkmn_id, int * adherent_rules)
{
  Move_record_ll * node;
  Move_record_ll * head;
  int max_hp, max_attack, max_defense, max_speed, max_special;
  int num_moves = 0;
  int move_numbers[4];
  int move_pps[4];
  int i;
  
  /* Set adherent to all rules. */
  *adherent_rules = (HOLDS_TO_ILLEGAL_MOVES_DISALLOWED |
		     HOLDS_TO_TM_MOVES_DISALLOWED |
		     HOLDS_TO_GS_MOVES_DISALLOWED |
		     HOLDS_TO_OHKO_MOVES_DISALLOWED |
		     HOLDS_TO_EVADE_MOVES_DISALLOWED |
		     HOLDS_TO_MEWTWO_DISALLOWED |
		     HOLDS_TO_MEW_DISALLOWED |
		     HOLDS_TO_ILLEGAL_STATS_DISALLOWED |
		     HOLDS_TO_ILLEGAL_LEVELS_DISALLOWED);
  /* Make adherent to only rules that PKMN itself is */
  *adherent_rules &= species_rules;
  
  if(get_random_move_numbers(species, learning_restrictions,
			     move_restrictions, &head) < 0)
    {
      return -1;
    }
  else
    {
      node = head;
      while(node != NULL)
	{
	  move_numbers[num_moves] = node->move_num;
	  move_pps[num_moves] = node->pp;
	  *adherent_rules &= node->ruleset;
	  num_moves++;
	  node = node->next;
	}
      if(get_max_hp(species, &max_hp) < 0)
	{
	  return -1;
	}
      if(get_max_attack(species, &max_attack) < 0)
	{
	  return -1;
	}
      if(get_max_defense(species, &max_defense) < 0)
	{
	  return -1;
	}
      if(get_max_speed(species, &max_speed) < 0)
	{
	  return -1;
	}
      if(get_max_special(species, &max_special) < 0)
	{
	  return -1;
	}
      
      if(insert_pkmn(user_id, species, nickname, 100, max_hp, max_attack,
		     max_defense, max_speed, max_special, *adherent_rules,
		     pkmn_id) < 0)
	{
	  for(i = 0; i < num_moves; i++)
	    {
	      insert_move(user_id, *pkmn_id, move_numbers[i], move_pps[i]);
	    }
	}
      else
	{
	  return -1;
	}
    }
  return 1;
}

int create_random_team(int user_id, int rules, int * team_id)
{
  Species_ll * node;
  Species_ll * head;
  int pkmn_restrictions =  rules &  PKMN_RESTRICTIONS;
  int move_restrictions = rules & MOVE_RESTRICTIONS;
  int learning_restrictions = rules & LEARNING_RESTRICTIONS;
  int adherent_rules = (HOLDS_TO_ILLEGAL_MOVES_DISALLOWED |
			HOLDS_TO_TM_MOVES_DISALLOWED |
			HOLDS_TO_GS_MOVES_DISALLOWED |
			HOLDS_TO_OHKO_MOVES_DISALLOWED |
			HOLDS_TO_EVADE_MOVES_DISALLOWED |
			HOLDS_TO_MEWTWO_DISALLOWED |
			HOLDS_TO_MEW_DISALLOWED |
			HOLDS_TO_ILLEGAL_STATS_DISALLOWED |
			HOLDS_TO_ILLEGAL_LEVELS_DISALLOWED);

  int pkmn_id;
  int pkmn_adherent_rules;

  if(insert_team(user_id, team_id) < 0)
    {
      fprintf(stderr, "*** could not insert team\n");
      return -1;
    }
  else
    {
      if(get_random_species_numbers(pkmn_restrictions,
				    !(rules & DUPLICATE_PKMN_DISALLOWED),
				    &head) < 0)
	{
	  fprintf(stderr, "*** could not get random species list\n");
	  return -1;
	}
      
      node = head;
      while(node != NULL)
	{
	  if(create_random_pkmn(user_id, move_restrictions,
				learning_restrictions, node->species_num,
				node->name, node->ruleset, &pkmn_id,
				&pkmn_adherent_rules) < 0)
	    {
	      fprintf(stderr, "*** could not insert team\n");
	      return -1;
	    }
	  else
	    {
	      adherent_rules &= pkmn_adherent_rules;
	      if(insert_team_pkmn(*team_id, pkmn_id) < 0)
		{
		  fprintf(stderr, "*** could not insert team\n");
		  return -1;
		}
	      else
		{
		  
		}
	    }
	  node = node->next;
	}
      if(insert_ruleset(*team_id) < 0)
	{
	  return -1;
	}
    }
  return 1;
}



int verify_user_status(char * user_name, int test_status)
{
  int user_status = -1;
  if(get_user_status(user_name, &user_status) < 0)
    {
      fprintf(stderr, "Bad user id\n");
      return -1;
    }
  else /* could get user status */
    {
      if(user_status == test_status)
	{
	  return 1;
	}
      else
	{
	  fprintf(stderr, "user has incorrect status\n");
	  return -1;
	}
    }
}

int verify_user_status_not(char * user_name, int test_status)
{
  int user_status = -1;
  if(get_user_status(user_name, &user_status) < 0)
    {
      fprintf(stderr, "Bad user id\n");
      return -1;
    }
  else /* could get user status */
    {
      if(user_status == test_status)
	{
	  fprintf(stderr, "user has incorrect status\n");
	  return -1;
	}
      else
	{
	  return 1;
	}
    }
}

int terminate_session(char * user_name)
{
  int ret = set_user_status(user_name, USER_STATUS_OFFLINE);
  /* log them out! */
 broadcast_user_change(user_name, 0);
 return ret;
}

User * initialize_user(int sock)
{
  User * user = malloc(sizeof(User));
  if(user != NULL)
    {
      user->chat_sock = sock;
      pthread_mutex_init(&user->user_lock, NULL);
      pthread_cond_init(&user->user_sig, NULL);
    }
  else
    {
      fprintf(stderr, "could not allocate user\n");
    }
  return user;
}

int extract_team(int * team_data, int num_members, int user_id, int * team_id)
{
  int i;
  if(insert_team(user_id, team_id) < 0)
    {
      return DATABASE_ERROR;
    }
  else
    {
      for(i = 0; i < num_members; i++)
	{
	  if(insert_team_pkmn(*team_id, ntohl(team_data[i])) < 0)
	    {
	      return DATABASE_ERROR;
	    }
	}
      insert_ruleset(*team_id);
    }
  return 1;
}

int bind_user_to_team(int sock, char * user_name, int user_id,
		      int team_id, int starter)
{
  int team_rules;
    /* confirm that this team exists, get the rules while
       we're at it */
  if(get_team_rules(user_id, team_id, &team_rules) < 0)
    {
      fprintf(stderr, "team does not exist\n");
      return -1;
    }
  else
    {
      /* set the team to be used */
      if(set_user_selected_team(user_name, team_id) < 0)
	{
	  fprintf(stderr, "could not set selected team\n");
	  return -1;
	}
      else
	{
	  if(set_user_selected_starter(user_name, starter) < 0)
	    {
	      fprintf(stderr, "could not set starter");
	      return -1;
	    }
	}
      return 1;
    }
}

int opponent_is_compatible(char * user_name, char * opponent_name)
{
#define TEAM_TO_USER_MATCHES (MEW_DISALLOWED | MEWTWO_DISALLOWED|\
			      EVADE_MOVES_DISALLOWED | OHKO_MOVES_DISALLOWED | \
			      GS_MOVES_DISALLOWED | TM_MOVES_DISALLOWED | \
			      ILLEGAL_MOVES_DISALLOWED | \
			      DUPLICATE_PKMN_DISALLOWED)
  int my_user_rules;
  int my_team_rules;
  int their_user_rules;
  int their_team_rules;
  int test_bit;
  unsigned int i;

  if(get_user_rules(user_name, &my_user_rules) < 0 ||
     get_user_rules(opponent_name, &their_user_rules) < 0)
    {
      fprintf(stderr, "could not get rules\n");
      return -1;
    }
  /* check that the non-team clauses are compatible */
  if((my_user_rules & (SLEEP_CLAUSE | FREEZE_CLAUSE)) !=
     (their_user_rules & (SLEEP_CLAUSE | FREEZE_CLAUSE)))
    {
      fprintf(stderr, "users do not match on non-team clauses\n");
      return -1;
    }
  if(get_user_current_team_rules(user_name, &my_team_rules) < 0 ||
     get_user_current_team_rules(opponent_name, &their_team_rules) < 0)
    {
      fprintf(stderr, "could not get team rules\n");
      return -1;
    }
  /* check that we can battle against the other team */
  my_user_rules = my_user_rules & (TEAM_TO_USER_MATCHES);
  their_user_rules = their_user_rules & (TEAM_TO_USER_MATCHES);
  for(i = 0; i < (sizeof(my_user_rules) * 8); i++)
    {
      test_bit = (1 << i);
      fprintf(stderr, "testing bit %i\n", i);
      if(!(((my_user_rules & test_bit) == 0) ||
	 ((my_user_rules & test_bit) ==
	  (their_team_rules & test_bit))))
	{
	  fprintf(stderr, "challenger rule (%i), opponent's team (%i) mismatch"
		  ": bit %i\n", my_user_rules,
		  their_team_rules, i);
	  return -1;
	}
      if(!(((their_user_rules & test_bit) == 0) ||
	   ((their_user_rules & test_bit) ==
	    (my_team_rules & test_bit))))
	{
	  fprintf(stderr, "opponent rule (%i), challenger's team (%i) mismatch"
		  ": bit %i\n", their_user_rules,
		  my_team_rules, i);
	  return -1;
	}
    }
  fprintf(stderr, "terminating compatibility check\n");
  return 1;
}

int end_user_battle(char * user_name)
{
  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
    {
      fprintf(stderr, "coud not set user status\n");
      return -1;
    }
  broadcast_user_change(user_name, 0);
  return 1;
}

int begin_battle(int user_sock, int user_id, char * user_name,
		 int opponent_sock, int opponent_id, char * opponent_name,
		 int battle_id)
{

  int public_engine_port;
   
  int child_pid;
  int child_ret;
  pid_t wait_ret;
  
  char port_str[64];
  char battle_id_str[64];
  
  public_engine_port = genrand(16384, 49152);
  sprintf(port_str, "%i", public_engine_port);
  sprintf(battle_id_str, "%i", battle_id);
  
  fprintf(stderr, "begin battle function\n");

  child_pid = fork();
  
  if(child_pid == -1)
    {
      perror("fork");
      return -1;
    }
  else if(child_pid != 0) /* the parent process */
    {
      fprintf(stderr, "about to send engine params\n");

      send_engine_params(user_sock, opponent_name, public_engine_port,
			 user_name);
      send_engine_params(opponent_sock, user_name, public_engine_port,
			 opponent_name);
      fprintf(stderr, "battle should be happening\n");
      wait_ret = wait(&child_ret);
      if(wait_ret == -1)
	{
	  perror("wait");
	  return -1;
	}
      child_ret = WEXITSTATUS(child_ret);
      fprintf(stderr,"battle process ended, returns %i\n", child_ret);
      if(child_ret == 0)
	{
	  if(end_user_battle(user_name) < 0 ||
	     end_user_battle(opponent_name) < 0)
	    {
	      fprintf(stderr, "could not end users' battle\n");
	      return -1;
	    }
	  else
	    {
	      fprintf(stderr, "parent thread exitting\n");
	    }
	}
      else if(child_ret == 1)
	{
	  if(end_user_battle(opponent_name) < 0 ||
	     terminate_session(user_name) < 0)
	    {
	      fprintf(stderr, "could not end users' battle\n");
	      return -1;
	    }
	  else
	    {
	      fprintf(stderr, "parent thread exitting\n");
	    }
	}
      else if(child_ret == 2)
	{
	  if(end_user_battle(user_name) < 0 ||
	     terminate_session(opponent_name) < 0)
	    {
	      fprintf(stderr, "could not end users' battle\n");
	      return -1;
	    }
	  else
	    {
	      fprintf(stderr, "parent thread exitting\n");
	    }
	}
      else if(child_ret == 3)
	{
	  if(terminate_session(user_name) < 0 ||
	     terminate_session(opponent_name) < 0)
	    {
	      fprintf(stderr, "could not end users' battle\n");
	      return -1;
	    }
	  else
	    {
	      fprintf(stderr, "parent thread exitting\n");
	    }
	}
    }
  else /* child process */
    {
      /* kick off a battle engine */
      if(execlp("openpkmn_engine", "openpkmn_engine", port_str,
		battle_id_str, (char *)NULL) == -1)
	{
	  perror("execlp");
	}
    }
  return 1;
}

int handle_battle_error(int user_sock, char * user_name,
			int opponent_sock, char * opponent_name)
{
  terminate_session(user_name);
  terminate_session(opponent_name);
  return 1;
}

int initialize_battle(int user_id, char * user_name, int user_sock,
		      int opponent_id, char * opponent_name,
		      int opponent_sock, int * battle_id)
{
  int user_team;
  int opponent_team;
  int user_starter;
  int opponent_starter;
  
  fprintf(stderr, "about to set opponent status...");
  if(set_user_status(opponent_name, USER_STATUS_BATTLING) < 0)
    {
      fprintf(stderr, "could not set opponent's status\n");
    }
  else /* could set opponent status */
    {
      fprintf(stderr, "success\n");
      fprintf(stderr, "about to set user status...");
      if(set_user_status(user_name, USER_STATUS_BATTLING) < 0)
	{
	  fprintf(stderr, "could not set user's status\n");
	}
      else /* could set user status */
	{
	  fprintf(stderr, "success\n");
	  fprintf(stderr, "about to load teams...");
	  if(get_user_current_team(user_name, &user_team) < 0 ||
	     get_user_current_team(opponent_name, &opponent_team) < 0)
	    {
	      fprintf(stderr, "could not get users' teams\n");
	    }
	  else
	    {
	      fprintf(stderr, "success\n");
	      fprintf(stderr, "about to get starters...");
	      if(get_user_selected_starter(user_name, &user_starter) < 0 ||
		 get_user_selected_starter(opponent_name,
					   &opponent_starter) < 0)
		{
		  fprintf(stderr, "could not get users' starters\n");
		}
	      else /* could get starter */
		{
		  fprintf(stderr, "success\n");
		  fprintf(stderr, "about to insert battle...");
		  if(insert_battle(user_id, opponent_id, user_team,
				   user_starter, opponent_team,
				   opponent_starter, battle_id) < 0)
		    {
		      fprintf(stderr, "could not insert battle "
			      "into database\n");
		    }
		  else
		    {
		      return 1;
		    }
		}
	    }
	}
    }
  return -1;
}

int prepare_battle(int user_id, char * user_name, int user_sock,
		   int opponent_id, char * opponent_name,
		   int opponent_sock, int * battle_id)
{
  if(initialize_battle(user_id, user_name, user_sock, opponent_id,
		       opponent_name, opponent_sock, battle_id) < 0)
    {
      fprintf(stderr, "could not initialize battle\n");
    }
  else /* could insert battle */
    {
      fprintf(stderr, "sucess\n");
      fprintf(stderr, "about to send response notification...");
      /* tell challenger battle has been accepted */
      if(send_response_notification(user_sock, user_name, 1) < 0)
	{
	  fprintf(stderr, "could not send notification\n");
	}
      else
	{
	  /* send ack to tell defender challenge response
	     has been received */
	  fprintf(stderr, "notification sent\n");
	  send_ack(opponent_sock, opponent_name);
	  broadcast_user_change(opponent_name, 0);
	  broadcast_user_change(user_name, 0);
	  return 1;
	}
    }
  return -1;
}



int search_users(int user_id, int desired_status)
{
  int i;
  char user_name[32];
  char opponent_name[32];
  int status;
  if(get_user_name(user_id, user_name) < 0)
    {
      return -1;
    }
  for(i = 0; i < NUM_USERS; i++)
    {
      if(i != user_id && users[i] != NULL)
	{
	  if(get_user_name(i, opponent_name) < 0)
	    {
	      return -1;
	    }
	  if(get_user_status(opponent_name, &status) < 0)
	    {
	      return -1;
	    }
	  if(status != desired_status)
	    {
	      continue;
	    }
	  if(opponent_is_compatible(user_name, opponent_name) == 1)
	    {
	      return i;
	    }
	}
    }
  return -1;
}

int generate_team(int user_id, char * user_name, int * team_id, int * starter)
{
  int rules;
  if(get_user_rules(user_name, &rules) < 0)
    {
      fprintf(stderr, "could not get rules\n");
      return -1;
    }
  else
    {
      if(create_random_team(user_id, rules, team_id) < 0)
	{
	  fprintf(stderr, "could not get rules\n");
	  return -1;
	}
      else
	{
	  *starter = 0;
	}
    }
  return 1;
}

int handle_battle_now_packet(int sock, void * decrypted_payload, int user_id,
			     char * user_name)
{
  int team_id;
  int starter;
  int battle_id;
  User * opponent;
  int opponent_id;
  int opponent_sock;
  char opponent_name[32];
  int new_status;
  if(verify_user_status(user_name, USER_STATUS_INITIAL) < 0)
    {
      fprintf(stderr, "could not verify user status\n");
    }
  else
    {
      team_id = ntohl(((Battle_now_pkt *)decrypted_payload)->with_team);
      starter = ntohl(((Battle_now_pkt *)decrypted_payload)->starter);
      if(team_id == -1)
	{
	  new_status = USER_STATUS_FINDING_RANDOM;
	  if(generate_team(user_id, user_name, &team_id, &starter) < 0)
	    {
	      send_error(sock, user_name);
	      return -1;
	    }
	}
      else
	{
	  new_status = USER_STATUS_FINDING_BATTLE;
	}
      if(set_user_status(user_name, new_status) < 0)
	{
	  fprintf(stderr, "could not update user status\n");
	  terminate_session(user_name);
	}
      else
	{
	  if(bind_user_to_team(sock, user_name, user_id, team_id, starter) < 0)
	    {
	      fprintf(stderr, "could not bind user to team\n");
	      send_error(sock, user_name);
	    }
	  else
	    {
	      /* check if there is anyone looking for a battle that we have
		  compatible rules with */
	      /* TODO there is probably a high likelyhood of ugly race
		 conditions */
	      opponent_id = search_users(user_id, new_status);
	      /* we found an opponent, we can challenge them now */
	      if(opponent_id != -1)
		{
		  opponent = users[opponent_id];
		  opponent_sock = opponent->wait_sock;
		  if(get_user_name(opponent_id, opponent_name) < 0)
		    {
		    }
		  else
		    {
		      if(initialize_battle(user_id, user_name, sock,
					   opponent_id, opponent_name,
					   opponent_sock, &battle_id) < 0)
			{
			  fprintf(stderr, "could not init battle\n");
			  handle_battle_error(sock, user_name, opponent_sock,
					      opponent_name);
			}
		      else
			{
			  send_ack(sock, user_name);
			  broadcast_user_change(opponent_name, 0);
			  broadcast_user_change(user_name, 0);
			  if(begin_battle(sock, user_id, user_name,
					  opponent_sock, opponent_id,
					  opponent_name, battle_id) < 0)
			    {
			      fprintf(stderr, "could not begin battle\n");
			      handle_battle_error(sock, user_name,
						  opponent_sock, opponent_name);
			    }
			  else
			    {
			      close(opponent_sock);
			      return 1;
			    }
			}
		    }
		}
	      
	      /* no one was available, so we have to wait */
	      else
		{
		  /* mark this socket as our one to keep alive */
		  users[user_id]->wait_sock = sock;
		  send_ack(sock, user_name);
		  broadcast_user_change(user_name, 0);
		}
	    }
	}
    }
  return -1;
}

void handle_login_packet(int sock, char * user_name, int user_id)
{
  fprintf(stderr, "received a login packet\n");
  /* check that the user is not already online */
  if(verify_user_status(user_name, USER_STATUS_OFFLINE) > 0)
    {
      
      /* make the local version of the user */
      users[user_id] = initialize_user(sock);
      fprintf(stderr, "User set up\n");
      /* set the user's status */
      if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
	{
	  fprintf(stderr, "Could not set user status\n");
	  terminate_session(user_name);
	}
      else
	{
	  send_ack(sock, user_name);
	  broadcast_user_change(user_name, 1);
	}
    }
  else
    {
      fprintf(stderr, "Duplicate sign-on detected\n");
      send_error(sock, user_name);
    }
}

void handle_logout_packet(int sock, Pkmn_setup_pkt * decrypted_payload,
			  int user_id, char * user_name)
{
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) < 0)
    {
      fprintf(stderr, "user is already offline\n");
    }
  else
    {
      if(set_user_status(user_name, USER_STATUS_OFFLINE) < 0)
	{
	  fprintf(stderr, "could not bring user offline\n");
	}
      else
	{
	  free(users[user_id]);
	  users[user_id] = NULL;
	  broadcast_user_change(user_name, 0);
	}
    }
} 

void handle_chat_message_packet(int sock, Chat_message_pkt * decrypted_payload,
				int user_id, char * user_name)
{
  char recipients[NUM_USERS/8];
  char recipient_name[32];
  char message[4096];
  int i, j;
  int id;
  User * recipient;

  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) > 0)
    {
      if(process_chat_message_packet(decrypted_payload, recipients,
				     message) != -1)
	{
	  for(i = 0; i < NUM_USERS/8; i++)
	    {
	      for(j = 0; j < 8; j++)
		{
		  id = j + (8 * i);
		  recipient = users[id];
		  if(recipient != NULL && recipients[i] & (0x1 << j))
		    {
		      if(get_user_name(id, recipient_name) < 0)
			{
			}
		      else
			{
			  send_chat_text_packet(recipient->chat_sock,
						recipient_name, user_name,
						message);
			}
		    }
		}
	    }
	}
    }
} 

void handle_pkmn_setup_packet(int sock, Pkmn_setup_pkt * decrypted_payload,
			      int user_id, char * user_name)
{
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) > 0)
    {
      if(process_pkmn_setup_packet(decrypted_payload, user_id) != -1)
	{
	  fprintf(stderr, "received pkmn packet\n");
	  send_ack(sock, user_name);
	}
      else
	{
	  fprintf(stderr, "error processing pkmn packet\n");
	  terminate_session(user_name);
	}
    }
  else
    {
      send_error(sock, user_name);
    }
}

void handle_preference_packet(int sock, void * decrypted_payload,
			      int user_id, char * user_name)
{
  if(verify_user_status(user_name, USER_STATUS_INITIAL) > 0)
    {
      set_user_ruleset(user_name,
		       ntohl(((Preference_pkt *)
			      decrypted_payload)->rules_value));
      send_ack(sock, user_name);
      broadcast_user_change(user_name, 0);
    }
  else
    {
      send_error(sock, user_name);
    }
}

void handle_form_team_packet(int sock, void * decrypted_payload,
			     int user_id, char * user_name)
{
  int team_id;
  int team_size;
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE))
    {
      team_size =  ntohl(((Form_team_pkt *)decrypted_payload)->num_members);
      /* extract the team from beyond the end of the packet */
      if(team_size <= 0 || team_size > 6)
	{
	  send_error(sock, user_name);
	}
      else if(extract_team((int *)(((char *)decrypted_payload) +
				   sizeof(Form_team_pkt)),
			   team_size, user_id, &team_id) < 0)
	{
	  fprintf(stderr, "error extracting with team\n");
	  terminate_session(user_name);
	}
      else /* could extract team */
	{
	  send_ack(sock, user_name);
	}
    }
  else
    {
      send_error(sock, user_name);
    }
}

void handle_request_available_pkmn_packet(int sock, int user_id,
					  char * user_name)
{
  Pkmn_ll * pkmn;
  int num_pkmn;
  int num_moves;
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) > 0)
    {
      send_ack(sock, user_name);
      get_users_pkmn(user_id, &pkmn, &num_pkmn, &num_moves);
      fprintf(stderr, "got %i pkmn and %i moves\n",
	      num_pkmn, num_moves);
      send_available_pkmn_list_packet(sock, user_name,
				      pkmn, num_pkmn, num_moves);
    }
  else
    {
      send_error(sock, user_name);
    }
}

void handle_request_available_teams_packet(int sock, int user_id,
					   char * user_name)
{
  Team_ll * teams;
  int num_teams;
  int num_members;
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) > 0)
    {
      send_ack(sock, user_name);
      get_users_teams(user_id, &teams, &num_teams, &num_members);
      fprintf(stderr, "got %i teams and %i members\n",
	      num_teams, num_members);
      send_available_team_list_packet(sock, user_name,
				      teams, num_teams, num_members);
    }
  else
    {
      send_error(sock, user_name);
    }
  
}


void handle_request_available_users_packet(int sock, int user_id,
					   char * user_name)
{
  User_ll * users;
  int num_users;
  if(verify_user_status_not(user_name, USER_STATUS_OFFLINE) > 0)
    {
      send_ack(sock, user_name);
      get_active_users(user_id, &users, &num_users);
      fprintf(stderr, "got %i users\n", num_users);
      send_available_users_list_packet(sock, user_name, users, num_users);
    }
  else
    {
      send_error(sock, user_name);
    }
  
}

void handle_cancel_packet(int sock, void * decrypted_payload,
			  int user_id, char * user_name)
{
  if(verify_user_status(user_name, USER_STATUS_WAITING) < 0)
    {
      if(verify_user_status(user_name, USER_STATUS_AWAITING_RESPONSE) < 0)
	{
	  if(verify_user_status(user_name, USER_STATUS_FINDING_BATTLE) < 0)
	    {
	      fprintf(stderr, "user status does not support cancelation\n");
	      send_error(sock, user_name);
	    }
	  else
	    {
	      /* Send an invalid engine_params to indicate it canceled. */
	      if(send_engine_params(users[user_id]->wait_sock, user_name, 0,
				    user_name) < 0)
		{
		  perror("close");
		  send_error(sock, user_name);
		}
	      else
		{
		  /* set logged in status */
		  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
		    {
		      send_error(sock, user_name);
		    }
		  else
		    {
		      send_ack(sock, user_name);
		      broadcast_user_change(user_name, 0);
		    }
		}
	    }
	}
      else /* is awaiting response */
	{
	  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
	    {
	      fprintf(stderr, "could not reset user status\n");
	      send_error(sock, user_name);
	    }
	  else
	    {
	      send_ack(sock, user_name);
	      broadcast_user_change(user_name, 0);
	    }
	}
    }
  else /* is waiting */
    {
      /* Send own id to indicate challenge was canceled. */
      if(send_challenge_notification_packet(users[user_id]->wait_sock,
					    user_name, user_id) < 0)
	{
	  perror("close");
	  send_error(sock, user_name);
	}
      else
	{
	  /* set logged in status */
	  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
	    {
	      send_error(sock, user_name);
	    }
	  else
	    {
	      send_ack(sock, user_name);
	      broadcast_user_change(user_name, 0);
	    }
	}
    }
}

void handle_hide_team_packet(int sock, Hide_team_pkt * decrypted_payload,
			     int user_id, char * user_name)
{
  set_team_invisible(ntohl(decrypted_payload->team_id));
  send_ack(sock, user_name);
}

void handle_hide_pkmn_packet(int sock, Hide_pkmn_pkt * decrypted_payload,
			     int user_id, char * user_name)
{
  set_pkmn_invisible(ntohl(decrypted_payload->pkmn_id));
  send_ack(sock, user_name);
}


void handle_wait_packet(int sock, void * decrypted_payload,
			int user_id, char * user_name)
{

  int team_id;
  int starter;
  int new_status;
  if(verify_user_status(user_name, USER_STATUS_INITIAL) < 0)
    {
      fprintf(stderr, "could not verify user status\n");
    }
  else
    {
      team_id = ntohl(((Wait_pkt *)decrypted_payload)->with_team);
      starter = ntohl(((Wait_pkt *)decrypted_payload)->starter);
      if(team_id == -1)
	{
	  new_status = USER_STATUS_WAITING_RANDOM;
	  if(generate_team(user_id, user_name, &team_id, &starter) < 0)
	    {
	      send_error(sock, user_name);
	      return;
	    }
	}
      else
	{
	  new_status = USER_STATUS_WAITING;
	}
      if(set_user_status(user_name, new_status) < 0)
	{
	  fprintf(stderr, "could not update user status\n");
	  terminate_session(user_name);
	}
      else /* could set user status */
	{

	  if(bind_user_to_team(sock, user_name, user_id, team_id, starter) < 0)
	    {
	      fprintf(stderr, "could not bind user to team\n");
	      send_error(sock, user_name);
	    }
	  else
	    {
	      /* mark this socket as our one to keep alive */
	      users[user_id]->wait_sock = sock;
	      send_ack(sock, user_name);
	      broadcast_user_change(user_name, 0);
	    }
	}
    }

}
int handle_bad_challenge_response(int user_sock, char * user_name) {
  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0)
    {
      return -1;
    }
  else
    {
      send_response_notification(user_sock, user_name, 0);
      broadcast_user_change(user_name, 0);
      return 1;
    }
}

int handle_rejection(int user_sock, char * user_name,
		     int opponent_sock, char * opponent_name)
{
  if(set_user_status(user_name, USER_STATUS_INITIAL) < 0 ||
     set_user_status(opponent_name, USER_STATUS_INITIAL) < 0)
    {
      return -1;
    }
  else
    {
      send_response_notification(user_sock, user_name, 0);
      broadcast_user_change(user_name, 0);
      broadcast_user_change(opponent_name, 0);
      return 1;
    }
}

int process_challenge(int sock, void * decrypted_payload,
		      int user_id, char * user_name, int * team_id,
		      int * starter, int * opponent_id,
		      char * opponent_name, int * opponent_sock)
{
  int desired_status;
  *opponent_id =  htonl(((Challenge_pkt *)decrypted_payload)->opponent_id); 
  if(get_user_name(*opponent_id, opponent_name) < 0)
    {
      fprintf(stderr, "opponent is not in system\n");
    }
  else
    {
      *team_id =
	ntohl(((Challenge_pkt *)decrypted_payload)->with_team);
      *starter =
	ntohl(((Challenge_pkt *)decrypted_payload)->starter);
      if(*team_id == -1)
	{
	  desired_status = USER_STATUS_WAITING_RANDOM;
	  if(generate_team(user_id, user_name, team_id, starter) < 0)
	    {
	      return -1;
	    }
	}
      else
	{
	  desired_status = USER_STATUS_WAITING;
	}
      if(verify_user_status(opponent_name, desired_status) < 0)
	{
	  fprintf(stderr, "opponent is not waiting\n");
	}
      else
	{
	  fprintf(stderr, "about to bind user to team...");
	  if(bind_user_to_team(sock, user_name, user_id, *team_id,
			       *starter) < 0)
	    {
	      fprintf(stderr, "could not bind user to team\n");
	    }
	  else
	    {
	      fprintf(stderr, "success\n");
	      /* check if we are rule-compatible */
	      if(opponent_is_compatible(user_name, opponent_name) < 0)
		{
		  fprintf(stderr, "battlers are not compatible\n");
		}
	      else
		{
		  *opponent_sock = users[*opponent_id]->wait_sock;
		  if(send_challenge_notification_packet(*opponent_sock,
							opponent_name,
							user_id) < 0)
		    {
		      fprintf(stderr, "could not send challenge\n");
		    }
		  else
		    {
		      if(set_current_opponent_id(user_id, *opponent_id) < 0)
			{
			  fprintf(stderr, "could not set current opponent\n");

			}
		      else
			{
			  if(set_user_status(user_name,
					     USER_STATUS_AWAITING_RESPONSE) < 0
			     || set_user_status(opponent_name,
						USER_STATUS_CHALLENGED) < 0)
			    {
			      fprintf(stderr, "could not set user statuses\n");
			    }
			  else
			    {
			      broadcast_user_change(user_name, 0);
			      broadcast_user_change(opponent_name, 0);
			      return 1;
			    }
			}
		    }
		}
	    }
	}
    }
  return -1;
}

int handle_stale_response(int opponent_sock, char * opponent_name)
{
  fprintf(stderr, "received response when not expecting\n");
  /* our challenger should just ignore the challenge,
     they really don't care anymore
     
     our challenged user should be notified with an error
     and should have their challenged status unset */
  if(set_user_status(opponent_name, USER_STATUS_INITIAL) < 0)
    {
      /* if status can't be set things are so bad we should
	 just abourt the challenged user */
      terminate_session(opponent_name);
    } 
  else
    {
      /* otherwise just an error */
      send_error(opponent_sock, opponent_name);
      broadcast_user_change(opponent_name, 0);
    }
  return 1;
}
			
int handle_challenge_packet(int sock, void * decrypted_payload,
			    int user_id, char * user_name)
{
  int battle_id;
  int opponent_id;
  int current_opponent;
  char opponent_name[32];
  int opponent_sock;
  int team_id;
  int starter;
  int response;
  
  if(verify_user_status(user_name, USER_STATUS_INITIAL) < 0)
    {
      send_error(sock, user_name);
    }
  else
    {
      if(process_challenge(sock, decrypted_payload, user_id, user_name,
			   &team_id, &starter, &opponent_id, opponent_name,
			   &opponent_sock) < 0)
	{
	  fprintf(stderr,"could not process challenge\n");
	  send_error(sock, user_name);
	}
      else
	{
	  /* Our challenge has been successfully processed, we can ACK the
	     challenger */
	  send_ack(sock, user_name);
	  /* the challeneged user has gotten back to us */
	  if(receive_challenge_response(opponent_sock, opponent_name,
					&response) < 0)
	    {
	      fprintf(stderr, "could not parse challenge response\n");
	      send_error(opponent_sock, opponent_name);	      
	      /* we can't send an error back to the challenger.  aside from the
		 fact that they have already been ACKed for the challenge
		 packet, ideologically it's not really their error
		 
		 instead we should make set the challenger to be logged in and
		 tell them their challenge has been rejected */
	      if(handle_bad_challenge_response(sock, user_name) < 0)
		{
		  /* there was a database or notification problem
		     either of these are sign that the challenger's session
		     if not the whole server is messed up, so terminate
		     the challenger */
		  fprintf(stderr, "could not handle bad challenge response\n");
		  terminate_session(user_name);
		}
	    }
	  else
	    {
	      /* in another thread, we may have received a cancelation
		 of the challenge, so make sure our user is still fit
		 to be challenged
		 
		 furthermore, make sure our user is not awaiting response
		 for a different opponent.*/
	      if(verify_user_status(user_name,
				    USER_STATUS_AWAITING_RESPONSE) < 0)
		{
		  handle_stale_response(opponent_sock, opponent_name);
		}  
	      else
		{
		  /* our user can receive challenge responses, but this
		     challenge may have since been canceled and they're waiting
		     on an entirely different one */
		  if(get_current_opponent_id(user_id, &current_opponent) < 0)
		    {
		      /* if we can't get our current opponent, there's
			 a major data consitency problem, bail */
		      terminate_session(user_name);
		    }
		  else
		    {
		      if(current_opponent != opponent_id)
			{
			  handle_stale_response(opponent_sock, opponent_name);
			}
		      /* at this point we know our challenger still wants this
			 challenge and our defender has sent a valid response */
		      else
			{
			  if(response == CHALLENGE_ACCEPTED)
			    {
			      if(prepare_battle(user_id, user_name, sock,
						opponent_id, opponent_name,
						opponent_sock, &battle_id) < 0)
				{
				  fprintf(stderr, "could not init battle\n");
				  handle_battle_error(sock, user_name,
						      opponent_sock,
						      opponent_name);
				}
			      else
				{
				  if(begin_battle(sock, user_id, user_name,
						  opponent_sock, opponent_id,
						  opponent_name, battle_id) < 0)
				    {
				      fprintf(stderr,
					      "could not begin battle\n");
				      handle_battle_error(sock, user_name,
							  opponent_sock,
							  opponent_name);
				    }
				  else
				    {
				      close(opponent_sock);
				      return 1;
				    }
				}
			    }
			  else /* challenge was rejected */
			    {
			      if(handle_rejection(sock, user_name,
						  opponent_sock,
						  opponent_name) < 0)
				{
				  fprintf(stderr,
					  "could not handle rejection\n");
				  terminate_session(user_name);
				  terminate_session(opponent_name);
			  
				}
			      else
				{
				  close(opponent_sock);
				  return 1;
				}
			    }
			}
		    }	 
		}
	    }
	}
    }
  return -1;
}

void * client_session(void * args)
{
  int user_id = -1;
  int ret_val;
  int decrypted_len;
  char user_name[32];
  int packet_type;
  void * decrypted_payload;

  Thread_args * targs = args;
  
  int sock = targs->sock;
  free(targs);
  /* if no error receiving packet */
  if((ret_val = receive_package(sock, user_name, &decrypted_payload,
				&decrypted_len)) < 0)
    {
      fprintf(stderr, "Bad packet receive\n");
      terminate_session(user_name);
      close(sock);
    }
  else if(ret_val == RECOVERABLE_ERROR)
    {
      fprintf(stderr, "Recoverable error: sending error\n");
      send_error(sock, user_name);
    }  
  else
    {
      if(get_user_id(user_name, &user_id) < 0)
	{
	  fprintf(stderr, "could not get user id for %s\n", user_name);
	}
      else
	{
	  packet_type = ntohl(((Packet *)decrypted_payload)->packet_type);
	  fprintf(stderr, "packet type is %i\n", packet_type);
	  switch(packet_type)
	    {
	    case LOGIN_PACKET:
	      handle_login_packet(sock, user_name, user_id);
	      /* leave the socket open for status updates */
	      break;
	    case PKMN_SETUP_PACKET:
	      handle_pkmn_setup_packet(sock, decrypted_payload,
				       user_id, user_name);
	      close(sock);
	      break;
	    case PREFERENCE_PACKET:
	      handle_preference_packet(sock, decrypted_payload,
				       user_id, user_name);
	      close(sock);
	      break;
	    case WAIT_PACKET:
	      handle_wait_packet(sock, decrypted_payload, user_id,
				 user_name);
	      break;
	    case BATTLE_NOW_PACKET:
	      handle_battle_now_packet(sock, decrypted_payload, user_id,
				       user_name);
	      break;
	    case REQUEST_AVAILABLE_PKMN_PACKET:
	      fprintf(stderr, "pass to request_available_pkmn handler\n");
	      handle_request_available_pkmn_packet(sock, user_id,
						   user_name);
	      close(sock);
	      break;
	    case FORM_TEAM_PACKET:
	      handle_form_team_packet(sock, decrypted_payload,
				      user_id, user_name);
	      close(sock);
	      break;
	    case REQUEST_AVAILABLE_TEAMS_PACKET:
	      handle_request_available_teams_packet(sock, user_id,
						    user_name);
	      close(sock);
	      break;
	    case REQUEST_AVAILABLE_USERS_PACKET:
	      fprintf(stderr, "pass to request_available_users handler\n");
	      handle_request_available_users_packet(sock, user_id, user_name);
	      close(sock);
	      break;
	    case CHALLENGE_PACKET:
	      handle_challenge_packet(sock, decrypted_payload,
				      user_id, user_name);
	      close(sock);
	      break;
	    case CANCEL_PACKET:
	      handle_cancel_packet(sock, decrypted_payload, user_id,
				   user_name);
	      close(sock);
	      break;
	    case CHAT_MESSAGE_PACKET:
	      handle_chat_message_packet(sock, decrypted_payload, user_id,
					 user_name);
	      close(sock);
	      break;
	    case HIDE_PKMN_PACKET:
	      handle_hide_pkmn_packet(sock, decrypted_payload, user_id,
				      user_name);
	      close(sock);
	      break;
	    case HIDE_TEAM_PACKET:
	      handle_hide_team_packet(sock, decrypted_payload, user_id,
				      user_name);
	      close(sock);
	      break;
	    case LOGOUT_PACKET:
	      handle_logout_packet(sock, decrypted_payload,
				   user_id, user_name);
	      close(sock);
	      break;
	    default:
	      fprintf(stderr, "Unrecognized packet type: %i\n", packet_type);
	      break;
	    }
	}
      free(decrypted_payload);
    }

  fprintf(stderr, "Thread is exitting\n");
  pthread_exit(NULL);
}


int main(int argc, char * argv[])
{
  Thread_args * thread_args;
  pthread_attr_t attr;
  int port;
  int sock;
  int client_sock;
  int i;
  struct sockaddr_in their_addr;
  socklen_t sin_size =  sizeof(struct sockaddr_in);
  
  signal(SIGPIPE, SIG_IGN);

  if(argc < 2)
    {
      port = WAITINGROOM_PORT;
    }
  else
    {
      port = strtol(argv[1], NULL, 10);
    }
      
  sock = init_socket(port);
  if(sock == -1)
    {
      return -1;
    }
  i = 0;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  while(1)
    {
      fprintf(stderr, "waiting for connection\n");
      if ((client_sock = accept(sock, (struct sockaddr *)&their_addr,
				&sin_size)) == -1)
	{
	  perror("accept");
	}

      fprintf(stderr, "establishing connection %i, socket %i\n",
	      i, client_sock);

      /* we have received a connection, create a new session
	 to service it */
      if((thread_args = malloc(sizeof(Thread_args))) != NULL)
	{
	  thread_args->sock = client_sock;
	  if(pthread_create(&thread_args->thread_ptr, &attr,
			    client_session, (void *)thread_args) != 0)
	    {
	      perror("pthread_create");
	      break;
	    }
	}
      else
	{
	  fprintf(stderr, "could not create thread args\n");
	  break;
	}
      i++;
    }
  pthread_attr_destroy(&attr);
  return 0;
}
