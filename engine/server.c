#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#include "engine.h"
#include "eutil.h"
#include "packet.h"
#include "events.h"
#include "db_iface.h"

#include "../share/netutil.h"
#include "../share/config.h"
#include "../share/packet.h"
#include "../share/error.h"

#define USER_0_FAULT 1
#define USER_1_FAULT 2

int get_remaining_millis(unsigned long max_time, struct timeval * end,
			 struct timeval * start)
{
  return max_time - (((end->tv_sec - start->tv_sec) * 1000) +
		     ((end->tv_usec - start->tv_usec) / 1000));
}

int get_second_response(Client_response * client_resp, int client_sock,
			char * username, struct pollfd * poll_sock, int fault,
			int timeout)
{  
  if(poll(poll_sock, 1, timeout) == -1)
    {
      return fault;
    }
  else
    {
      if(poll_sock->revents & POLLIN)
	{
	  if(receive_client_response(client_resp, client_sock, username) < 0)
	    {
	      fprintf(stderr, "receive error on 1\n");
	      return fault;
	    }
	}
      else /* user 1 is gone */
	{
	  return fault;
	}
    }
  return 0;
}


int get_responses(Client_response * client_resp, int * client_sock,
		  int timeout)
{
  
  struct pollfd poll_sock[2];
  int poll_events;

  struct timeval start;
  struct timeval end;
  
  int fault = 0;
  poll_sock[0].fd = client_sock[0];
  poll_sock[0].events = POLLIN;
  poll_sock[1].fd = client_sock[1];
  poll_sock[1].events = POLLIN;
  
  /* we don't know how far into our timeout we are when we
     are done polling, so keep track */
  gettimeofday(&start, NULL); 
  /* initialize the user sessions */
  if((poll_events = poll(poll_sock, 2, timeout)) == -1)
    {
      perror("poll");
      fault |= USER_0_FAULT | USER_1_FAULT;
    }
  else
    {
      /* receive user 0's events, then wait for 1s */
      if(poll_sock[0].revents & POLLIN)
	{
	  gettimeofday(&end, NULL);
	  if(receive_client_response(&client_resp[0],
				     client_sock[0],
				     session.user[0].name) < 0)
	    {
	      fprintf(stderr, "receive error on 0\n");
	      fault |= USER_0_FAULT;
	    }
	  
	  fault |= get_second_response(&client_resp[1], client_sock[1],
				       session.user[1].name,
				       &poll_sock[1], USER_1_FAULT,
				       get_remaining_millis(timeout, &end,
							    &start));
	}
      /* receive user 1's events, then wait for 0s */
      else if(poll_sock[1].revents & POLLIN)
	{
	  gettimeofday(&end, NULL);
	  if(receive_client_response(&client_resp[1], client_sock[1],
				     session.user[1].name) < 0)
	    {
	      fault |= USER_1_FAULT;
	    }
	  fault |= get_second_response(&client_resp[0], client_sock[0],
				       session.user[0].name, &poll_sock[0],
				       USER_0_FAULT,
				       get_remaining_millis(timeout, &end,
							    &start));
	}
      else /* both users went AWOL */
	{
	  fault |= USER_0_FAULT | USER_1_FAULT;
	}
    }
  return fault;
}

int extract_user_pkmn(User * user, Db_pkmn * pkmn)
{
  int i = 0;
  int j = 0;
  Db_move * move;
  
  memset(user->pkmn, 0, sizeof(user->pkmn));

  /* set so that no pkmn is out initially */
  user->num_active = 1;
  user->total = 0;
  user->fainted = 0;
  
  while(pkmn != NULL && i < 6)
    {
      strncpy(user->pkmn[i].nickname, pkmn->nickname, 32);

      user->pkmn[i].id = pkmn->id;
      
      user->pkmn[i].number = pkmn->species;
      user->pkmn[i].level = pkmn->level;
     
      user->pkmn[i].current_hp = pkmn->max_hp;
      user->pkmn[i].max_hp = pkmn-> max_hp;
      
      user->pkmn[i].unmod_stats.attack = pkmn->attack;
      user->pkmn[i].unmod_stats.defense = pkmn->defense;
      user->pkmn[i].unmod_stats.speed = pkmn->speed;
      user->pkmn[i].unmod_stats.special = pkmn->special;
      
      user->pkmn[i].roster_number = i;
      
      user->total++;
      
      memset(user->pkmn[i].move, 0, sizeof(user->pkmn[i].move));
      j = 0;
     
      move = pkmn->moves;
      
      while(move != NULL && j < 4)
	{
	  user->pkmn[i].move[j].number = move->move_id;
	  user->pkmn[i].move[j].current_pp = move->pp;
	  move = move->next;
	  j++;
	}
      user->pkmn[i].num_moves = j;
      pkmn = pkmn->next;
      i++;
    }
  return 1;
}

int extract_challenger(int battle_id, User * user)
{
  int starter_id;
  Db_pkmn * db_pkmn;
  int i;
  user->fainted = 0;
  user->num_active = 0;
  if(get_battle_challenger_name(battle_id, user->name) < 0)
    {
      fprintf(stderr, "cNAME: could not access database battle_id: %i\n",
	      battle_id);
      return DATABASE_ERROR;
    }
  else
    {
      if(get_user_id(user->name, &(user->id)) < 0)
	{
	  fprintf(stderr, "cuid: not there\n");
	  return DATABASE_ERROR;
	}
      else
	{
	  if(get_battle_challenger_pkmn(battle_id, &db_pkmn) < 0)
	    {
	      fprintf(stderr, "cPKMN: could not access database\n");
	      return DATABASE_ERROR;
	    }
	  else
	    {
	      if(extract_user_pkmn(user, db_pkmn) < 0)
		{
		  fprintf(stderr, "cPKMN: could not extract pkmn\n");
		  return -1;
		}
	      else
		{
		  if(get_battle_challenger_starter(battle_id, &starter_id) < 0)
		    {
		      fprintf(stderr, "cPKMN: could not get starter\n");
		      return DATABASE_ERROR;
		    }
		  else
		    {
		      user->starter = 0;
		      for(i = 0; i < user->total; i++)
			{
			  if(user->pkmn[i].id == starter_id)
			    {
			      user->starter = i;
			      break;
			    }
			}
		    }
		}
	    }
	}
    }
  return 1;
}

int extract_defender(int battle_id, User * user)
{
  int i;
  int starter_id;
  Db_pkmn * db_pkmn;
  
  user->fainted = 0;
  user->num_active = 0;
  if(get_battle_defender_name(battle_id, user->name) < 0)
    {
      fprintf(stderr, "dNAME: could not access database "
	      "battle_id: %i\n", battle_id);
      return DATABASE_ERROR;
    }
  else
    {
      if(get_user_id(user->name, &(user->id)) < 0)
	{
	  fprintf(stderr, "duid: not there\n");
	  return DATABASE_ERROR;
	}
      else
	{
	  if(get_battle_defender_pkmn(battle_id, &db_pkmn) < 0)
	    {
	      fprintf(stderr, "dPKMN: could not access database\n");
	      return DATABASE_ERROR;
	    }
	  else
	    {
	      if(extract_user_pkmn(user, db_pkmn) < 0)
		{
		  fprintf(stderr, "dPKMN: could not extract pkmn\n");
		  return -1;
		}
	      else
		{
		  if(get_battle_defender_starter(battle_id, &starter_id) < 0)
		    {
		      fprintf(stderr, "dPKMN: could not get starter\n");
		      return DATABASE_ERROR;
		    }
		  else
		    {
		      user->starter = 0;
		      for(i = 0; i < user->total; i++)
			{
			  if(user->pkmn[i].id == starter_id)
			    {
			      user->starter = i;
			      break;
			    }
			}
		    }
		}
	    }
	}
    }
  return 1;
}

int process_turns_and_events(Event * events, int num_events, int round_id,
			     Active_data * state0, Active_data * state1,
			     int pkmn_id0, int pkmn_id1)
{
  int i;
  int turn_id = -1;
  int attacker_index;
  int subject_index;
  char lead_turn = 1;
  Active_data * state;
  int pkmn_id;
  int state_id;

  fprintf(stderr, "@@@enterred log processing function\n");
  for(i = 0; i < num_events; i++)
    {
      fprintf(stderr, "@@@found an event %i\n", events[i].message_id);
      /* we have found a turn delimiter, add a turn */
      if(events[i].message_id == BEGIN_TURN)
	{
	  attacker_index = events[i].absolute_actor_id;
	  insert_turn(session.user[attacker_index].id,
		      session.user[!attacker_index].id,
		      session.user[attacker_index].pkmn
		      [session.user[attacker_index].num_active].id,
		      session.user[!attacker_index].pkmn
		      [session.user[!attacker_index].num_active].id,
		      lead_turn, round_id, &turn_id);
	  state = (attacker_index) ? state1 : state0;
	  pkmn_id = (attacker_index) ? pkmn_id1 : pkmn_id0;
	  insert_battle_state(state, turn_id, pkmn_id, &state_id);
	  /* set lead_turn to false so that the second turn
	     is not marked is the lead turn */
	  lead_turn = 0;
	}
      /* otherwise add events to a turn */
      else if(turn_id != -1)
	{
	  subject_index = events[i].absolute_actor_id;
	  insert_event(turn_id, events[i].message_id, events[i].details,
		       session.user[subject_index].id,
		       session.user[subject_index].pkmn
		       [session.user[subject_index].num_active].id,
		       session.user[!subject_index].id,
		       session.user[!subject_index].pkmn
		       [session.user[!subject_index].num_active].id);
	}
      else
	{
	  fprintf(stderr, "@@@event outside of a turn!\n"); 
	}
    }
  return 1;
}

int main(int argc, char * argv[])
{  
  Client_response client_resp[2];
  
  unsigned int port;
  
  int battle_id;
  int round_id;
  
  int sock;
  int initial_sock;
  int client_sock[2];
  int connected[2] = { 0, 0 };

  char test_uname[32];
  void * encrypted_payload;
  int encrypted_len;
  void * decrypted_payload;
  int decrypted_len;
  int packet_type;
  
  struct sockaddr_in their_addr;
  socklen_t sin_size =  sizeof(struct sockaddr_in);
  
  int final_state = NORMAL_ROUND;
  unsigned long turn_timeout = 300000;
  int fault = 0;
  
  /* ignore broken pipe signal, otherwise aborted clients will kill server */
  signal(SIGPIPE, SIG_IGN);

  fprintf(stderr, "engine is alive port: %s, id: %s\n", argv[1], argv[2]);
  
  /* get the port */
  port = strtol(argv[1], NULL, 10);
  battle_id = strtol(argv[2], NULL, 10);
  
  /* with the users read in set up the session */
  session.status = NORMAL_ROUND;
  session.round = 0;

  /* set up the challenger and defender sessions */
  if(extract_challenger(battle_id, &session.user[0]) < 0)
    {
      fprintf(stderr, "failed to extract challenger\n");
      exit(USER_0_FAULT | USER_1_FAULT);
    }
  if(extract_defender(battle_id, &session.user[1]) < 0)
    {
      fprintf(stderr, "failed to extract defender\n");
      exit(USER_0_FAULT | USER_1_FAULT);
    }
  if(get_battle_rules(battle_id, &(session.enforced_clauses)) < 0)
    {
      fprintf(stderr, "failed to extract enforced clauses\n");
      exit(USER_0_FAULT | USER_1_FAULT);
    }
  sock = init_socket(port);
  fprintf(stderr, "listening on port %i\n", port);
  
  /* create 2 sockets for users to initiate connection */
  while(!connected[0] || !connected[1])
    {
      fprintf(stderr, "waiting for client\n");
      if((initial_sock = accept(sock, (struct sockaddr *)
				&their_addr, &sin_size)) != -1)
	{
	  if(receive_package_unverified(initial_sock, test_uname,
				     	&encrypted_payload, &encrypted_len,
					&packet_type) > 0)
	    {	      
	      if(strncmp(test_uname, session.user[0].name, 32) == 0)
		{
		  if(verify_package(test_uname, &decrypted_payload,
				    &decrypted_len, encrypted_payload,
				    encrypted_len, packet_type) > 0)
		    {
		      client_sock[0] = initial_sock;
		      connected[0] = 1;
		    }
		}
	      else if(strncmp(test_uname, session.user[1].name, 32) == 0)
		{
		  if(verify_package(test_uname, &decrypted_payload,
				    &decrypted_len, encrypted_payload,
				    encrypted_len, packet_type) > 0)
		    {
		      client_sock[1] = initial_sock;
		      connected[1] = 1;
		    }
		}				     
	    }
	  else
	    {
	      perror("receive");
	    }
	}
      else
	{
	  perror("accept");
	}
    }
  do_sendout(session.user[0].starter, session.user[1].starter);
  
  while(1)
    {
      /* put the round in the database */
      insert_round(session.round, battle_id, session.status, &round_id);
      
      process_turns_and_events(log_register, log_counter, round_id,
			       &session.user[0].active_data,
			       &session.user[1].active_data,
			       session.user[0].pkmn[session.user[0].num_active].
			       id, session.user[1].pkmn[session.user[1].
							num_active].id);
      
      /* send the clients the server status */
      send_server_status_packet(client_sock[0], session.user[0].name,
				0, &session);
      send_event_logs(log_register, log_counter, client_sock[0], 0,
		      session.user[0].name);
      
      send_server_status_packet(client_sock[1], session.user[1].name,
				1, &session);
      send_event_logs(log_register, log_counter, client_sock[1], 1,
		      session.user[1].name);
      
      /* once we have told the client the final outcome, we can exit */
      if(final_state == TIE || final_state == LOSS_1 || final_state == LOSS_0)
	{
	  insert_battle_score(battle_id, session.user[0].total -
			      session.user[0].fainted, session.user[1].total
			      - session.user[1].fainted);
	  if(final_state == LOSS_1)
	    {
	      insert_battle_winner_loser(battle_id, session.user[0].id,
					 session.user[1].id);
	    }
	  else if(final_state == LOSS_0)
	    {
	      insert_battle_winner_loser(battle_id, session.user[1].id,
					 session.user[0].id);
	    }
	  break;
	}
      else
	{
	  fault = get_responses(client_resp, client_sock, turn_timeout);
	}
      /* process the client's response to the status */
      if(fault == 0)
	{
	  fprintf(stderr, "we are giving the engine the following: %i %i %i "
		  "%i\n", client_resp[0].command, client_resp[0].option,
		  client_resp[1].command, client_resp[1].option);
	  
	  
	  final_state = compute_round(client_resp[0].command,
				      client_resp[0].option,
				      client_resp[1].command,
				      client_resp[1].option);
	}
      else if((fault & USER_0_FAULT) && (fault & USER_1_FAULT))
	{
	  final_state = compute_round(RUNNING, 0, RUNNING, 0);
	}
      else if(fault & USER_0_FAULT)
	{
	  final_state = compute_round(RUNNING, 0, NO_COMMAND, 0);
	}
      else if(fault & USER_1_FAULT)
	{
	  final_state = compute_round(NO_COMMAND, 0, RUNNING, 0);
	}
      else
	{
	  fprintf(stderr, "faulty fault setting\n");
	  exit(USER_0_FAULT | USER_1_FAULT);
	}
      session.round++;
    }
  fprintf(stderr, "server exitted with final status %i\n", final_state);
  fprintf(stderr, "server returned fault of %i\n", fault);
  return fault;
}
