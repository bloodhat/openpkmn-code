#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "packet.h"
#include "datatypes.h"

#include "eutil.h"
#include "environment.h"
#include "events.h"

#include "../share/netutil.h"
#include "../share/packet.h"
#include "../share/error.h"
#include "../share/move_defs.h"

#define DEBUG_PACKETS 1

void pack_pkmn_list_data_pkt(Pkmn * pkmn,
			     Pkmn_list_data_pkt * pkt)
{
  if(pkmn != NULL)
    {
      pkt->number = pkmn->number;
      pkt->level = pkmn->level;
      pkt->status = (pkmn->status > NRM &&
		     pkmn->status <= SLP) ? SLP : pkmn->status;
      
      pkt->current_hp = htons(pkmn->current_hp);
      pkt->max_hp = htons(pkmn->max_hp);
      
      strncpy(pkt->nickname, pkmn->nickname,
	      sizeof(char) * 32);
  }
  else
    {
      pkt->number = 0;
    }
}

void pack_pkmn_list_data_pkt_from_active(Active_data * a_data,
					 Pkmn * pkmn,
					 Pkmn_list_data_pkt * pkt)
{
  pkt->number = a_data->number;
  pkt->level = pkmn->level;
  pkt->status = (pkmn->status > NRM &&
		     pkmn->status <= SLP) ? SLP : pkmn->status;

  pkt->current_hp = htons(pkmn->current_hp);
  pkt->max_hp = htons(pkmn->max_hp);

  strncpy(pkt->nickname, pkmn->nickname,
	 sizeof(char) * 32);
}
			  
int send_server_status_packet(int sock, char * user_name,
			      unsigned int user_id,
			      Session * session)
{
  int i;
  char * current;
  int payload_len;
  int num_moves;
  int num_pkmn;
  Server_status_pkt * pkt;

  void * payload = malloc(payload_len =
			  (sizeof(Server_status_pkt) +
			   (num_pkmn = session->user[user_id].total) *
			   sizeof(Pkmn_list_data_pkt) +
			   (num_moves =
			    session->user[user_id].active_data.num_moves) *
			   sizeof(Pkmn_move_data_pkt)));
  current = payload;
  pkt = (Server_status_pkt * )current;
  memset(pkt, 0, sizeof(*pkt));
  
  pkt->packet_type = htonl(SERVER_STATUS_PACKET);
  /* get the session status */
#if(DEBUG_PACKETS)
  fprintf(stderr, "~~~session status: %i\n",
	  session->status);
#endif
  pkt->major_status =
    get_user_change_from_status(session->status, user_id);
#if(DEBUG_PACKETS)
  fprintf(stderr, "~~~user status: %i\n",
	  pkt->major_status);
#endif
  /* get the opponents pokeballs */
  pkt->opp_fainted_ct = session->user[!user_id].fainted;
#if(DEBUG_PACKETS)
  fprintf(stderr, "~~~packing fainted: %i\n",
	  pkt->opp_fainted_ct);
#endif
  pkt->opp_normal_ct = get_normal_count(session,
					!user_id);
#if(DEBUG_PACKETS)
  fprintf(stderr, "~~~packing normal: %i\n",
	  pkt->opp_normal_ct);
#endif
  pkt->opp_status_ct = get_status_count(session,
				       !user_id);
#if(DEBUG_PACKETS)
  fprintf(stderr, "~~~packing statused: %i\n",
	  pkt->opp_status_ct);
#endif

  /* get the opponent's active pkmn */
  pkt->opp_number = session->user[!user_id].active_data.number;
  pkt->opp_level = session->user[!user_id].pkmn
    [session->user[!user_id].num_active].level;
  pkt->opp_status =
    ( session->user[!user_id].pkmn
      [session->user[!user_id].num_active].status > NRM &&
      session->user[!user_id].pkmn
      [session->user[!user_id].num_active].status <= SLP ) ?
    SLP : session->user[!user_id].pkmn
    [session->user[!user_id].num_active].status;
  
  pkt->opp_hp_bars = get_hp_bars(&session->user[!user_id].pkmn
				[session->user[!user_id].num_active]);
  
  strncpy(pkt->opp_nickname, session->user[!user_id].pkmn
	  [session->user[!user_id].num_active].nickname,
	  sizeof(char) * 32);

  /* get your active pkmn */
  pkt->active = htonl(session->user[user_id].num_active);
  pkt->num_pkmn = htonl(num_pkmn);
  pkt->num_moves = htonl(num_moves);
  
  current += sizeof(Server_status_pkt);
  
  /* get your pkmn list */
  for(i = 0; i < num_pkmn; i++)
    {
      if(i != session->user[user_id].num_active)
	{
	  pack_pkmn_list_data_pkt(&session->user[user_id].pkmn[i],
				  (Pkmn_list_data_pkt *)current);
	}
      else
	{
	  pack_pkmn_list_data_pkt_from_active(&session->user[user_id].
					      active_data,
					      &session->user[user_id].
					      pkmn[i],
					      (Pkmn_list_data_pkt *)current);
	}
      current += sizeof(Pkmn_list_data_pkt);
    }

  /* get your moves to display */
  for(i = 0; i < num_moves; i++)
    {      
      ((Pkmn_move_data_pkt *)current)->number =
	session->user[user_id].active_data.move[i].number;
#if(DEBUG_PACKETS)
      fprintf(stderr, "~~~packing move %i\n",
	      ((Pkmn_move_data_pkt *)current)->number);
#endif
      ((Pkmn_move_data_pkt *)current)->pp =
	session->user[user_id].active_data.move[i].current_pp;
      ((Pkmn_move_data_pkt *)current)->restriction = 0;
      current += sizeof(Pkmn_move_data_pkt);
    }

  send_package(sock, user_name, payload, payload_len);
  return 1;
}
			       

int receive_client_response(Client_response * cr,
			    int sock,
			    char * user_name)
{
  
  Client_response_pkt * pkt;
  int pkt_len;
  /* get the user initialization data */
  if(receive_package(sock, user_name,
		     (void **)&pkt, &pkt_len) < 0)
    {
      return RECEIVE_ERROR;
    }

  cr->command = ntohs(pkt->command);
  cr->option = ntohs(pkt->option);
  free(pkt);
  return 1;
}

int send_event_logs(Event * events,
		    unsigned int num_events,
		    int sock,
		    unsigned int user_id,
		    char * user_name)
{
  uint16_t * event_out;
  int client_event_count = 0;
  int i;
  char * buffer;
  int buffer_size;
  int byte_offset;
  /* the initial byte offset is past the packet type */

  /* count the number of events that go to the client */
  for(i = 0; i < num_events; i++)
    {
      if(events[i].outputs & CLIENT_LOG_OUTPUT)
	{
	  client_event_count++;
	}
    }
  
  /* allow the packet type, a variable number of events, and
     a terminal event */
  buffer_size = sizeof(uint32_t) +
    (client_event_count * sizeof(uint16_t))
    + sizeof(uint16_t);
  buffer = malloc(buffer_size);
  /* first four types are packet type */
  *((uint32_t *)buffer) = htonl(EVENT_LOG_PACKET);
  /* set first event to be offset past the header*/
  byte_offset = sizeof(uint32_t);
  event_out = (uint16_t *)(buffer + byte_offset);
  
  for(i = 0; i < num_events; i++) {
    if(events[i].outputs & CLIENT_LOG_OUTPUT)
      {
#if(DEBUG_PACKETS)
	fprintf(stderr, "~~~event code: %i, abs user %i, details %i\n",
		events[i].message_id, events[i].absolute_actor_id,
		events[i].details);
#endif
    
	*event_out = 0;
	*event_out |= (events[i].message_id &
		       MESSAGE_MASK) << MESSAGE_SHIFT;
	*event_out |= (!(events[i].absolute_actor_id ^ user_id))
	  << ACTOR_SHIFT;
	*event_out |= events[i].details & DETAILS_MASK;
#if(DEBUG_PACKETS)
	fprintf(stderr, "~~~event code: %i\n", *event_out);
#endif
	*event_out = htons(*event_out);
	
	/* increase the offset in the buffer by 1 event */ 
	byte_offset += sizeof(uint16_t);
	/* point to the next event */
	event_out = (uint16_t *)(buffer + byte_offset);
      }
  }

  
  *event_out = htons(END_EVENTS);
  if(send_package(sock, user_name, buffer, buffer_size) < 0)
    {
      return SEND_ERROR;
    }
  return 1;
}
