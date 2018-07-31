#ifndef __OPENPKMN_ENGINE_PACKET__
#define __OPENPKMN_ENGINE_PACKET__ 1

#include <stdint.h>
#include <string.h>

#include "datatypes.h"
#include "events.h"

#include "../share/packet.h"
			  

typedef struct pkmn_list_data_pkt
{
  uint8_t number;
  uint8_t level;
  uint8_t status;
  uint8_t reserved;

  uint16_t current_hp;
  uint16_t max_hp;

  char nickname[32];
} Pkmn_list_data_pkt;

typedef struct pkmn_move_data_pkt {
  uint8_t number;
  uint8_t pp;
  uint8_t restriction;
  uint8_t reserved;
} Pkmn_move_data_pkt;

enum major_statuses {
  MS_NORMAL_ROUND = 0,
  MS_OWN_FAINT = 1,
  MS_OPP_FAINT = 2,
  MS_BOTH_FAINT = 3,
  MS_OWN_LOSS = 4,
  MS_OPP_LOSS = 5,
  MS_TIE = 6};

typedef struct server_status_pkt
{
  uint32_t packet_type;
  
  uint8_t major_status;
  uint8_t opp_fainted_ct;
  uint8_t opp_status_ct;
  uint8_t opp_normal_ct;

  uint8_t opp_number;
  uint8_t opp_level;
  uint8_t opp_status;
  uint8_t opp_hp_bars;

  char opp_nickname[32];
  uint32_t active;
  uint32_t num_pkmn;
  uint32_t num_moves;
} Server_status_pkt;

extern int send_server_status_packet(int sock, char * user_name,
				     unsigned int user_id,
				     Session * session);
extern int receive_client_response(Client_response * cr,
				   int sock, char * user_name);

typedef struct client_response_pkt {
  uint32_t packet_type;
  
  uint16_t command;
  uint16_t option;
} Client_response_pkt;

#define END_EVENTS    0xffff
#define MESSAGE_MASK  0x7f
#define MESSAGE_SHIFT 9
#define ACTOR_SHIFT   8
#define DETAILS_MASK  0xff

extern int send_event_logs(Event * events,
			   unsigned int num_events,
			   int sock,
			   unsigned int user_id,
			   char * user_name);

#endif
