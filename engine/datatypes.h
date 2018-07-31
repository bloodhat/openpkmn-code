#ifndef __OPENPKMN_ENGINE_DATA_TYPES__
#define __OPENPKMN_ENGINE_DATA_TYPES__

#include "../share/datatypes.h"

typedef struct active_data
{
  unsigned int number;

  int type[2];

  Stats current_stats;

  Move move[4];
  unsigned int num_moves;

  int evade_mod;
  int accuracy_mod;

  Stat_mods stat_mods;

  unsigned int physical_halve;
  unsigned int special_halve;

  unsigned int critical_hit_up;

  unsigned int mod_lock;

  int substitute;

  unsigned int toxic_turns;
  unsigned int confusion_turns;

  unsigned int seeded;
  unsigned int enraged;
  unsigned int flinched;

  unsigned int recharging;

  unsigned int disabled_selection;
  unsigned int disable_turns;

  unsigned int trapping_damage;
  unsigned int trapping_move;
  unsigned int trapping_turns;

  unsigned int original_trapping_victim;

  unsigned int thrashing_move;
  unsigned int thrashing_turns;

  unsigned int charging_move;
  unsigned int temp_invulnerable;
  unsigned int repeated_move_index;

  unsigned int damage_taken;
  unsigned int damage_is_counterable;

  unsigned int biding_turns;
  unsigned int bide_damage;

  unsigned int last_move_used;
} Active_data;


typedef struct user
{
  char name[32];
  Pkmn pkmn[6];
  unsigned int starter;
  unsigned int num_active;
  unsigned int total;
  unsigned int fainted;

  int id;

  unsigned int sleep_count;
  unsigned int frozen_count;

  Active_data active_data;
} User;

typedef struct session
{
  User user[2];
  unsigned int status;
  unsigned int round;
  int enforced_clauses;
} Session;


enum commands {NO_COMMAND = 0, SWITCHING = 2, ATTACKING = 1, RUNNING = 3};

typedef struct client_reponse
{
  unsigned int command;
  unsigned int option;
} Client_response;

extern void init_active_data(Active_data * a_data);

#endif
