#ifndef __OPENPKMN_WAITINGROOM_PACKET__
#define __OPENPKMN_WAITINGROOM_PACKET__

#include <stdint.h>

#include "../share/datatypes.h"
#include "datatypes.h"

typedef struct login_pkt
{
  uint32_t packet_type;
} Login_pkt;

typedef struct pkmn_setup_pkt
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
} Pkmn_setup_pkt;

typedef struct move_setup_pkt
{
  uint32_t number;
  uint32_t pp;
} Move_setup_pkt;

typedef struct request_available_users_pkt
{
  uint32_t packet_type;
} Request_available_users_pkt;


typedef struct available_users_list_pkt
{
  uint32_t packet_type;
  uint32_t num_users;
} Available_users_list_pkt;

typedef struct available_user_pkt
{
  uint32_t id;
  char user_name[32];
  uint32_t ruleset;
  uint32_t status;
  uint32_t team_rules;
} Available_user_pkt;

typedef struct request_available_pkmn_pkt
{
  uint32_t packet_type;
} Request_available_pkmn_pkt;

typedef struct available_pkmn_list_pkt
{
  uint32_t packet_type;
  uint32_t num_pkmn;
} Available_pkmn_list_pkt;

typedef struct available_pkmn_pkt
{
  uint32_t index;
  uint32_t number;
  uint32_t rules;
  uint32_t num_moves;
} Available_pkmn_pkt;

typedef struct available_pkmn_move_pkt
{
  uint32_t number;
} Available_pkmn_move_pkt;

typedef struct request_available_teams_pkt
{
  uint32_t packet_type;
} Request_available_teams_pkt;

typedef struct available_team_list_pkt
{
  uint32_t packet_type;
  uint32_t num_teams;
} Available_team_list_pkt;

typedef struct available_team_pkt
{
  uint32_t index;
  uint32_t rules;
  uint32_t num_members;
} Available_team_pkt;

typedef struct available_team_member_pkt
{
  uint32_t index;
} Available_team_member_pkt;

typedef struct form_team_pkt
{
  uint32_t packet_type;
  uint32_t num_members;
} Form_team_pkt;

typedef uint32_t Team_member;


typedef struct preference_pkt
{
  uint32_t packet_type;
  uint32_t rules_value;
} Preference_pkt;


typedef struct wait_pkt
{
  uint32_t packet_type;
  uint32_t with_team;
  uint32_t starter;
} Wait_pkt;

typedef struct challenge_pkt
{
  uint32_t packet_type;
  uint32_t opponent_id;
  uint32_t with_team;
  uint32_t starter;
} Challenge_pkt;

typedef struct challenge_notification_pkt
{
  uint32_t packet_type;
  uint32_t challenger_id;
} Challenge_notification_pkt;

#define CHALLENGE_ACCEPTED 1
#define CHALLANGE_REJECTED 0

typedef struct challenge_response_pkt
{
  uint32_t packet_type;
  uint32_t accepted;
} Challenge_response_pkt;

typedef struct cancel_pkt
{
  uint32_t packet_type;
} Cancel_pkt;

typedef struct chat_message_pkt
{
  uint32_t packet_type;
  char recipients[NUM_USERS/8];
  char message[4096];
} Chat_message_pkt;

typedef struct chat_text_pkt
{
  uint32_t packet_type;
  char sender_name[32];
  char message[4096];
} Chat_text_pkt;

typedef struct hide_pkmn_pkt
{
  uint32_t packet_type;
  uint32_t pkmn_id;
} Hide_pkmn_pkt;

typedef struct hide_team_pkt
{
  uint32_t packet_type;
  uint32_t team_id;
} Hide_team_pkt;
  

typedef struct battle_now_pkt
{
  uint32_t packet_type;
  uint32_t with_team;
  uint32_t starter;
} Battle_now_pkt;


typedef struct random_battle_now_pkt
{
  uint32_t packet_type;
} Random_battle_now_pkt;

typedef struct user_change_pkt {
  uint32_t packet_type;
  Available_user_pkt user;
  uint32_t new;
} User_change_pkt;

int process_pkmn_setup_packet(void * decrypted_payload, int user_id);
  
int send_available_pkmn_list_packet(int sock, char * user_name,
				    Pkmn_ll * pkmn, int num_pkmn,
				    int num_moves);

int send_available_team_list_packet(int sock, char * user_name,
				     Team_ll * teams, int num_teams,
				    int num_members);

int send_available_users_list_packet(int sock, char * user_name,
				     User_ll * users, int num_users);
				    
int send_challenge_notification_packet(int sock, char * user_name,
				       int challenger_id);

int send_response_notification(int sock, char * user_name, int affirmative);

int receive_challenge_response(int sock, char * user_name, int * response);

int send_chat_text_packet(int sock, char * user_name, char * sender,
			  char * message);

int process_chat_message_packet(Chat_message_pkt * decrypted_payload,
				char * recipients, char * message);


int send_user_change_packet(int sock, char * user_name,
			    unsigned int updated_id, char * updated_name,
			    unsigned int updated_rules,
			    unsigned int updated_status,
			    unsigned int updated_team_rules, int new);
#endif
