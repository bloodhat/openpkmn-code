#include "datatypes.h"

User * users[NUM_USERS];

int get_current_opponent_id(int user_id, int * opponent_id)
{
  *opponent_id = users[user_id]->current_opponent;
  return 1;
}

int set_current_opponent_id(int user_id, int opponent_id)
{
  users[user_id]->current_opponent = opponent_id;
  return 1;
}
