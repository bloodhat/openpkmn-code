#ifndef __OPENPKMN_SHARE_PACKET__
#define __OPENPKMN_SHARE_PACKET__ 1

#include <stdint.h>

#include "datatypes.h"

enum packet_types { NULL_PACKET = 0, LOGIN_PACKET = 1,
		    SERVER_RESPONSE_PACKET = 2, PKMN_SETUP_PACKET = 3,
		    PKMN_DATA_PACKET = 4, WAIT_PACKET = 5,
		    CHALLENGE_PACKET = 6, ENGINE_PARAMS_PACKET = 7,
		    SERVER_STATUS_PACKET = 8, EVENT_LOG_PACKET = 9,
		    CLIENT_RESPONSE = 10, LOGOUT_PACKET = 11,
		    ENGINE_START_PACKET = 12, ENGINE_DONE_PACKET = 13,
		    AVAILABLE_PKMN_LIST_PACKET = 14,
		    REQUEST_AVAILABLE_PKMN_PACKET = 15,
		    REQUEST_AVAILABLE_TEAMS_PACKET = 16,
		    FORM_TEAM_PACKET = 17, AVAILABLE_TEAMS_LIST_PACKET = 18,
		    REQUEST_AVAILABLE_USERS_PACKET = 19,
		    AVAILABLE_USERS_LIST_PACKET = 20, PREFERENCE_PACKET = 21,
		    CHALLENGE_NOTIFICATION_PACKET = 22,
		    CHALLENGE_RESPONSE_PACKET = 23, CANCEL_PACKET = 24,
		    CHAT_MESSAGE_PACKET = 25, CHAT_TEXT_PACKET = 26,
		    HIDE_PKMN_PACKET = 27, HIDE_TEAM_PACKET = 28,
		    BATTLE_NOW_PACKET = 29, RANDOM_BATTLE_NOW_PACKET = 30,
		    USER_CHANGE_PACKET = 31
};

int receive_package(int sock, char * user_name, void ** payload,
		    int * payload_len);


int receive_package_unverified(int sock, char * user_name, void ** payload,
			       int * payload_len, int * packet_type);


int verify_package(char * user_name, void ** payload, int * payload_len,
		   void * encrypted_payload, int encrypted_len,
		   int packet_type);


int send_package(int sock, char * user_name, void * payload, int payload_len);

int send_error(int sock, char * user_name);

int send_ack(int sock, char * user_name);

int send_engine_params(int sock, char * opponent_name, int port,
		       char * user_name);

typedef struct header
{
  uint32_t packet_type;
  char user_name[32];
  uint32_t encryption_type;
  uint32_t packet_len;
} Header;

typedef struct packet
{
  uint32_t packet_type;
} Packet;


#define ACK_CODE 1
#define ERR_CODE 2
typedef struct server_response_packet
{
  uint32_t packet_type;
  uint32_t response_code;
} Server_response_packet;

typedef struct pkmn_data_pkt
{
  uint32_t packet_type;
  
  char nickname[32];
  
  uint8_t number;
  uint8_t level;
  uint16_t max_hp;
  
  uint16_t attack;
  uint16_t defense;
  
  uint16_t speed;
  uint16_t special;
  
  uint32_t num_moves;
  /* with a variable number of move_data_pkts */
} Pkmn_data_pkt;


typedef struct engine_params_pkt {
  uint32_t packet_type;
  uint32_t port;
  char opponent_name[32];
} Engine_params_pkt;

#define MANAGER_SRC_TYPE 1
#define ENGINE_SRC_TYPE  2
				   
#endif
