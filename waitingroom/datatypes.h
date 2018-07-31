#ifndef __OPENPKMN_WAITINGROOM_DATATYPES__
#define __OPENPKMN_WAITINGROOM_DATATYPES__

#include <pthread.h>

typedef struct thread_args
{
  pthread_t thread_ptr;
  int sock;
} Thread_args;

enum user_statuses { USER_STATUS_OFFLINE = 0, USER_STATUS_INITIAL = 1,
		     USER_STATUS_WAITING = 2, USER_STATUS_CHALLENGED = 3,
		     USER_STATUS_AWAITING_RESPONSE = 4,
		     USER_STATUS_BATTLE_PENDING = 5, USER_STATUS_BATTLING = 6,
		     USER_STATUS_FINDING_BATTLE = 7,
		     USER_STATUS_FINDING_RANDOM = 8,
		     USER_STATUS_WAITING_RANDOM = 9};

typedef struct user
{
  pthread_mutex_t user_lock;
  pthread_cond_t user_sig;
  int wait_sock;
  int chat_sock;
  int current_opponent;
} User;

#define NUM_USERS 1024
extern User * users[NUM_USERS];

int get_current_opponent_id(int user_id, int * opponent_id);

int set_current_opponent_id(int user_id, int opponent_id);

#endif
