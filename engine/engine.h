#ifndef __OPENPKMN_ENGINE__
#define __OPENPKMN_ENGINE__ 1

/* do we want to expose this much */

#include "datatypes.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

extern Session session;     
enum priorities { LOW_PRIORITY = -1,
		  NORMAL_PRIORITY = 0,
		  HIGH_PRIORITY = 1 };

enum turn_statuses { NO_EVENTS = 0,
		     ATTACKER_FAINTS = 0x01,
		     DEFENDER_FAINTS = 0x02,
		     ATTACKER_PREVENTED_FROM_ATTACKING = 0x04,
		     ATTACKER_MOVE_MISSES = 0x08,
		     ATTACKER_MOVE_DOES_NO_DAMAGE = 0x10,
		     ATTACKER_PREVENTED_FROM_SECONDARY = 0x20,
		     SUPRESS_PP_USE = 0x80,
		     DEFENDER_SUB_BROKE = 0x100,
		     ATTACKER_FINISHED_CHARGING = 0x200};

int std_damage_calc(unsigned int att_lvl, unsigned int att_stat,
		    unsigned int pow, unsigned int def_stat,
		    float stab, float tmod, unsigned int rand);

int compute_attack_order(int speed1, int priority1, int speed2, int priority2);

void correct_commands_for_faint(int previous_status, int * command1,
				int * command2);

Pkmn * can_switch(int switch_to, unsigned int total, Pkmn * active,
		  Pkmn * team);

unsigned int compute_round(int command0, int option0,
			   int command1, int option1);


unsigned int process_events(unsigned int events,
			    unsigned int attacker_id,
			    unsigned int * attacker_fainted,
			    unsigned int attacker_total,
			    unsigned int * defender_fainted,
			    unsigned int defender_total);


void apply_self_confusion(unsigned int * confusion_turns);

void do_sendout(int starter0, int starter1);

#endif
