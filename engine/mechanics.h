#ifndef __OPENPKMN_MECHANICS__
#define __OPENPKMN_MECHANICS__

#include "environment.h"

#define MAX_SLP_TURNS 8
#define REST_TURNS    2
#define SE 2
#define NVE .5

enum base_modifiers { BASE_STAT_MOD = 6, BASE_EVADE_MOD = 0,
		      BASE_ACCURACY_MOD = 7 };

enum max_modifiers { MAX_STAT_MOD = 12, MAX_HIT_MOD = 6 };
/* though there are 8 values in the array, we should never reach that
 * one through normal means (it is reserved for invulnerable) */

enum min_modifiers { MIN_STAT_MOD = 0, MIN_HIT_MOD = 0};

enum invalid_mod_values { INVALID_EVADE_MOD = 8};

enum damage_randomness { RANDOM_BASE = 217, RANDOM_RANGE = 39 };

enum stat_change_levels { GREATLY = 2, NORMALLY = 1 };

/* [attacking type][defending type] */
extern const float type_chart[TYPE_MAX][TYPE_MAX];

extern const float hit_mods_val[8];
extern const float stat_mod_val[13];

extern unsigned int calc_true_attack(unsigned int base_attack,
				     unsigned int mod_val,
				     unsigned int status);

extern unsigned int calc_true_defense(unsigned int base_defense,
				      unsigned int mod_val,
				      unsigned int reflect_active);

extern unsigned int calc_true_speed(unsigned int base_speed,
				    unsigned int mod_val,
				    unsigned int status);
 
extern unsigned int calc_true_special(unsigned int base_special,
				      unsigned int mod_val,
				      unsigned int light_screen_active);


extern unsigned int get_attacking_stat(int type);

extern unsigned int get_defending_stat(int type);

float modcalc(int statmod, int status, int ls, int refl, int stat);

extern const int move_power[];
extern const int move_accuracy[];
extern const int move_secondary_effect[];
extern const int move_effect_chance[];
extern const int move_parameter[];


enum secondary_effects{  
  ATT_UP_NRM = 1,
  ATT_UP_GRT,
  DEF_UP_NRM,
  DEF_UP_GRT,
  SPD_UP_GRT,
  SPEC_UP_GRT,
  SPEC_UP_NRM,
  EVD_UP_NRM,
  CRITICAL_HIT_UP,
  /* CAUSE STATISTIC DECREASES */
  ATT_DOWN_NRM,
  DEF_DOWN_NRM,
  DEF_DOWN_GRT,
  SPD_DOWN_NRM,
  SPEC_DOWN_NRM,
  ACC_DOWN_NRM,
  /* CAUSE EXCLUSIVE STATUS CHANGES */
  FREEZE,
  PARALYZE,
  BURN,
  SLEEP,
  POISON_MILD,
  POISON_BAD,
  /* CAUSE NON-EXCLUSIVE STATUS CHANGES */
  IMMOBILIZE,
  CONFUSE,
  SEED,
  /* ATTACKER N-E STATUS CHANGES */
  HALVE_SPEC_DAM,
  HALVE_PHYS_DAM,
  STATISTIC_LOCK,
  COPY_PKMN,
  /* CAUSE OTHER ATTACKER EFFECTS */
  LEECH,
  THRASH_ABOUT,
  CONVERT_TYPE,
  QUARTER_RECOIL,
  SUICIDE,
  ONE_DAMAGE_RECOIL,
  COPY_MOVE,
  HEAL_HALF,
  SLEEP_AND_HEAL,
  MAKE_SUB,
  RECHARGE,
  GET_ENRAGED,
  START_BIDING,
  /* CAUSE OTHER DEFENDER EFFECTS */
  FLINCH,
  DISABLE_MOVE,
  /* EFFECT BOTH SIDES */
  ELIMINATE_CHANGES };


enum parameters {
  REPEAT_2_5_TIMES = 1,
  REPEAT_2_TIMES,
  STRIKE_LAST,
  EFFECT_IGNORES_TYPE,
  SIMPLE_DAMAGE,
  SLP_NEEDED,
  IGNORE_EVD_ACC,
  STRIKE_FIRST,
  LOWER_HEAD,
  IN_AIR,
  UNDERGROUND,
  HIGH_CRITICAL_HIT,
  SECONDARY_ON_MISS,
  SECONDARY_WITHOUT_HIT, 
  ONLY_IF_FIRST,
  RANDOM_ATTACK,
  OPPONENTS_LAST_ATTACK,
  SELF_TARGETING,
  MAKE_WHIRLWIND,
  GLOW,
  GATHER_SUNLIGHT
 };

/* ALTERNATE DAMAGES */
enum alt_damages {
  OHKO = -1,
  HALF = -2,
  PW_VARIABLE = -3,
  
  ATTACKER_LEVEL = -4,
  DOUBLE_COUNTERABLE = -5 };

enum damage_constants { CONFUSION_DAMAGE = 40 };

#endif
