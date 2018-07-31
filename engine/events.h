#ifndef __OPENPKMN_EVENTS__
#define __OPENPKMN_EVENTS__ 1

#define NUM_EVENTS 40

#define LOG_RESET() log_counter = 0; 

#define ATTACKER_ACTOR 1
#define DEFENDER_ACTOR 0

#define ABSOLUTE_ACTOR_0 0
#define ABSOLUTE_ACTOR_1 1

#define CLIENT_LOG_OUTPUT 0x1
#define DATABASE_LOG_OUTPUT 0x2

typedef struct event
{
  unsigned int message_id;
  unsigned int relative_actor_id;
  unsigned int absolute_actor_id;
  int details;
  unsigned int outputs;
} Event;
  
extern Event log_register[NUM_EVENTS];
extern unsigned int log_counter;

extern void log_event(unsigned int event_id,
		      unsigned int actor_id,
		      unsigned int details,
		      unsigned int outputs);

extern void buffer_event(unsigned int event_id,
			 unsigned int actor_id,
			 unsigned int details,
			 unsigned int outputs,
			 Event * buffer,
			 unsigned int * counter);

extern void consume_buffer(Event * event_buffer, unsigned int event_count);

enum events {
  SENT_OUT = 0,
  USED_MOVE = 1,
  MOVE_MISSED = 2,
  MOVE_FAILED = 3,
  NOT_VERY_EFFECTIVE = 4,
  SUPER_EFFECTIVE = 5,
  NO_EFFECT = 6,

  FLEW = 7,
  DUG = 8,
  LOWERED_HEAD = 9,

  CRITICAL_HIT = 10,
  HIT_N_TIMES = 11, 

  BUILDING_ENERGY = 12,
  UNLEASHED_ENERGY = 70,

  SUBSTITUTE_BROKE = 13,
  RAGE_BUILDING = 14,

  ATTACK_ROSE = 15,
  ATTACK_GREATLY_ROSE = 16,
  DEFENSE_ROSE = 17,
  DEFENSE_GREATLY_ROSE = 18,
  SPEED_ROSE = 19,
  SPEED_GREATLY_ROSE = 20,
  SPECIAL_ROSE = 21,
  SPECIAL_GREATLY_ROSE = 22,
  EVADE_ROSE = 23,
  GETTING_PUMPED = 24,

  ATTACK_FELL = 25,
  ATTACK_GREATLY_FELL = 26,
  DEFENSE_FELL = 27,
  DEFENSE_GREATLY_FELL = 28,
  SPEED_FELL = 29,
  SPEED_GREATLY_FELL = 30,
  SPECIAL_FELL = 31,
  SPECIAL_GREATLY_FELL = 32,
  ACCURACY_FELL = 33,
  
  HURT_BY_BURN = 34,
  HURT_BY_POISON = 35,
  HURT_BY_LEECH_SEED = 36,

  FELL_ASLEEP = 37,
  WAS_PARALYZED = 38,
  WAS_BURNED = 39,
  WAS_POISONED = 40,
  WAS_BADLY_POISONED = 41,
  WAS_FROZEN = 42,
  
  WAS_CONFUSED = 43,
  WAS_SEEDED = 44,

  STATUS_CHANGES_ELIMINATED = 45,
  
  PROTECTED_SPECIAL = 46,
  GAINED_ARMOR = 47,
  SHROUDED = 48,

  TRANSFORMED_INTO = 49,
  CONVERTED_TYPE = 50,
  LEARNED_MOVE = 51,

  REGAINED_HEALTH = 52,
  STARTED_SLEEPING = 53,

  CREATED_SUBSTITUTE = 54,

  WOKE_UP = 55,
  IS_ASLEEP = 56,
  IS_FROZEN = 57,
  IS_FULLY_PARALYZED = 58,

  THAWED = 59,
  FLINCHED = 60, 

  DISABLED_MOVE = 61,

  IS_CONFUSED = 62,
  HURT_BY_CONFUSION = 63,
  
  MUST_RECHARGE = 64,
  CANNOT_MOVE = 65,
  
  THRASHING_ABOUT = 66,
  HIT_BY_RECOIL = 67,
  
  NO_LONGER_CONFUSED = 68,
  NO_LONGER_DISABLED = 69,

  FAINTED = 72,
  SUCKED_HP = 73,
  ATTACK_CONTINUES = 74,

  USER_WINS = 75,
  USER_LOSES = 76,
  USERS_TIE = 77,
  
  RAN_AWAY = 78,
  
  DAMAGE_RANDOMIZER = 79,
  HAS_STATUS = 80,
  DAMAGE_DONE = 81,
  HP_REMAINING = 82,
  SUB_REMAINING = 83,

  MOVE_DISABLED = 84,

  MADE_WHIRLWIND = 85,
  IS_GLOWING = 86,
  GATHERED_SUNLIGHT = 87,
 
  BEGIN_TURN = 255
};


void translate_relative_events(Event * events,
			       unsigned int num_events,
			       unsigned int actor);
  

#endif

