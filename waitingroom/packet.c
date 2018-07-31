#ifndef OPENPKMN_WAITINGROOM_PACKET__
#define OPENPKMN_WAITINGROOM_PACKET__ 1

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "db_iface.h"
#include "stats.h"

#include "../share/packet.h"
#include "../share/encryption.h"
#include "../share/move_defs.h"
#include "../share/preferences.h"


int process_chat_message_packet(Chat_message_pkt * decrypted_payload,
				char * recipients, char * message)
{
  memcpy(recipients, decrypted_payload->recipients,
	 NUM_USERS/8);
  strncpy(message, decrypted_payload->message, 4096);
  return 1;
}

/* TODO: remove magic numbers once role of environment.h file is sorted out */
int process_pkmn_setup_packet(void * decrypted_payload, int user_id) {
  int num_moves;
  int i;
  int pkmn_id;
  int pkmn_num;
  int level;
  int hp, attack, defense, speed, special;
  int move_numbers[4];
  int move_pps[4];
  int max_pp;
  void * decrypted_move_payload;
  int found = 0;
  int adherent_rules = (HOLDS_TO_ILLEGAL_MOVES_DISALLOWED |
			HOLDS_TO_TM_MOVES_DISALLOWED |
			HOLDS_TO_GS_MOVES_DISALLOWED |
			HOLDS_TO_OHKO_MOVES_DISALLOWED |
			HOLDS_TO_EVADE_MOVES_DISALLOWED |
			HOLDS_TO_MEWTWO_DISALLOWED |
			HOLDS_TO_MEW_DISALLOWED |
			HOLDS_TO_ILLEGAL_STATS_DISALLOWED |
			HOLDS_TO_ILLEGAL_LEVELS_DISALLOWED);
  
  fprintf(stderr, "starting with rules %i\n", adherent_rules);
  
  num_moves = ntohl(((Pkmn_setup_pkt *)decrypted_payload)->num_moves);
  
  pkmn_num = ((Pkmn_setup_pkt *)decrypted_payload)->number;
  level = ((Pkmn_setup_pkt *)decrypted_payload)->level;
  hp = ntohs(((Pkmn_setup_pkt *)decrypted_payload)->max_hp);
  attack = ntohs(((Pkmn_setup_pkt *)decrypted_payload)->attack);
  defense = ntohs(((Pkmn_setup_pkt *)decrypted_payload)->defense);
  speed = ntohs(((Pkmn_setup_pkt *)decrypted_payload)->speed);
  special = ntohs(((Pkmn_setup_pkt *)decrypted_payload)->special);

  if(pkmn_num < 1 || pkmn_num > 151) {
    return -1;
  }
  
  if(level < 2 || level > 100)
    {
      adherent_rules &= ~(HOLDS_TO_ILLEGAL_LEVELS_DISALLOWED);
    }
  
  if(!hp_stat_valid(hp, pkmn_num, level) ||
     !attack_stat_valid(attack, pkmn_num, level) ||
     !defense_stat_valid(defense, pkmn_num, level) ||
     !speed_stat_valid(speed, pkmn_num, level) ||
     !special_stat_valid(special, pkmn_num, level))
    {
      adherent_rules &= ~(HOLDS_TO_ILLEGAL_STATS_DISALLOWED);
    }
  if(pkmn_num == 151)
    {
      adherent_rules &= ~(HOLDS_TO_MEW_DISALLOWED);
    }
  else if(pkmn_num == 150)
    {
      adherent_rules &= ~(HOLDS_TO_MEWTWO_DISALLOWED);
    }
  
  decrypted_move_payload = (char *)(decrypted_payload) + sizeof(Pkmn_setup_pkt);

  for(i = 0; i < num_moves; i++)
    {
      move_numbers[i] = ntohl(((Move_setup_pkt *)
			       decrypted_move_payload)->number);

      /* if the move is NONE to not evaluate its legality */
      if(move_numbers[i] == NONE)
	{
	  fprintf(stderr, "ignoring NONE move\n");
	  continue;
	}
      
      /* check that the move is within a valid range */
      if(move_numbers[i] < 0 || move_numbers[i] > 165)
	{
	  return -1;
	}

      if(get_move_pp(move_numbers[i], &max_pp) < 0)
	{
	  return -1;
	}
      
      move_pps[i] = ntohl(((Move_setup_pkt *)decrypted_move_payload)->pp);
      if(move_pps[i] > max_pp || move_pps[i] < ((max_pp != 61)?
						((max_pp * 5)/8):40))
	{
	  return -1;
	}
      
      fprintf(stderr, "testing move %i\n", move_numbers[i]);
      
      if(move_learned_by_method(pkmn_num, move_numbers[i], NATURAL, &found) > 0
	 && found)
	{
	  found = 0;
	  fprintf(stderr, "found as natural\n");
	}
      else if(move_learned_by_method(pkmn_num, move_numbers[i], TM_HM,
				     &found) > 0 && found)
	{
	  fprintf(stderr, "found as tm/hm\n");
	  found = 0;
	  adherent_rules &= ~(HOLDS_TO_TM_MOVES_DISALLOWED);
	}
      else if(move_learned_by_method(pkmn_num, move_numbers[i],
				     GS_TRADEBACK, &found) > 0 && found)
	{
	  fprintf(stderr, "found as gs tradeback\n");
	  found = 0;
	  adherent_rules &= ~(HOLDS_TO_GS_MOVES_DISALLOWED);
	}
      else
	{
	  found = 0;
	  adherent_rules &= ~(HOLDS_TO_ILLEGAL_MOVES_DISALLOWED);
	}

      if(move_numbers[i] == DOUBLETEAM || move_numbers[i] == MINIMIZE)
	{
	   adherent_rules &= ~(HOLDS_TO_EVADE_MOVES_DISALLOWED);
	}
      else if(move_numbers[i] == HORNDRILL || move_numbers[i] == GUILLOTINE ||
	      move_numbers[i] == FISSURE)
	{
	   adherent_rules &= ~(HOLDS_TO_OHKO_MOVES_DISALLOWED);
	}
      decrypted_move_payload = ((char *)decrypted_move_payload) +
	sizeof(Move_setup_pkt);
    }
  
  if(insert_pkmn(user_id, pkmn_num,
		 ((Pkmn_setup_pkt *)decrypted_payload)->nickname,
		 ((Pkmn_setup_pkt *)decrypted_payload)->level,
		 ntohs(((Pkmn_setup_pkt *)decrypted_payload)->max_hp),
		 ntohs(((Pkmn_setup_pkt *)decrypted_payload)->attack),
		 ntohs(((Pkmn_setup_pkt *)decrypted_payload)->defense),
		 ntohs(((Pkmn_setup_pkt *)decrypted_payload)->speed),
		 ntohs(((Pkmn_setup_pkt *)decrypted_payload)->special),
		 adherent_rules, &pkmn_id) < 0)
    {
      return -1;
    }
  
  for(i = 0; i < num_moves; i++)
    {
      if(insert_move(user_id, pkmn_id, move_numbers[i], move_pps[i]) < 0)
	{
	  fprintf(stderr, "Database error: cannot insert pkmn\n");
	  return -1;
	}
    }
  return 1;
}

int send_available_pkmn_list_packet(int sock, char * user_name, Pkmn_ll * pkmn,
				    int num_pkmn, int num_moves)
{
  int i;
  int pkmn_moves;
  Available_pkmn_pkt * current_pkmn;
  char * current;
  int payload_len;
  void * payload = malloc(payload_len = (sizeof(Available_pkmn_list_pkt) +
			   num_pkmn * sizeof(Available_pkmn_pkt) +
			   num_moves * sizeof(Available_pkmn_move_pkt)));
  current = payload;
  

  ((Available_pkmn_list_pkt *)current)->packet_type =
    htonl(AVAILABLE_PKMN_LIST_PACKET);
  ((Available_pkmn_list_pkt *)current)->num_pkmn = htonl(num_pkmn);
  current += sizeof(Available_pkmn_list_pkt);
  /* go through each pkmn */
  for(i = 0; i < num_pkmn; i++)
    {
      /* write vitals to packet payload */
      ((Available_pkmn_pkt *)current)->index = htonl(pkmn->index);
      ((Available_pkmn_pkt *)current)->number = htonl(pkmn->species);
      ((Available_pkmn_pkt *)current)->rules = htonl(pkmn->ruleset);
      /* back up the current pkmn for later use*/
      current_pkmn = (Available_pkmn_pkt * )current;
      /* skip to the move portion */
      current +=  sizeof(Available_pkmn_pkt);

      /* now go through each move in the pkmn */
      pkmn_moves = 0;
      while(pkmn->moves != NULL)
	{
	  /* write each move to payload */
	  ((Available_pkmn_move_pkt *)current)->number =
	    htonl(pkmn->moves->move_num);
	  /* get to our next node in the linked list */
	  pkmn->moves = pkmn->moves->next;
	  /* increment move counter */
	  pkmn_moves++;
	  /* go to the next slot in the packet */
	  current +=  sizeof(Available_pkmn_move_pkt);
	}
      
      current_pkmn->num_moves = htonl(pkmn_moves);
      pkmn = pkmn->next;
    }
  return send_package(sock, user_name, payload, payload_len);  
}

int send_chat_text_packet(int sock, char * user_name, char *
			  sender, char * message)
{
  Chat_text_pkt pkt;
  pkt.packet_type = htonl(CHAT_TEXT_PACKET);
  strncpy(pkt.sender_name, sender, 32);
  strncpy(pkt.message, message, 4096);
  return send_package(sock, user_name, &pkt, sizeof(pkt));
}


int send_user_change_packet(int sock, char * user_name,
			    unsigned int updated_id, char * updated_name,
			    unsigned int updated_rules,
			    unsigned int updated_status,
			    unsigned int updated_team_rules, int new)
{
  User_change_pkt pkt;
  pkt.packet_type = htonl(USER_CHANGE_PACKET);
  pkt.user.id = htonl(updated_id);
  strncpy(pkt.user.user_name, updated_name, 32);
  pkt.user.ruleset = htonl(updated_rules);
  pkt.user.status = htonl(updated_status);
  pkt.user.team_rules = htonl(updated_team_rules);
  pkt.new = htonl(new);
  return send_package(sock, user_name, &pkt, sizeof(pkt));
}

int send_available_team_list_packet(int sock, char * user_name, Team_ll * teams,
				    int num_teams, int num_members)
{
  int i;
  int team_members;
  Available_team_pkt * current_team;
  char * current;
  int payload_len;
  void * payload = malloc(payload_len = (sizeof(Available_team_list_pkt) +
			   num_teams * sizeof(Available_team_pkt) +
			   num_members * sizeof(Team_member)));
  current = payload;

  ((Available_team_list_pkt *)current)->packet_type =
    htonl(AVAILABLE_TEAMS_LIST_PACKET);
  ((Available_team_list_pkt *)current)->num_teams =
    htonl(num_teams);
  current += sizeof(Available_team_list_pkt);
  /* go through each pkmn */
  for(i = 0; i < num_teams; i++)
    {
      /* write vitals to packet payload */
      ((Available_team_pkt *)current)->index = htonl(teams->index);
      ((Available_team_pkt *)current)->rules = htonl(teams->ruleset);
      /* back up the current pkmn for later use*/
      current_team = (Available_team_pkt * )current;
      /* skip to the move portion */
      current +=  sizeof(Available_team_pkt);

      /* now go through each move in the pkmn */
      team_members = 0;
      while(teams->members != NULL)
	{
	  /* write each move to payload */
	  ((Available_team_member_pkt *)current)->index =
	    htonl(teams->members->index);
	  /* get to our next node in the linked list */
	  teams->members = teams->members->next;
	  /* increment move counter */
	  team_members++;
	  /* go to the next slot in the packet */
	  current +=  sizeof(Available_team_member_pkt);
	}
      
      current_team->num_members = htonl(team_members);
      teams = teams->next;
    }
  return send_package(sock, user_name, payload, payload_len);
}


int send_available_users_list_packet(int sock, char * user_name,
				     User_ll * users, int num_users)
{
  int i;
  char * current;
  int payload_len;
  void * payload = malloc(payload_len = (sizeof(Available_users_list_pkt) +
			   num_users * sizeof(Available_user_pkt)));
  current = payload;

  ((Available_users_list_pkt *)current)->packet_type =
    htonl(AVAILABLE_USERS_LIST_PACKET);
  ((Available_users_list_pkt *)current)->num_users =
    htonl(num_users);
  current += sizeof(Available_users_list_pkt);
  /* go through each user */
  for(i = 0; i < num_users; i++)
    {
      /* write vitals to packet payload */
      ((Available_user_pkt *)current)->id = htonl(users->id);
      strncpy(((Available_user_pkt *)current)->user_name, users->user_name, 32);
      ((Available_user_pkt *)current)->ruleset = htonl(users->ruleset);
      ((Available_user_pkt *)current)->status = htonl(users->status);
      ((Available_user_pkt *)current)->team_rules = htonl(users->team_rules);
      /* skip to the move portion */
      current +=  sizeof(Available_user_pkt);
      users = users->next;
    }
  return send_package(sock, user_name, payload, payload_len);
}


int send_challenge_notification_packet(int sock, char * user_name,
				       int challenger_id)
{
  Challenge_notification_pkt pkt;
  pkt.packet_type = htonl(CHALLENGE_NOTIFICATION_PACKET);
  pkt.challenger_id = htonl(challenger_id);
  return send_package(sock, user_name, &pkt, sizeof(pkt));
}

int send_response_notification(int sock, char * user_name, int affirmative)
{
  Challenge_response_pkt pkt;
  pkt.packet_type = htonl(CHALLENGE_RESPONSE_PACKET);
  pkt.accepted = htonl(affirmative);
  return send_package(sock, user_name, &pkt, sizeof(pkt));
}

int receive_challenge_response(int sock, char * user_name, int * response)
{
  void * payload;
  int decrypted_len;
  int ret_val;
  if((ret_val = receive_package(sock, user_name, &payload, &decrypted_len)) < 0)
    {
      
    }
  else
    {
      *response = ntohl(((Challenge_response_pkt *)payload)->accepted);
    }
  return ret_val;
}

#endif
