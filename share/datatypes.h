#ifndef __OPENPKMN_SHARE_DATA_TYPES__
#define __OPENPKMN_SHARE_DATA_TYPES__ 1

enum update_operation{ WRITE_BACK, LEAVE_AS_IS };

typedef struct move
{
  int number;
  int current_pp;

  /*unsigned int use_flag;*/
  unsigned int update_flag;
} Move;

typedef struct stats
{
  int attack;
  int defense;
  int speed;
  int special;
} Stats;

typedef struct mods
{
  int attack;
  int defense;
  int speed;
  int special;
} Stat_mods;

enum infliction_mechanism { SELF, OPPONENT };

typedef struct pkmn
{
  char nickname[32];

  int id;

  int number;
  unsigned int level;
  unsigned int status;
  unsigned int sleep_infliction_mechanism;

  int current_hp;
  int max_hp;
    
  Stats unmod_stats;
  unsigned int roster_number;

  Move move[4];
  unsigned int num_moves;
} Pkmn;

enum disabled_status{ NONE_DISABLED = 5};


typedef struct move_ll {
  int move_num;
  struct move_ll * next;
} Move_ll;

typedef struct move_record_ll {
  int move_num;
  int ruleset;
  int pp;
  struct move_record_ll * next;
} Move_record_ll;

typedef struct species_ll {
  int species_num;
  int ruleset;
  char name[32];
  struct species_ll * next;
} Species_ll;

typedef struct pkmn_ll {
  int species;
  int index;
  int ruleset;
  struct pkmn_ll * next;
  struct move_ll * moves;
} Pkmn_ll;

typedef struct member_ll {
  int index;
  struct member_ll * next;
} Member_ll;

typedef struct team_ll {
  int index;
  int ruleset;
  struct team_ll * next;
  struct member_ll * members;
} Team_ll;

typedef struct user_ll {
  int id;
  char user_name[32];
  int ruleset;
  int status;
  int team_rules;
  struct user_ll * next;
} User_ll;

#endif
