#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <univalue.h>


#include "datatypes.h"
#include "engine.h"
#include "environment.h"
#include "events.h"
#include "mechanics.h"

#include "db_iface.h"
#include "engine_conf.h"
#include "eutil.h"

#include "../share/entropy.h"
#include "../share/move_defs.h"
#include "../share/preferences.h"

#define DEBUG_ENGINE 1

Session session;

/* priority_of
 *
 * TAKES
 * move_num the number of the move
 *
 * RETURNS
 * the priority of the move
 */
int priority_of(unsigned int move_parameter)
{
  if(move_parameter == STRIKE_FIRST)
    {
      return HIGH_PRIORITY;
    }
  else if(move_parameter == STRIKE_LAST)
    {
      return LOW_PRIORITY;
    }
  return NORMAL_PRIORITY;
}

/* std_damage_calc
 *
 * TAKES
 * att_level: level of attacking pkmn
 * att_stat: value of relevent attacking stat
 * pow: power of the move
 * def_stat: value of the relevent defending stat
 * stab: the same type attack bonus modifier (1 or 1.5)
 * tmod: the type effectiveness modifier
 * rand: a random number 217-255
 *
 * RETURNS
 * the amount of damage to be done
 */
int std_damage_calc(unsigned int att_lvl, unsigned int att_stat,
		    unsigned int pow, unsigned int def_stat,
		    float stab, float type_mod, unsigned int rand)
{
  int damage;
#if (DEBUG_ENGINE)
  fprintf(stderr, "***attacker level: %i\n***attacking stat: %i\n"
	  "***attacker power: %i\n***defending stat: %i\n"
	  "***stab: %f\n***mod %f\n***rand: %i\n",
	  att_lvl, att_stat, pow, def_stat, stab, type_mod, rand);
#endif
  log_event(DAMAGE_RANDOMIZER, ATTACKER_ACTOR,
            rand, DATABASE_LOG_OUTPUT);
  damage = ((((min(((((((2 * att_lvl)/5) + 2) * att_stat * pow)/
		      max(1, def_stat)) / 50), 997) + 2) * stab)
	      * type_mod) * rand) / 255;
  return damage;
}

int confusion_damage_calc(unsigned int att_lvl, unsigned int att_stat,
			  unsigned int def_stat)
{
  int damage;
  damage = min((40 * att_lvl * ((2 * att_lvl / 5) + 2)
		/ def_stat) / 50, 997) + 2;
  return damage;
}

/* compute_attack_order
 *
 * TAKES
 * speed1,2 the speed of the attacker
 * priority1,2 the priority of the move used
 *
 * RETURNS 0 if 1 is fast 1 if 2 is faster
 */
int compute_attack_order(int speed1, int priority1, int speed2, int priority2)
{
  int pf[2] = {0,0}; /* priority factors */

  /* allow moves with altered priority
   *  to override speed while allowing
   *  speed to still have an effect
   *  if same-priority moves are used */
  pf[0] = ((speed1 > speed2)? 1:0) + ((priority1 == HIGH_PRIORITY)? 2:0)
    + ((priority1 == LOW_PRIORITY)? -2:0);

  pf[1] =((speed2 > speed1)?1:0) + ((priority2 == HIGH_PRIORITY)? 2:0)
    + ((priority2 == LOW_PRIORITY)? -2:0);
  /* assign random numbers to each until they
   * are different */
  while(pf[0] == pf[1])
    {
      pf[0] = genrand(INT_MAX, 0);
      pf[1] = genrand(INT_MAX, 0);
    }
  return (pf[0] > pf [1]) ? 0 : 1;
}

/* correct_commands_for_faint
 *
 * TAKES
 * previous_status the last round's status
 * command1,2_ptr pointers to the two commands
 *
 * RETURNS nothing
 */
void correct_commands_for_faint(int previous_status, int * command0,
				int * command1)
{

  if(previous_status == FAINTED_0)
    {
      *command0 = SWITCHING;
      *command1 = NO_COMMAND;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***correcting for faint of user 0\n");
#endif
    }
  else if(previous_status == FAINTED_1)
    {
      *command0 = NO_COMMAND;
      *command1 = SWITCHING;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***correcting for faint of user 1\n");
#endif
    }
  else if(previous_status == FAINTED_BOTH)
    {
      *command0 = *command1 = SWITCHING;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***correcting for faint of both\n");
#endif
    }
}

/* can_switch
 *
 * TAKES
 * switch_to the pkmn attempted to be switched to
 * total the total number of active pkmn
 * active the number of the active pkmn
 * team the array of pkmn
 *
 * RETURNS null if a switch cannot be made,
 * to whom the switch can be made otherwise
 */
Pkmn * can_switch(int switch_to, unsigned int total, Pkmn * active,
		  Pkmn * pkmn_list)
{
  int valid_switch = 1;
  int i;
  /* sanitize client input */
  switch_to = abs(switch_to) % total;

   /* invalid switch if same or to fainted*/
  if(switch_to == active->roster_number ||
     pkmn_list[switch_to].status == FNT)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***attempting an illegal switch\n");
#endif
      valid_switch = 0;
      for(i = 0; i < total; i++)
	{
	  if(pkmn_list[i].status != FNT && i != active->roster_number)
	    {
	      switch_to = i;
	      valid_switch = 1;
	      break;
	    }
	}
    }
  return (valid_switch) ? &(pkmn_list[switch_to]) : NULL;
}

void write_active_data(Pkmn * pkmn, Active_data * a_data)
{
  int i;

  /* copy the newly active pkmn's data to the active data */
  a_data->number = pkmn->number;
  get_first_type(pkmn->number, &a_data->type[0]);
  get_second_type(pkmn->number, &a_data->type[1]);

  memcpy(&(a_data->current_stats), &(pkmn->unmod_stats), sizeof(Stats));

  for(i = 0; i < (a_data->num_moves = pkmn->num_moves); i++)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***copying move %i to active data\n",
	      pkmn->move[i].number);
#endif
      memcpy(&(a_data->move[i]), &(pkmn->move[i]), sizeof(Move));

    }

  /* initialize the rest of the active data */
  init_active_data(a_data);

}

void make_faint(Pkmn * pkmn, int enforced_clauses,
		unsigned int * frozen_count, unsigned int * sleep_count)
{
  if(pkmn->status == SLP && pkmn->sleep_infliction_mechanism == OPPONENT &&
     (enforced_clauses & SLEEP_CLAUSE))
    {
      (*sleep_count)--;
    }
  if(pkmn->status == FRZ &&
     (enforced_clauses & FREEZE_CLAUSE))
    {
      (*frozen_count)--;
    }
  pkmn->status = FNT;
}

/* do_switch
 *
 * TAKES
 * switch_to number of the pkmn to switch to
 * active a pointer to the active field
 * modifies the modifiers structure to clear
 *
 * RETURNS nothing
 */

void do_switch(Pkmn * in_pkmn, Pkmn * out_pkmn, unsigned int * num_active,
	       Active_data * a_data, unsigned int enforced_clauses,
	       unsigned int * sleep_count, unsigned int * frozen_count)
{
  int i;

  /* write cached pp decrements to backing store */
  for(i = 0; i < a_data->num_moves; i++)
    {
      if(a_data->move[i].update_flag == WRITE_BACK )
	{
	  out_pkmn->move[i].current_pp = a_data->move[i].current_pp;
	}
   }
  /* write hp changes to backing store */
  /* out_pkmn->current_hp = a_data->current_hp; */

  /* change the active number to incoming pkmn */
  *num_active = in_pkmn->roster_number;
  write_active_data(in_pkmn, a_data);
}

/* apply_damage
 *
 * TAKES:
 * damage - the damage to be done
 * current_hp - current HP of the pkmn
 *
 * RETURNS: the actual damage done
 */
unsigned int apply_damage(unsigned int damage, int * current_hp)
{
  unsigned int actual_damage;
  if(damage < *current_hp)
    {
      log_event(DAMAGE_DONE, ATTACKER_ACTOR, damage, DATABASE_LOG_OUTPUT);
      *current_hp -= damage;
      return damage;
    }
  else
    {
      actual_damage = *current_hp;
      log_event(DAMAGE_DONE, ATTACKER_ACTOR, actual_damage,
		DATABASE_LOG_OUTPUT);
      *current_hp = 0;
      return actual_damage;
    }
}

/* heal
 *
 * TAKES:
 * healing - the amount of health to add
 * current_hp - current HP of the pkmn
 * max_hp - the max HP the pkmn may have
 *
 * RETURNS - the new hp
 */

unsigned int heal(unsigned int healing, unsigned int current_hp,
		  unsigned int max_hp)
{
  if(current_hp + healing > max_hp)
    {
      return max_hp;
    }
  else
    {
      return current_hp + healing;
    }
}

unsigned int inflict_damage(int * damage, int * substitute_hp_ptr,
			    int * current_hp_ptr)
{

  if(substitute_hp_ptr != NULL && *substitute_hp_ptr != 0)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***applying damage to substitute\n");
#endif
      *damage = apply_damage(*damage, substitute_hp_ptr);
      log_event(SUB_REMAINING, DEFENDER_ACTOR, *substitute_hp_ptr,
		DATABASE_LOG_OUTPUT);
      if(*substitute_hp_ptr == 0)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***substitute broke\n");
#endif
	  log_event(SUBSTITUTE_BROKE, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      return DEFENDER_SUB_BROKE;
    }
  /* there is no substitute, apply directly */
  else
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***applying directly\n");
#endif
      *damage =  apply_damage(*damage, current_hp_ptr);
      log_event(HP_REMAINING, DEFENDER_ACTOR, *current_hp_ptr,
		DATABASE_LOG_OUTPUT);

      /* if defender has fainted */
      if(*current_hp_ptr == 0)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***defender faints due to inflicted damage\n");
#endif
	  return DEFENDER_FAINTS;
	}
       return NO_EVENTS;
    }
}

int status_is_sleep(int status)
{
  return status > 0 && status <= SLP;
}

int report_sleeping(unsigned int status, unsigned int infliction_mechanism,
		    unsigned int enforced_clauses, unsigned int * sleep_count)
{
  if(status_is_sleep(status))
    {
      if(status == 1)
	{
	  log_event(WOKE_UP, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	  if((enforced_clauses & SLEEP_CLAUSE) &&
	     infliction_mechanism == OPPONENT)
	    {
	      (*sleep_count)--;
	    }
	}
      else
	{
	  log_event(IS_ASLEEP, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

int report_frozen(unsigned int status)
{
  if(status == FRZ)
    {
      log_event(IS_FROZEN, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

int report_recharge(unsigned int * recharging)
{
  if(*recharging)
    {
      /* no longer recharging */
      *recharging = 0;
      log_event(MUST_RECHARGE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

int report_immobilized(unsigned int trapping_turns, unsigned int trapping_move)
{
  if(trapping_turns > 0)
    {
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

void report_biding(unsigned int * biding_turns)
{
  if(*biding_turns > 0)
    {
      (*biding_turns)--;
    }
}

int report_unleashed_energy(unsigned int biding_turns, int bide_damage,
			    int * current_hp_ptr, int * damage_done)
{
  if(biding_turns == 1)
    {
      /* set the damage to be the amount of damage done by bide */
      bide_damage *= 2;
      *damage_done = bide_damage;

      log_event(UNLEASHED_ENERGY, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      if(bide_damage == 0)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

	  return NO_EVENTS;
	}
      else
	{
	  return inflict_damage(&bide_damage, NULL, current_hp_ptr);
	}
    }
  return NO_EVENTS;
}

void report_thrashing_about(unsigned int * thrashing_turns)
{
  if(*thrashing_turns > 0)
    {
      if(*thrashing_turns > 1)
	{
	  log_event(THRASHING_ABOUT, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      (*thrashing_turns)--;
    }
}

void report_thrashing_confusion(unsigned int thrashing_turns,
				unsigned int * confusion_turns)
{
  /* made it to last turn of thrash */
  if(thrashing_turns == 1)
    {
      /* apply confusion to self */
      apply_self_confusion(confusion_turns);
    }
}

void report_disabled_end(unsigned int * disabled_turns)
{

  if(*disabled_turns > 0)
    {
      (*disabled_turns)--;
      if(*disabled_turns == 0)
	{
	  log_event(NO_LONGER_DISABLED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
}

void report_confusion_status(unsigned int * confusion_turns)
{
  if(*confusion_turns > 0)
    {
      (*confusion_turns)--;
      if(*confusion_turns == 0)
	{
	  log_event(NO_LONGER_CONFUSED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  log_event(IS_CONFUSED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
}

int report_move_disabled(unsigned int index, unsigned int move,
			 unsigned int disabled_index)
{
  if(index == disabled_index)
    {
      log_event(MOVE_DISABLED, ATTACKER_ACTOR, move, CLIENT_LOG_OUTPUT
		| DATABASE_LOG_OUTPUT);
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

int report_full_paralysis(unsigned int status, unsigned int * thrashing_turns,
			  unsigned int * charging_move,
			  unsigned int * temp_invulnerable)
{
  if(status == PAR && genrand(4, 0) == 0)
    {
      log_event(IS_FULLY_PARALYZED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

      /* stop thrashing */
      *thrashing_turns = 0;
      /* stop 2 turn move */
      *charging_move = NONE;
      /* fall from the sky or rise from the ground */
      *temp_invulnerable = 0;
      return ATTACKER_PREVENTED_FROM_ATTACKING;
    }
  return NO_EVENTS;
}

int report_confusion_damage(unsigned int confusion_turns, int * current_hp_ptr,
			    int level, int att_stat, int def_stat,
			    unsigned int * thrashing_turns,
			    unsigned int * charging_move,
			    unsigned int * temp_invulnerable)
{
  unsigned int confusion_damage;
  if(confusion_turns > 0 && genrand(4 ,0) == 0)
    {
      log_event(HURT_BY_CONFUSION, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      confusion_damage = confusion_damage_calc(level, att_stat, def_stat);
      apply_damage(confusion_damage, current_hp_ptr);

      log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp_ptr,
		DATABASE_LOG_OUTPUT);

      if(*current_hp_ptr > 0)
	{
	  /* stop thrashing on confusion */
	  *thrashing_turns = 0;
	  /* stop our two-turn move */
	  *charging_move = NONE;
	  /* fall from the sky or rise from the ground on confusion */
	  *temp_invulnerable = 0;
	  return ATTACKER_PREVENTED_FROM_ATTACKING;
	}
      else
	{
	  return ATTACKER_PREVENTED_FROM_ATTACKING | ATTACKER_FAINTS;
	}
    }
  return NO_EVENTS;
}

int choose_number_iterations(void)
{
  int rand_num = genrand(8,0);
  if(rand_num < 3)
    {
      return 2;
    }
  else if(rand_num < 6)
    {
      return 3;
    }
  else if(rand_num < 7)
    {
      return 4;
    }
  else
    {
      return 5;
    }
}


int report_attack_primary( int move_power, int move_type, int move_parameter,
			   int * attacker_type, int attacker_number,
			   int defender_number, int attacker_level,
			   int attacker_stat, int original_attacker_stat,
			   int * defender_type, int defender_stat,
			   int original_defender_stat, int critical_hit_up,
			   int counterable_damage, int * substitute_hp_ptr,
			   int * current_hp_ptr, int max_hp,
			   int * damage_done)
{
   int use_simple_damage = 0;
   int critical_hit = 0;
   int critical_modifier = 1;
   int damage;
   int power;
   int iterations;
   int base_speed;

   float mod = 1;
   float stab = 1;

   int i;
   unsigned int events = 0;
  if(move_power != 0)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***move is damaging, power: %i\n", move_power);
#endif
      /* negative powers have special meanings, set the power to the proper
	 value */
      if(move_power > 0)
	{
	  power = move_power;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***passed through to power value\n");
#endif
	}
      else
	{
	  switch(move_power)
	    {
	    case OHKO:
	      power = *current_hp_ptr;
	      break;
	    case HALF:
	      power = *current_hp_ptr / 2;
	      break;
	    case PW_VARIABLE:
	      power = genrand((attacker_level * 1.5) + 1, 1);
	      break;
	    case ATTACKER_LEVEL:
	      power = attacker_level;
	      break;
	    case DOUBLE_COUNTERABLE:
	      power = counterable_damage * 2;
	      break;
	    }
	  /* these special damages do not go into the battle formula,
	     so set simple damage to 1 */
	  use_simple_damage = 1;
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***correted power: %i\n", power);
#endif
	}

      if(move_parameter == SIMPLE_DAMAGE)
	{
	  use_simple_damage = 1;
	}
      /* move uses the normal battle formula */
      if(!use_simple_damage)
	{
	  /* find out of we have a critical hit: we generate a random number
	     0-255 we generate another number with an upper bound of 255. This
	     number is based off the base speed modified by the value calculated
	     below */

	  /* if the move has a high critical hit chance increase the chance by
	     8 */
	  if(move_parameter == HIGH_CRITICAL_HIT)
	    {
	      critical_modifier = 8;
	    }
	  /* otherwise, check focus energy has been used */
	  else if(critical_hit_up)
	    {
	      critical_modifier = 4;
	    }
	  /* otherwise we leave it at 1 */

	  /* now we do the aforementioned test */
	  if(get_base_speed(attacker_number, &base_speed) == -1)
	    {
	      fprintf(stderr, "Could not pull base speed from database");
	      exit(3);
	    }

	  if((critical_hit = genrand(256, 0) <
	      min(critical_modifier * base_speed/2, 255)) != 0)
	    {
#if(DEBUG_ENGINE)
	      fprintf(stderr, "***critical hit\n");
#endif
	      attacker_level *= 2;
	      attacker_stat = original_attacker_stat;
	      defender_stat = original_defender_stat;
	    }
	  /* calculate the type modifier based on
	     the move's type and the defender's types */

	  mod = type_chart[move_type][defender_type[0]]
	    * type_chart[move_type][defender_type[1]];
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***defender types: %i %i\n",
		  defender_type[0], defender_type[1]);
	  fprintf(stderr, "***modifier is %f*%f=%f\n",
		  type_chart[move_type][defender_type[0]],
		  type_chart[move_type][defender_type[1]], mod);
#endif

	  /* calculate same-type-attack-bonus */
	  stab = (move_type == attacker_type[0] ||
		  move_type == attacker_type[1])? 1.5:1;
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***STAB is %f\n", stab);
#endif
	  damage = std_damage_calc(attacker_level, attacker_stat, power,
				   defender_stat, stab, mod,
				   genrand(RANDOM_RANGE, RANDOM_BASE));
	}
      else
	{
	  damage = power;
	}
#if(DEBUG_ENGINE)
      fprintf(stderr, "***damage is %i\n", damage);
#endif
      if(damage == 0)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***indicating no damage\n");
#endif
	  /* If based on type, the move didn't effect the defender. */
	  if(mod == 0)
	    {
	      log_event(NO_EFFECT, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  /* Otherwise (just really low damage), we report the move as having
	     missed */
	  else
	    {
	      log_event(MOVE_MISSED, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  return ATTACKER_MOVE_DOES_NO_DAMAGE |
	    ((move_parameter != SECONDARY_WITHOUT_HIT)?
	     ATTACKER_PREVENTED_FROM_SECONDARY : NO_EVENTS);
	}
      else if(!use_simple_damage)
	{
	  /* show messages only if damage was done and
	   the move takes into acount these factors */
	  /* report the mod, super or not very effective */
	  if(mod > 0 && mod < 1)
	    {
	      log_event(NOT_VERY_EFFECTIVE, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else if(mod > 1)
	    {
	      log_event(SUPER_EFFECTIVE, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }

	  if(critical_hit)
	    {
	      log_event(CRITICAL_HIT, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}

      if(move_parameter == REPEAT_2_TIMES)
	{
	  iterations = 2;
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***repeating twice\n");
#endif
	  /* log number of times hit */
	  log_event(HIT_N_TIMES, ATTACKER_ACTOR, 2,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

	}
      if(move_parameter == REPEAT_2_5_TIMES)
	{
	  iterations = choose_number_iterations();
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***repeating %i times\n", iterations);
#endif
	  log_event(HIT_N_TIMES, ATTACKER_ACTOR, iterations,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  iterations = 1;
	}

      /* damage reported is always the last iteration of a multi-iteration move
       * and no more than the hp left (which might have been already reduced
       * by other iterations) */
      for(i = 0; i < iterations; i++)
	{
	  events = inflict_damage(&damage, substitute_hp_ptr, current_hp_ptr);
	  *damage_done = damage;
	  if(events & (DEFENDER_FAINTS | DEFENDER_SUB_BROKE))
	    {
	      break;
	    }
	}
      return events;
    }
  else
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***attack is non-damaging\n");
#endif
      if(move_parameter != SELF_TARGETING)
	{
	  mod = type_chart[move_type][defender_type[0]]
	    * type_chart[move_type][defender_type[1]];
	  /* if the move should not be effective on account of type */
	  if(mod == 0)
	    {
	      log_event(NO_EFFECT, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      return ATTACKER_PREVENTED_FROM_SECONDARY;
	    }
	}
    }
  return NO_EVENTS;
}

void report_building_rage(unsigned int enraged, unsigned int damage_taken,
			  int * attack_mod)
{
  if(enraged && damage_taken > 0)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***building rage (%i damage)\n",
	      damage_taken);
#endif
      log_event(RAGE_BUILDING, DEFENDER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      if(*attack_mod < MAX_STAT_MOD)
	{
	  log_event(ATTACK_ROSE, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	  (*attack_mod)++;
	}
    }
}

void report_thaw(unsigned int move_type, unsigned int * status,
		 unsigned int enforced_clauses, unsigned int * frozen_count)
{
  if(*status == FRZ && move_type == FIRE)
    {
      log_event(DEFENDER_ACTOR, THAWED, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
      *status = NRM;
      if(enforced_clauses & FREEZE_CLAUSE)
	{
	  (*frozen_count)--;
	}
    }
}

int report_poison_burn_damage(unsigned int status,
			      unsigned int * toxic_turns_ptr,
			      int * current_hp_ptr,
			      int max_hp)
{
  unsigned int damage;
  if(status == BRN || status == PSN)
    {
      if(status == BRN)
	{
	  log_event(HURT_BY_BURN, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      if(status == PSN)
	{
	  log_event(HURT_BY_POISON, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      damage = max((((*toxic_turns_ptr == 0)? 1:*toxic_turns_ptr)
		    * max_hp)/16, 1);
#if(DEBUG_ENGINE)
      fprintf(stderr, "***poison/burn at level %i, doing %i damage\n",
	      *toxic_turns_ptr, damage);
#endif
      if(*toxic_turns_ptr > 0)
	{
	  (*toxic_turns_ptr)++;
	}
      apply_damage(damage, current_hp_ptr);
      log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp_ptr,
		DATABASE_LOG_OUTPUT);
      if(*current_hp_ptr > 0)
	{
	  return NO_EVENTS;
	}
      else
	{
	  return ATTACKER_FAINTS;
	}
    }
  return NO_EVENTS;
}

int report_leech_damage(unsigned int leeched, unsigned int * toxic_turns_ptr,
			int * attacker_hp_ptr, int attacker_max_hp,
			int * defender_hp_ptr, int defender_max_hp)
{
  unsigned int leech_amount;
  if(leeched)
    {
      log_event(HURT_BY_LEECH_SEED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      leech_amount = max((((*toxic_turns_ptr == 0)? 1:*toxic_turns_ptr)
			  * attacker_max_hp)/16, 1);
      /* implement the 'toxic seed' behavior */
      if(*toxic_turns_ptr > 0)
	{
	  (*toxic_turns_ptr)++;
	}

      /* true fact: the leech amount healed can be greater than the actual
	 damage done, so don't update leech amount */
      apply_damage(leech_amount, attacker_hp_ptr);

      log_event(HP_REMAINING, ATTACKER_ACTOR, *attacker_hp_ptr,
		DATABASE_LOG_OUTPUT);

      *defender_hp_ptr = heal(leech_amount, *defender_hp_ptr,
			      defender_max_hp);

      log_event(HP_REMAINING, DEFENDER_ACTOR, *defender_hp_ptr,
		DATABASE_LOG_OUTPUT);

      if(*attacker_hp_ptr > 0)
	{
	  return NO_EVENTS;
	}
      else
	{
	  return ATTACKER_FAINTS;
	}
    }
  return NO_EVENTS;
}

int report_trapping_damage(unsigned int trapping_turns,
			   unsigned int trapping_move, int trapping_damage,
			   int * current_hp_ptr, int * damage)
{
  unsigned int ret_val = NO_EVENTS;
#if(DEBUG_ENGINE)
  fprintf(stderr, "***report_trapping_damage sees %i trapping turns\n",
	  trapping_turns);
#endif
  if(trapping_turns > 0)
    {
      log_event(ATTACK_CONTINUES, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      if(trapping_damage > 0)
	{
	  ret_val =  inflict_damage(&trapping_damage, NULL, current_hp_ptr);
	  *damage = trapping_damage;
	}
    }
  return ret_val;
}

/**
 * Select the move for the pkmn to use this round.
 * @param[in,out] move_index Pointer to the index into the of the pkmn's move.
 * Will be changed if a move other than the intended move must to selected, such
 * as by a lack of PP. (0-3).
 * @param[in] num_moves The total number of legal moves in the moveset. (0-3).
 * @param[in] Move_list The array of move structures holding the pkmn's moveset
 * @param[in] trapping_move The current trapping move that the pkmn is using,
 * or NONE if it is not currently using a trapping move.
 * @param[in] trapping_turns The amount of turns left of the trapping move
 * @param[in] original_trapping_victim Is the opponent's pkmn the original
 * victim of the trapping move? (0-1).
 * @param[in] thrashing_move The move associated with the pkmn thrashing about,
 * or NONE if this opponent is not thrashing about.
 * @param[in] thrashing_turns The number of turns remaining for the pkmn to
 * thrash about.
 * @param[in] enraged Is the current pkmn enraged? (0-1)
 * @param[in] biding_turns The number of turns remaining for the pkmn to bide.
 * @param[in] opponents_last_move The last move used by the opponent.
 * @param[out] Event * event_buffer An array of events to store the Events
 * generated from move selection.
 * @param[in,out] event_counter Counter into event_buffer.
 **/
int select_move(int * move, unsigned int * move_index, unsigned int num_moves,
		Move * move_list, unsigned int charging_move,
		unsigned int trapping_move, unsigned int trapping_turns,
		unsigned int original_trapping_victim,
		unsigned int repeated_move_index, unsigned int thrashing_move,
		unsigned int thrashing_turns, unsigned int enraged,
		unsigned int biding_turns)
{
  int i;
  int valid_move_found = 0;

  if(charging_move != NONE)
    {
      *move = charging_move;
      *move_index = repeated_move_index;
      return NO_EVENTS;
    }
  /* if there's a trapping move, execute it */
  if(trapping_turns > 0)
    {
      /* We handle trapping move pp in somewhat of an unusual way:
	 Stadium allows a new move to be selected on an opponent switch during
	 a trapping move. We reject this because it defeats the ability to pp
	 waste the trapping move through repeated switching.
	 The gameboy games unconditionally use the trapping move again. We
	 reject this because it allows the pp of the move to underflow.

	 So what we do is if the opponent switches, we force the re-use of the
	 trapping move unless there is no pp for it. If there is no pp then the
	 user may use a new move */

      if((!original_trapping_victim &&
	  move_list[repeated_move_index].current_pp > 0) ||
	 original_trapping_victim)
	{
	  *move = trapping_move;
	  *move_index = repeated_move_index;
	  return NO_EVENTS;
	}
    }
  /* if biding, do not attack */
  if(biding_turns > 0)
    {
      *move = NONE;
      return SUPRESS_PP_USE;
    }

  /* if there's a thrash-like move in progress, execute it */
  if(thrashing_turns > 0)
    {
      *move = thrashing_move;
      return NO_EVENTS;
    }
  /* if the pkmn is enraged, use rage */
  if(enraged)
    {
      *move = RAGE;
      /* Rage does not use subsequent PP */
      return SUPRESS_PP_USE;
    }

  /* We got through the initial checks that would force us to to use a move
   * other than the one that we selected */
  /* We have proposed using a move wih no pp */
  if(move_list[*move_index].current_pp == 0)
    {
      /* look through the moves that we have available */
      for(i = 0; i < num_moves; i++)
	{
	  /* we have found a valid move: with pp and not disabled
	     (?) why was this check removed? */
	  if(move_list[i].current_pp > 0)
	    {
	      *move = move_list[i].number;
	      /* break out of the loop indicating success
		 and procede with the attack as usual */
	      valid_move_found = 1;
	      /* Update the index in case we need to overwrite it (Mimic) */
	      *move_index = i;
	      break;
	    }
	}

      /* we found no valid moves, so just return struggle */
      if(!valid_move_found)
	{
	  *move = STRUGGLE;
	  return NO_EVENTS;
	}
    }
  /* our move selection was fine, pass it back */
  else
    {
      *move =  move_list[*move_index].number;
    }
  return NO_EVENTS;
}


int standard_move_hits(unsigned int accuracy, unsigned int parameter,
		       unsigned int attacker_accuracy_mod,
		       unsigned int defender_evade_mod,
		       unsigned int temp_invulnerable)
{
  int rand;
  int hits;
  int acc;
  int evade_mod;
#if(DEBUG_ENGINE)
  fprintf(stderr, "***base accuracy: %i\n", accuracy);
  fprintf(stderr, "***attacker accuracy mod: %i (%f)\n", attacker_accuracy_mod,
	  hit_mods_val[attacker_accuracy_mod]);
  fprintf(stderr, "***defender evade  mod: %i (%f)\n", defender_evade_mod,
	  hit_mods_val[defender_evade_mod]);
  fprintf(stderr, "***ignoring these?: %i\n", parameter == IGNORE_EVD_ACC);
#endif
  rand = genrand(256, 0);
  evade_mod = (!temp_invulnerable) ? defender_evade_mod : 7;
  acc = accuracy * ((parameter != IGNORE_EVD_ACC) ?
		    (hit_mods_val[attacker_accuracy_mod] *
		     (1.0 - hit_mods_val[evade_mod])) : 1);
  hits = rand < acc;
#if(DEBUG_ENGINE)
    fprintf(stderr, "***final accuracy: %i, random is %i\n***hit? %i\n", acc,
	    rand, hits);
#endif
    return hits;
}

int secondary_hits(unsigned int hit_chance)
{
  return genrand(256, 0) < hit_chance;
}


int report_move_miss(unsigned int move_accuracy, unsigned int move_parameter,
		     unsigned int accuracy_mod, unsigned int evade_mod,
		     unsigned int temp_invulnerable,
		     unsigned int defender_status,
		     unsigned int attacker_charging, unsigned int is_first)
{
  /* Return cases that always hit */
  if(move_accuracy == 256)
    {
      return NO_EVENTS;
    }
  /* Return cases that always miss */
  if(move_parameter == SLP_NEEDED &&
     (defender_status < 1 || defender_status > SLP))
    {
      log_event(MOVE_MISSED, ATTACKER_ACTOR, 0,	CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);

      return ATTACKER_MOVE_MISSES | ATTACKER_PREVENTED_FROM_SECONDARY;
    }
  if(move_parameter == ONLY_IF_FIRST && !is_first)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);

      return ATTACKER_MOVE_MISSES | ATTACKER_PREVENTED_FROM_SECONDARY;
    }
  /* otherwise compute accuracy normally */
  if(!standard_move_hits(move_accuracy, move_parameter, accuracy_mod, evade_mod,
			 temp_invulnerable))
    {
      log_event(MOVE_MISSED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);

      return ATTACKER_MOVE_MISSES |
	((move_parameter != SECONDARY_WITHOUT_HIT &&
	  move_parameter != SECONDARY_ON_MISS)?
	 ATTACKER_PREVENTED_FROM_SECONDARY : NO_EVENTS);
    }
    else /* move hits */
    {
      return ((move_parameter != SECONDARY_ON_MISS)? NO_EVENTS :
	      ATTACKER_PREVENTED_FROM_SECONDARY);
    }
}

int apply_suicide(int * current_hp)
{
  *current_hp = 0;
  return ATTACKER_FAINTS;
}

int apply_quarter_recoil(int * current_hp, unsigned int damage_done)
{
  log_event(HIT_BY_RECOIL, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);
  apply_damage(damage_done / 4, current_hp);
  log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp, DATABASE_LOG_OUTPUT);
  if(*current_hp == 0)
    {
      return ATTACKER_FAINTS;
    }
  return NO_EVENTS;
}

int apply_one_damage_recoil(int * current_hp)
{
  log_event(HIT_BY_RECOIL, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);

  apply_damage(1, current_hp);

  log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp, DATABASE_LOG_OUTPUT);
  if(*current_hp == 0)
    {
      return ATTACKER_FAINTS;
    }
  return NO_EVENTS;
}

void apply_recharge(unsigned int * recharging)
{
  *recharging = 1;
}

void apply_leech(int * current_hp, int max_hp, unsigned int damage_done)
{
  log_event(SUCKED_HP, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);

  *current_hp = heal(damage_done / 4, *current_hp, max_hp);
  log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp, DATABASE_LOG_OUTPUT);
}

void apply_attack_up(int * mod, unsigned int level)
{
  if(*mod == MAX_STAT_MOD)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
    }
  else
    {
      *mod += level;

      if(*mod > MAX_STAT_MOD)
	{
	  *mod = MAX_STAT_MOD - 1;
	  log_event(ATTACK_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(ATTACK_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(ATTACK_GREATLY_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_defense_up(int * mod, unsigned int level)
{
  if(*mod == MAX_STAT_MOD)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***increase def beyond %i failed\n", *mod);
#endif
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      *mod += level;

      if(*mod > MAX_STAT_MOD)
	{
	  *mod = MAX_STAT_MOD - 1;
	  log_event(DEFENSE_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(DEFENSE_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(DEFENSE_GREATLY_ROSE, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
#if(DEBUG_ENGINE)
      fprintf(stderr, "***def is now %i\n", *mod);
#endif
    }
}

void apply_speed_up(int * mod, unsigned int level)
{
  if(*mod == MAX_STAT_MOD)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      *mod += level;

      if(*mod > MAX_STAT_MOD)
	{
	  *mod = MAX_STAT_MOD - 1;
	  log_event(SPEED_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(SPEED_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(SPEED_GREATLY_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_special_up(int * mod, unsigned int level)
{
  if(*mod == MAX_STAT_MOD)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
    }
  else
    {
      *mod += level;

      if(*mod > MAX_STAT_MOD)
	{
	  *mod = MAX_STAT_MOD - 1;
	  log_event(SPECIAL_ROSE, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(SPECIAL_ROSE, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(SPECIAL_GREATLY_ROSE, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_evade_up(int * mod)
{
  if(*mod < MAX_HIT_MOD)
    {
      log_event(EVADE_ROSE, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
      (*mod)++;
    }
  else
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
    }
}

void apply_critical_hit_up(unsigned int * critical_hit_up)
{
  if(*critical_hit_up == 1)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(GETTING_PUMPED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      *critical_hit_up = 1;
    }
}


void apply_attack_down(unsigned int chance, int * mod,
		       unsigned int level, unsigned int substitute,
		       int sub_broke, unsigned int mod_lock)
{
  if(*mod == MIN_STAT_MOD || substitute || sub_broke || mod_lock ||
     !secondary_hits(chance))
    {
      /* this is the only effect of the move announce failure */
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
  else /* secondary succeeds */
    {
      *mod -= level;

      if(*mod < MIN_STAT_MOD)
	{
	  *mod = MIN_STAT_MOD;
	  log_event(ATTACK_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(ATTACK_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(ATTACK_GREATLY_FELL, DEFENDER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_defense_down(unsigned int chance, int * mod,
			unsigned int level, unsigned int substitute,
			int sub_broke, unsigned int mod_lock)
{
  if(*mod == MIN_STAT_MOD || substitute || sub_broke || mod_lock ||
     !secondary_hits(chance))
    {
      /* this is the only effect of the move announce failure */
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
  else /* secondary succeeds */
    {
      *mod -= level;

      if(*mod < MIN_STAT_MOD)
	{
	  *mod = MIN_STAT_MOD;
	  log_event(DEFENSE_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(DEFENSE_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(DEFENSE_GREATLY_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_speed_down(unsigned int chance, int * mod,
		      unsigned int level, unsigned int substitute,
		      int sub_broke, unsigned int mod_lock)
{
  if(*mod == MIN_STAT_MOD || substitute || sub_broke || mod_lock ||
     !secondary_hits(chance))
    {
      /* this is the only effect of the move announce failure */
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
  else /* secondary succeeds */
    {
      *mod -= level;

      if(*mod < MIN_STAT_MOD)
	{
	  *mod = MIN_STAT_MOD;
	  log_event(SPEED_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(SPEED_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(SPEED_GREATLY_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}


void apply_special_down(unsigned int chance, int * mod,
			unsigned int level, unsigned int substitute,
			int sub_broke, unsigned int mod_lock)
{
  if(*mod == MIN_STAT_MOD || substitute || sub_broke || mod_lock ||
     !secondary_hits(chance))
    {
      /* this is the only effect of the move announce failure */
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
  else
    {
      *mod -= level;

      if(*mod < MIN_STAT_MOD)
	{
	  *mod = MIN_STAT_MOD;
	  log_event(SPECIAL_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else
	{
	  if(level == NORMALLY)
	    {
	      log_event(SPECIAL_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  else
	    {
	      log_event(SPECIAL_GREATLY_FELL, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
    }
}

void apply_accuracy_down(int * mod, unsigned int substitute, int sub_broke,
			 unsigned int mod_lock)
{
  if(*mod == MIN_HIT_MOD || substitute || sub_broke || mod_lock)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(ACCURACY_FELL, DEFENDER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      (*mod)--;
    }
}

int apply_status(unsigned int new_status, unsigned int * status,
		 int substitute, int sub_broke, int * type, int chance,
		 unsigned int * infliction_mechanism,
		 unsigned int enforced_clauses, unsigned int *sleep_count,
		 unsigned int *frozen_count)
{
  if((*status != NRM) || substitute || sub_broke ||
     (new_status == PSN && (type[0] == POISON  || type[1] == POISON))||
     (new_status == BRN && (type[0] == FIRE  || type[1] == FIRE))||
     (new_status == FRZ && (type[0] == ICE  || type[1] == ICE))||
     (status_is_sleep(new_status) && (*sleep_count > 0)) ||
     (new_status == FRZ && (*frozen_count > 0)) || !secondary_hits(chance))
    {
      /* if we're dealing with a move that has the sole purpose
	 of applying a status, our chance of the effect
	 100% (256) and modulate chance with the accuracy.
	 if such a move fails to apply the status, we want to
	 acknowlege it.  in any other case, the status is a side
	 effect and it would look really silly to display a status
	 message for an ancillary effect when the move itself succeeded */
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      return -1;
    }
  else
    {
      *status = new_status;
      if(status_is_sleep(new_status))
	{
	  log_event(FELL_ASLEEP, DEFENDER_ACTOR, new_status,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	  *infliction_mechanism = OPPONENT;
	  if(enforced_clauses & SLEEP_CLAUSE)
	    {
	      (*sleep_count)++;
	    }
	}
      else if(new_status == PAR)
	{
	  log_event(WAS_PARALYZED, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
      else if(new_status == BRN)
	{
	  log_event(WAS_BURNED, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT );
	}
      else if(new_status == PSN)
	{
	  /* defer the message until we return, at this point
	     we don't know whether we are poising or badly poisoning */
	}
      else if(new_status == FRZ)
	{
	  log_event(WAS_FROZEN, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	  if(enforced_clauses & FREEZE_CLAUSE)
	    {
	      (*frozen_count)++;
	    }
	}
      else
	{
	  fprintf(stderr, "@@@whoa: weird status %i\n", new_status);
	  exit(3);
	}
      return 0;
    }
}

void apply_confusion(unsigned int * confusion_turns, unsigned int substitute,
		     int sub_broke, unsigned int chance)
{
  /* if the opponent has a substitute up or is already confused and the sole
     purpose of the the move is to confuse, then display a failure message (if
     the confusion is a side effect it would not be correct to see a failure
     message) */
  if(substitute || sub_broke || confusion_turns == 0 || !secondary_hits(chance))
    {
      if(chance == 256)
	{
	  log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	}
    }
  else
    {
      /* generate between 1 and 7 turns of confusion */
      *confusion_turns = genrand(7, 1);
      log_event(WAS_CONFUSED, DEFENDER_ACTOR, *confusion_turns,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

    }
}

void apply_self_confusion(unsigned int * confusion_turns)
{
   /* generate between 1 and 7 turns of confusion */

  *confusion_turns = genrand(7, 1);
  log_event(WAS_CONFUSED, ATTACKER_ACTOR, *confusion_turns,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

}

void apply_seed(unsigned int * seeded, int * type, int substitute,
		int sub_broke)
{
  if((type[0] == GRASS || type[1] == GRASS) || substitute || sub_broke ||
     *seeded)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(WAS_SEEDED, DEFENDER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      *seeded = 1;
    }
}

void apply_special_halve(unsigned int * special_halve)
{
  if(*special_halve)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(PROTECTED_SPECIAL, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      *special_halve = 1;
    }
}

void apply_mod_lock(unsigned int * mod_lock)
{
  if(*mod_lock)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(SHROUDED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      *mod_lock = 1;
    }
}

void apply_copy_pkmn(unsigned int * attacker_number,
		     unsigned int  defender_number,
		     Stats * attacker_stats, Stats * defender_stats,
		     Stat_mods * attacker_stat_mods,
		     Stat_mods * defender_stat_mods,
		     int * attacker_evade_mod, int defender_evade_mod,
		     int * attacker_accuracy_mod, int defender_accuracy_mod,
		     int * attacker_type, int * defender_type,
		     Move * backing_move, unsigned int num_backing_moves,
		     Move * current_move, unsigned int * num_current_moves,
		     Move * defender_move, unsigned int num_defender_moves)
{
  int i;

  *attacker_number = defender_number;
  memcpy(attacker_stats, defender_stats, sizeof(Stats));
  memcpy(attacker_stat_mods, defender_stat_mods, sizeof(Stat_mods));
  memcpy(attacker_type, defender_type, 2 * sizeof(*attacker_type));
  *attacker_evade_mod = defender_evade_mod;
  *attacker_accuracy_mod = defender_accuracy_mod;
  for(i = 0; i < num_backing_moves; i++)
    {
      backing_move[i].update_flag = LEAVE_AS_IS;
    }
  for(i = 0; i < num_defender_moves; i++)
    {
      current_move[i].number = defender_move[i].number;
      current_move[i].current_pp = 5;
    }
  *num_current_moves = num_defender_moves;
  log_event(TRANSFORMED_INTO, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);
}

void apply_convert_type(int * attacker_type, int * defender_type)
{
  /* this will probably get broken somehow */
  memcpy(attacker_type, defender_type, sizeof(unsigned int) * 2);

  log_event(CONVERTED_TYPE, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);
}

void apply_copy_move(unsigned int selection, Move * attacker_current_move,
		     Move * attacker_backing_move, Move * defender_move,
		     unsigned int defender_move_num)
{
  int rand_slot = genrand(defender_move_num, 0);
  /* setting this flag prevents us from making the copied move permanant on a
     switch */
  attacker_backing_move[selection].update_flag = LEAVE_AS_IS;
  attacker_current_move[selection].number = defender_move[rand_slot].number;

  log_event(LEARNED_MOVE, ATTACKER_ACTOR,
	    attacker_current_move[selection].number,
	    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
}

void apply_heal_half(int * current_hp, int max_hp)
{
  /* totally not doing the weird failures on 256 and 512, sorry purists */
  if(*current_hp != max_hp)
    {
      *current_hp = heal(max_hp / 2, *current_hp, max_hp);
      log_event(REGAINED_HEALTH, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

      log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp,
		DATABASE_LOG_OUTPUT);

    }
  else
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
}

void apply_sleep_and_heal(int *current_hp, int max_hp, unsigned int * status,
			  unsigned int * infliction_mechanism)
{
  if(*current_hp != max_hp)
    {
      *current_hp = heal(max_hp, *current_hp, max_hp);
      *status = REST_TURNS;
      *infliction_mechanism = SELF;
      log_event(STARTED_SLEEPING, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);

      log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp, DATABASE_LOG_OUTPUT);
    }
  else
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
}

void apply_make_sub(int * current_hp, int max_hp, int * substitute)
{
  if(*substitute || *current_hp <= max_hp / 4)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      *current_hp -= max_hp / 4;

      log_event(HP_REMAINING, ATTACKER_ACTOR, *current_hp, DATABASE_LOG_OUTPUT);

      *substitute = max_hp/4;
      log_event(CREATED_SUBSTITUTE, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
}

void apply_physical_halve(unsigned int * physical_halve)
{
  if(*physical_halve)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      *physical_halve = 1;
      log_event(GAINED_ARMOR, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
}

void apply_flinch(unsigned int * flinched, unsigned int status,
		  unsigned int substitute, int sub_broke, unsigned int is_first,
		  unsigned int chance)
{
#if(DEBUG_ENGINE)
  fprintf(stderr, "***checking to apply flinch\n");
#endif
  if(secondary_hits(chance) && !substitute && !sub_broke && status != SLP &&
     status != FRZ
     && is_first)
    {
      *flinched = 1;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***did flinch\n");
#endif
    }
}

void apply_disable_move(unsigned int * disabled_selection,
			unsigned int * disable_turns,
			Move * defender_move, unsigned int num_moves)
{
  int selection;
  if(*disabled_selection != NONE_DISABLED)
    {
      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else
    {
      selection = genrand(num_moves, 0);
      while(defender_move[selection].current_pp == 0)
	{
	  selection = genrand(num_moves, 0);
	}
      *disabled_selection = selection;
      *disable_turns = genrand(8, 1);

      log_event(DISABLED_MOVE, DEFENDER_ACTOR, defender_move[selection].number,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
}

void apply_eliminate_changes(Stat_mods * attacker_mods,
			     Stat_mods * defender_mods,
			     int * attacker_evade_mod, int * defender_evade_mod,
			     int * attacker_accuracy_mod,
			     int * defender_accuracy_mod,
			     unsigned int * attacker_physical_halve,
			     unsigned int * defender_physical_halve,
			     unsigned int * attacker_special_halve,
			     unsigned int * defender_special_halve,
			     unsigned int * attacker_critical_hit_up,
			     unsigned int * defender_critical_hit_up,
			     unsigned int * attacker_mod_lock,
			     unsigned int * defender_mod_lock,
			     unsigned int * attacker_confusion_turns,
			     unsigned int * defender_confusion_turns,
			     unsigned int * attacker_seeded,
			     unsigned int * defender_seeded,
			     unsigned int * attacker_toxic_turns,
			     unsigned int * defender_toxic_turns,
			     unsigned int * attacker_status,
			     unsigned int * defender_status,
			     unsigned int attacker_infliction_mechanism,
			     unsigned int defender_infliction_mechanism,
			     unsigned int enforced_clauses,
			     unsigned int * attacker_sleep_count,
			     unsigned int * defender_sleep_count,
			     unsigned int * attacker_frozen_count,
			     unsigned int * defender_frozen_count)
{
  attacker_mods->attack = attacker_mods->defense =
    attacker_mods->speed = attacker_mods->special =
    defender_mods->attack = defender_mods->defense =
    defender_mods->speed = defender_mods->special = BASE_STAT_MOD;

  *attacker_evade_mod = *defender_evade_mod = BASE_EVADE_MOD;

  *attacker_accuracy_mod = * defender_accuracy_mod = BASE_ACCURACY_MOD;

  *defender_physical_halve =  *attacker_special_halve =
    *attacker_special_halve = *defender_special_halve =
    *attacker_critical_hit_up = *defender_critical_hit_up =
    *attacker_mod_lock = *defender_mod_lock =
    *attacker_confusion_turns = *defender_confusion_turns =
    *attacker_seeded =  *defender_seeded =
    *attacker_toxic_turns = * defender_toxic_turns = 0;

  if(*defender_status == SLP && defender_infliction_mechanism == OPPONENT &&
     (enforced_clauses & SLEEP_CLAUSE))
    {
      (*defender_sleep_count)--;
    }
  else if(*defender_status == FRZ && (enforced_clauses & FREEZE_CLAUSE))
    {
      (*defender_frozen_count)--;
    }
  *defender_status = NRM;
#if (HAZE_CLEARS_ATTACKER_STATUS)
  if(*attacker_status == SLP && attacker_infliction_mechanism == OPPONENT &&
     (enforced_clauses & SLEEP_CLAUSE))
    {
      (*attacker_sleep_count)--;
    }
  else if(*attacker_status == FRZ && (enforced_clauses & FREEZE_CLAUSE))
    {
      (*attacker_frozen_count)--;
    }
  *attacker_status = NRM;
#endif
  log_event(STATUS_CHANGES_ELIMINATED, ATTACKER_ACTOR, 0,
	    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
}

void apply_thrash_about(unsigned int * thrashing_turns,
			unsigned int * thrashing_move, int move)
{
  if(*thrashing_turns == 0)
    {
      /* set to 2 or 3 turns */
      *thrashing_turns = genrand(2, 2);
      *thrashing_move = move;
    }
}

void apply_get_enraged(unsigned int * enraged)
{
  *enraged = 1;
}

void apply_start_biding(unsigned int * biding_turns, unsigned int * bide_damage)
{
  if(*biding_turns == 0)
    {
      /* go for 2-3 turns */
      *biding_turns = genrand(2,2);
      *bide_damage = 0;
    }
}

void apply_immobilize(unsigned int * trapping_damage,
		      unsigned int * trapping_move,
		      unsigned int * trapping_turns,
		      unsigned int * repeated_move_index, unsigned int index,
		      unsigned int use_move, unsigned int damage_done,
		      unsigned int * original_trapping_victim,
		      unsigned int * defender_recharge)
{
#if(DEBUG_ENGINE)
  fprintf(stderr, "***apply_immobilize sees %i trapping turns\n",
	  *trapping_turns);
#endif

      log_event(CANNOT_MOVE, DEFENDER_ACTOR, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      *trapping_damage = damage_done;
      *trapping_move = use_move;
      *repeated_move_index = index;
      *trapping_turns = choose_number_iterations();
      /* the first time we immobilize, we set the defender
	 to be the original, that way when a new pkmn switches
	 in, the move selector knows whether to apply recurrent
	 damage (flag is set) or reuse the trapping move and
	 recalculate the damage done (flag is clear) */
      *original_trapping_victim = 1;
      /* a successful trapping move cancels opponent's recharge */
      *defender_recharge = 0;
#if(DEBUG_ENGINE)
      fprintf(stderr, "***setting trapping turns to %i\n", *trapping_turns);
#endif
}

int finish_charging_move(unsigned int move, unsigned int * charging_move,
			 unsigned int *temp_invulnerable)
{
  /* If charging_move is set, we are in the second stage of a charging move */
  if(*charging_move != NONE)
    {
      assert(*charging_move == move);
      /* Make the flying/digging pkmn hittable again */
      *temp_invulnerable = 0;
      /* Unset the charging move. */
      *charging_move = NONE;
      /* Indicate that a charging move finish */
      return ATTACKER_FINISHED_CHARGING;
    }
  return NO_EVENTS;
}

int select_alternate_move(unsigned int * move, unsigned int opponents_last_move)
{
  int move_parameter;
  if(get_move_parameter(*move, &move_parameter) == -1)
    {
      fprintf(stderr, "Could not get move parameter.\n");
      exit(3);
    }

  while(move_parameter == RANDOM_ATTACK || move_parameter ==
	OPPONENTS_LAST_ATTACK)
    {
      /* handle metronome */
      if(move_parameter == RANDOM_ATTACK)
	{
	  log_event(USED_MOVE, ATTACKER_ACTOR, *move, CLIENT_LOG_OUTPUT
		    | DATABASE_LOG_OUTPUT);
	  /* don't let it choose metronome or struggle
	     this is verified <somewhere> */
	  while(*move == METRONOME || *move == STRUGGLE)
	    {
	      /* choose our random move */
	      *move = genrand(MOVE_MAX - 1, 1);
	    }
	}
      /* handle mirror move */
      else if(move_parameter == OPPONENTS_LAST_ATTACK)
	{
	  *move = opponents_last_move;
	  /* Could not copy any move, display failure */
	  if(move == NONE)
	    {
	      log_event(MOVE_FAILED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT
			| DATABASE_LOG_OUTPUT);
	    }
	}
      if(get_move_parameter(*move, &move_parameter) == -1)
	{
	  fprintf(stderr, "Could not get move parameter.\n");
	  exit(3);
	}
    }
  return NO_EVENTS;
}

int check_original_victim(unsigned int * move, unsigned int * trapping_turns,
			  unsigned int trapping_move,
			  unsigned int original_trapping_victim)
{
  /* If a trapping move is in used, the same damage is applied to the victim
     unless that user switches out. In that case the move is re-done
     to calculate the new damage and number of turns. */
  if(*trapping_turns > 0)
    {
      assert(trapping_move == *move);
      if(original_trapping_victim)
	{
	  /* If the user has not switched out, change our move to NONE
	     because the recurent damage code does the damage. Also no PP is
	     used. */
	  *move = NONE;
	  return SUPRESS_PP_USE;
	}
      else
	{
	  /* Set the trapping turns to 0; a new value will be chosen. */
	  *trapping_turns = 0;
	}
    }
  return NO_EVENTS;
}

int begin_charging(unsigned int * move, unsigned int * charging_move,
		   unsigned int * temp_invulnerable,
		   unsigned int * repeated_move_index, unsigned int index)
{
  int move_parameter;
  if(get_move_parameter(*move, &move_parameter) == -1)
    {
      fprintf(stderr, "Could not get move parameter.\n");
      exit(3);
    }
  /* Any move that requires two turns to complete must first complete a turn
     of setup where effectively no move is used. In the case of moves that fly
     or dig, the pkmn must be made unhittable as well. */
  if(move_parameter == UNDERGROUND || move_parameter == IN_AIR ||
     move_parameter == LOWER_HEAD || move_parameter == MAKE_WHIRLWIND ||
     move_parameter == GLOW || move_parameter == GATHER_SUNLIGHT)
    {
      *charging_move = *move;
      *repeated_move_index = index;
      if(move_parameter == UNDERGROUND || move_parameter == IN_AIR)
	{
	  if(move_parameter == UNDERGROUND)
	    {
	      log_event(DUG, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
			DATABASE_LOG_OUTPUT);
	    }
	  else if(move_parameter == IN_AIR)
	    {
	      log_event(FLEW, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
			DATABASE_LOG_OUTPUT);
	    }
	  *temp_invulnerable = 1;
	}
      else
	{
	  if(move_parameter == LOWER_HEAD)
	    {
	      log_event(LOWERED_HEAD, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
			DATABASE_LOG_OUTPUT);
	    }
	  else if(move_parameter == MAKE_WHIRLWIND)
	    {
	      log_event(MADE_WHIRLWIND, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
			DATABASE_LOG_OUTPUT);
	    }
	  else if(move_parameter == GLOW)
	    {
	      log_event(IS_GLOWING, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
			DATABASE_LOG_OUTPUT);
	    }
	  else if(move_parameter == GATHER_SUNLIGHT)
	    {
	      log_event(GATHERED_SUNLIGHT, ATTACKER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	}
      *move = NONE;
      return SUPRESS_PP_USE;
    }
  return NO_EVENTS;
}

/* compute_turn
 *
 *
 * TAKES
 *
 *RETURNS
 */
unsigned int compute_turn(User * attacking_user, Pkmn * attacker,
			  Active_data * attacker_data, User * defending_user,
			  Pkmn * defender, Active_data * defender_data,
			  unsigned int enforced_clauses, unsigned int move,
			  unsigned int index, unsigned int first_turn,
			  unsigned int prior_events, Event * move_event_buffer,
			  unsigned int move_event_count)
{
  unsigned int turn_events = NO_EVENTS;
  unsigned int use_move = NONE;
  int damage_done = 0;
  int attacker_stat;
  int original_attacker_stat;
  int defender_stat;
  int original_defender_stat;

  int move_type = 0;
  int move_power = 0;
  int move_parameter = 0;
  int move_accuracy = 0;
  int move_secondary_effect = 0;
  int move_secondary_chance = 0;

  int sub_broke = 0;

  log_event(HAS_STATUS, ATTACKER_ACTOR,
            attacker->status, DATABASE_LOG_OUTPUT);
  /* Report whether pkmn is sleeping */
  turn_events |= report_sleeping(attacker->status,
				 attacker->sleep_infliction_mechanism,
				 enforced_clauses,
				 &attacking_user->sleep_count);
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after sleep test: %i\n", turn_events);
#endif
    /* Decrement sleep turns, if sleeping */
  if(status_is_sleep(attacker->status))
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***prior to decrement %i sleep turns\n",
	      attacker->status);
#endif
      attacker->status--;
    }
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking if frozen\n");
#endif
      turn_events |= report_frozen(attacker->status);

    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after freeze test: %x\n", turn_events);
#endif
  if(attacker_data->flinched)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking if past turn prevented attacking\n");
#endif
      /* we flinched our previous turn, so clear that out */
      attacker_data->flinched = 0;
      log_event(FLINCHED, ATTACKER_ACTOR, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
      turn_events |= ATTACKER_PREVENTED_FROM_ATTACKING;
    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after attack prevention test: %x\n", turn_events);
#endif
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking immobilization\n");
#endif
      turn_events |= report_immobilized(defender_data->trapping_turns,
					defender_data->trapping_move);
    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after immobilize test: %x\n", turn_events);
#endif
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking recharge\n");
#endif
      turn_events |= report_recharge(&(attacker_data->recharging));
    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after recharge test: %x\n", turn_events);
#endif
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking paralysis\n");
#endif
      report_disabled_end(&(attacker_data->disable_turns));

      report_thrashing_about(&(attacker_data->thrashing_turns));

      report_confusion_status(&attacker_data->confusion_turns);

      turn_events |= report_full_paralysis(attacker->status,
					   &(attacker_data->thrashing_turns),
					   &attacker_data->charging_move,
					   &(attacker_data->temp_invulnerable));
    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after recharge par: %x\n", turn_events);
#endif
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***checking confusion damage\n");
#endif
      turn_events |=
	report_confusion_damage(attacker_data->confusion_turns,
				&attacker->current_hp, attacker->level,
				attacker_data->current_stats.attack,
				attacker_data->current_stats.defense,
				&(attacker_data->thrashing_turns),
				&attacker_data->charging_move,
				&(attacker_data->temp_invulnerable));
    }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after conf: %x\n", turn_events);
#endif
  if(turn_events == NO_EVENTS)
    {
#if(DEBUG_ENGINE)
  fprintf(stderr, "***checking if attack has become disabled\n");
#endif
  turn_events |=
    report_move_disabled(index, move, attacker_data->disabled_selection);
}
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after disabled: %x\n", turn_events);
#endif

  if(turn_events == NO_EVENTS)
    {
      /* Finish a charging move if one has been started. */
      turn_events |= finish_charging_move(move, &attacker_data->charging_move,
					  &attacker_data->temp_invulnerable);
    }
  /* If we are finishing a charging move, we aren't seleting an alternate
     move (metronome or mimic), we aren't using a trapping move, and we're
     obviously not beginning a trapping move. Therefore, we only do this
     if finish_charging_move has not changed turn_events. */
  if(turn_events == NO_EVENTS)
    {
      /* If we are not finishing a charging move or are otherwise unable to
	 attack, we need to unsure that there are not other factors that
	 dictate the attack that we use */
      select_alternate_move(&move, defender_data->last_move_used);
      turn_events |= check_original_victim(&move,
					   &attacker_data->trapping_turns,
					   attacker_data->trapping_move,
					   defender_data->
					   original_trapping_victim);
      turn_events |= begin_charging(&move, &attacker_data->charging_move,
				    &attacker_data->temp_invulnerable,
				    &attacker_data->repeated_move_index, index);
    }

  /* If no events have occured except for finishing charging, we should
     decrement PP and use the move */
  if((turn_events & ~ATTACKER_FINISHED_CHARGING) == NO_EVENTS)
    {
      if((~(prior_events | turn_events)) & SUPRESS_PP_USE)
	{
	  if(attacker_data->move[index].current_pp > 0)
	    {
	      attacker_data->move[index].current_pp--;
	    }
	}

      use_move = move;

      if(get_move_type(use_move, &move_type) == -1)
	{
	  fprintf(stderr, "Could not get move type\n");
	  exit(3);
	}
      if(get_move_parameter(use_move, &move_parameter) == -1)
	{
	  fprintf(stderr, "Could not get move parameter\n");
	  exit(3);
	}
      if(get_move_power(use_move, &move_power) == -1)
	{
	  fprintf(stderr, "Could not get move power\n");
	  exit(3);
	}
      if(get_move_accuracy(use_move, &move_accuracy) == -1)
	{
	  fprintf(stderr, "Could not get move accuracy\n");
	  exit(3);
	}

      /* set the move to be the last move used */
      attacker_data->last_move_used = use_move;
      /* Ensure that a valid move was selected */
      if(use_move != NONE)
	{
	  log_event(USED_MOVE, ATTACKER_ACTOR, move, CLIENT_LOG_OUTPUT |
		    DATABASE_LOG_OUTPUT);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***using move %i\n", use_move);
#endif
	  turn_events |= report_move_miss(move_accuracy, move_parameter,
					  attacker_data->accuracy_mod,
					  defender_data->evade_mod,
					  defender_data->temp_invulnerable,
					  defender->status,
					  attacker_data->charging_move,
					  first_turn);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***events after miss test: %x\n", turn_events);
#endif
#if(DEBUG_ENGINE)
	}
      fprintf(stderr, "***did move miss? status: %x\n", turn_events);
#endif
    }

  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
     !(turn_events & ATTACKER_MOVE_MISSES))
    {
#if(DEBUG_ENGINE)
      fprintf(stderr, "***attack was allowed to occur\n");
#endif
      if(get_attacking_stat(move_type) == ATTACK)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***type %i implies attack stat\n", move_type);
#endif
	  attacker_stat = calc_true_attack(attacker_data->current_stats.attack,
					   attacker_data->stat_mods.attack,
					   attacker->status);
	  original_attacker_stat = attacker_data->current_stats.attack;
	}
      else
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***type %i implies special stat\n", move_type);
#endif
	  attacker_stat =
	    calc_true_special(attacker_data->current_stats.special,
			      attacker_data->stat_mods.special, 0);
	  original_attacker_stat = attacker_data->current_stats.special;
	}
      if(get_defending_stat(move_type) == DEFENSE)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***type %i implies defense stat\n", move_type);
#endif
	  defender_stat =
	    calc_true_defense(defender_data->current_stats.defense,
			      defender_data->stat_mods.defense,
			      defender_data->physical_halve);
	  original_defender_stat = defender_data->current_stats.defense;
	}
      else
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***type %i implies special stat\n", move_type);
#endif
	  defender_stat =
	    calc_true_special(defender_data->current_stats.special,
			      defender_data->stat_mods.special,
			      defender_data->special_halve);
	   original_defender_stat = defender_data->current_stats.special;
	}

      /* the primary is largely just applying damage but also regulates
	 type-base misses for moves with only secondary effects */
      turn_events |=
	report_attack_primary(move_power, move_type, move_parameter,
			      &(attacker_data->type[0]), attacker_data->number,
			      defender_data->number, attacker->level,
			      attacker_stat, original_attacker_stat,
			      &(defender_data->type[0]), defender_stat,
			      original_defender_stat,
			      attacker_data->critical_hit_up,
			      (attacker_data->damage_is_counterable)?
			      attacker_data->damage_taken:0,
			      &(defender_data->substitute),
			      &defender->current_hp,
			      defender->max_hp, &damage_done);
      sub_broke = (turn_events & DEFENDER_SUB_BROKE);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***events after attack: %x\n",turn_events);
	  fprintf(stderr, "***damage done: %i\n", damage_done);
#endif
    }
  /* Handle actions that occur right after damage is applied. */
  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING))
    {
      turn_events |= report_unleashed_energy(attacker_data->biding_turns,
					     attacker_data->bide_damage,
					     &defender->current_hp,
					     &damage_done);

      report_biding(&(attacker_data->biding_turns));
#if(DEBUG_ENGINE)
      fprintf(stderr, "***events after bide done test: %x\n", turn_events);
#endif
    }
  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING))
    {
      report_thrashing_confusion(attacker_data->thrashing_turns,
				 &(attacker_data->confusion_turns));
    }

  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
     !(turn_events & ATTACKER_MOVE_MISSES))
    {
      report_thaw(move_type, &(defender->status), session.enforced_clauses,
		  &(defending_user->frozen_count));

      report_building_rage(defender_data->enraged, damage_done,
			   &(defender_data->stat_mods.attack));
    }

  /* Most seconary effects will engage under a typical set of circumstances,
     neither of our pkmn are currently fainted, and the attack went off as
     usual.

     TODO: Does it make sense to consider these a configurable property of the
     secondary effect, or to treat them as an intrinsic part, as they are now.*/

  if(get_move_secondary_effect(use_move, &move_secondary_effect) == -1)
    {
      fprintf(stderr, "Could not get move secondary effect.\n");
      exit(3);
    }
  if(get_move_secondary_chance(use_move, &move_secondary_chance) == -1)
    {
      fprintf(stderr, "Could not get move secondary chance.\n");
      exit(3);
    }

  switch(move_secondary_effect)
    {
      /* Suicide should occur regardless of the opponent's death or
         a miss. */
    case SUICIDE:
      if(!(turn_events & ATTACKER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING))
	{
	  turn_events |= apply_suicide(&attacker->current_hp);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***events after suicide test: %x\n", turn_events);
#endif
	  break;
	}
      /* Thrashing should occur regardless of a miss (you will be trapped
         thrashing even if the first thrash missed. */
    case THRASH_ABOUT:
      if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS)
	 && !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING))
	{
	  apply_thrash_about(&(attacker_data->thrashing_turns),
			     &(attacker_data->thrashing_move), use_move);
	}
      break;
    case QUARTER_RECOIL:
#if (RECOIL_ON_OPPONENT_FAINT)
      /* Recoil should occur even if the opponent faints. */
      if(!(turn_events & ATTACKER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#else
	/* Recoil should occur only under normal circumstances. */
      if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#endif
	{
	  turn_events |= apply_quarter_recoil(&attacker->current_hp,
					      damage_done);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***events after recoil test: %x\n", turn_events);
#endif
	}
      break;
#if (RECOIL_ON_OPPONENT_FAINT)
      /* Recoil should occur even if the opponent faints. */
    case ONE_DAMAGE_RECOIL:
      if(!(turn_events & ATTACKER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#else
	/* Recoil should occur only under normal circumstances. */
      if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#endif
	{
	  turn_events |= apply_one_damage_recoil(&attacker->current_hp);
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***events after 1d rcoil test: %x\n", turn_events);
#endif
	}
      break;
    case RECHARGE:
#if (RECARGE_ON_OPPONENT_FAINT)
      /* Recharge should occur even if the opponent faints. */
      if(!(turn_events & ATTACKER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#else
	/* Recharge should occur only under normal circumstances. */
      if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	 !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
#endif
	{
	  apply_recharge(&(attacker_data->recharging));
	}
      break;
    case LEECH:
      /* Leeching of HP should occur even if the opponent faints. */
       if(!(turn_events & ATTACKER_FAINTS) &&
	  !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
	  !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
	 {
	   apply_leech(&attacker->current_hp, attacker->max_hp, damage_done);
	 }
       break;
    }

  /* Now consider secondaries that engage during traditional circumstances.
     What is traditional?
     1) The attacker has not fainted.
     2) The defender has not fainted.
     3) The attack was not prevented.
     4) The secondary effect of the attack was not prevented. */
  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
     !(turn_events & ATTACKER_PREVENTED_FROM_SECONDARY))
    {
      switch(move_secondary_effect)
	{
	case ATT_UP_NRM:
	  apply_attack_up(&(attacker_data->stat_mods.attack), NORMALLY);
	  break;
	case ATT_UP_GRT:
	  apply_attack_up(&(attacker_data->stat_mods.attack), GREATLY);
	  break;
	case DEF_UP_NRM:
	  apply_defense_up(&(attacker_data->stat_mods.defense), NORMALLY);
	  break;
	case DEF_UP_GRT:
	  apply_defense_up(&(attacker_data->stat_mods.defense), GREATLY);
	  break;
	case SPD_UP_GRT:
	  apply_speed_up(&(attacker_data->stat_mods.speed), GREATLY);
	  break;
	case SPEC_UP_NRM:
	  apply_special_up(&(attacker_data->stat_mods.special), NORMALLY);
	  break;
	case SPEC_UP_GRT:
	  apply_special_up(&(attacker_data->stat_mods.special), GREATLY);
	  break;
	case EVD_UP_NRM:
	  apply_evade_up(&(attacker_data->evade_mod));
	  break;
	case CRITICAL_HIT_UP:
	  apply_critical_hit_up(&(attacker_data->critical_hit_up));
	  break;
	case ATT_DOWN_NRM:
	  apply_attack_down(move_secondary_chance,
			    &(defender_data->stat_mods.attack), NORMALLY,
			    defender_data->substitute, sub_broke,
			    defender_data->mod_lock);
	  break;
	case DEF_DOWN_NRM:
	  apply_defense_down(move_secondary_chance,
			     &(defender_data->stat_mods.defense), NORMALLY,
			     defender_data->substitute, sub_broke,
			     defender_data->mod_lock);
	  break;
	case DEF_DOWN_GRT:
	  apply_defense_down(move_secondary_chance,
			     &(defender_data->stat_mods.defense), GREATLY,
			     defender_data->substitute, sub_broke,
			     defender_data->mod_lock);
	  break;
	case SPD_DOWN_NRM:
	  apply_speed_down(move_secondary_chance,
			   &(defender_data->stat_mods.speed), NORMALLY,
			   defender_data->substitute, sub_broke,
			   defender_data->mod_lock);
	  break;
	case SPEC_DOWN_NRM:
	  apply_special_down(move_secondary_chance,
			     &(defender_data->stat_mods.special), NORMALLY,
			     defender_data->substitute, sub_broke,
			     defender_data->mod_lock);
	  break;
	case ACC_DOWN_NRM:
	  apply_accuracy_down((&defender_data->accuracy_mod),
			      defender_data->substitute, sub_broke,
			      defender_data->mod_lock);
	  break;
	case FREEZE:
	  apply_status(FRZ, &(defender->status), defender_data->substitute,
		       sub_broke, &(defender_data->type[0]),
		       move_secondary_chance,
		       &(defender->sleep_infliction_mechanism),
		       session.enforced_clauses, &defending_user->sleep_count,
		       &defending_user->frozen_count);
	  break;
	case PARALYZE:
	  apply_status(PAR, &(defender->status), defender_data->substitute,
		       sub_broke, &(defender_data->type[0]),
		       move_secondary_chance,
		       &(defender->sleep_infliction_mechanism),
		       session.enforced_clauses, &defending_user->sleep_count,
		       &defending_user->frozen_count);
	  break;
	case BURN:
	  apply_status(BRN, &(defender->status), defender_data->substitute,
		       sub_broke, &(defender_data->type[0]),
		       move_secondary_chance,
		       &(defender->sleep_infliction_mechanism),
		       session.enforced_clauses, &defending_user->sleep_count,
		       &defending_user->frozen_count);
	  break;
	case SLEEP:
	  apply_status(genrand(SLP, 1), &(defender->status),
		       defender_data->substitute, sub_broke,
		       &(defender_data->type[0]),
		       move_secondary_chance,
		       &(defender->sleep_infliction_mechanism),
		       session.enforced_clauses,
		       &defending_user->sleep_count,
		       &defending_user->frozen_count);
	  break;
	case POISON_MILD:
	  if(apply_status(PSN, &(defender->status), defender_data->substitute,
			  sub_broke, &(defender_data->type[0]),
			  move_secondary_chance,
			  &(defender->sleep_infliction_mechanism),
			  session.enforced_clauses,
			  &defending_user->sleep_count,
			  &defending_user->frozen_count) != -1)
	    {
	      log_event(WAS_POISONED, DEFENDER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  break;
	case POISON_BAD:
	  if(apply_status(PSN, &(defender->status), defender_data->substitute,
			  sub_broke, &(defender_data->type[0]),
			  move_secondary_chance,
			  &(defender->sleep_infliction_mechanism),
			  session.enforced_clauses,
			  &defending_user->sleep_count,
			  &defending_user->frozen_count) != -1)
	    {
	      /* on success of poising, set toxic damage
		 to be cumulative, if has already been set, reset */
	      defender_data->toxic_turns = 1;
	      log_event(WAS_BADLY_POISONED, DEFENDER_ACTOR, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	    }
	  break;
	case CONFUSE:
	  apply_confusion(&(defender_data->confusion_turns),
			  defender_data->substitute, sub_broke,
			  move_secondary_chance);
	  break;
	case SEED:
	  apply_seed(&(defender_data->seeded), &(defender_data->type[0]),
		     defender_data->substitute, sub_broke);
	  break;
	case HALVE_SPEC_DAM:
	  apply_special_halve(&(attacker_data->special_halve));
	  break;
	case HALVE_PHYS_DAM:
	  apply_physical_halve(&(attacker_data->physical_halve));
	  break;
	case STATISTIC_LOCK:
	  apply_mod_lock(&(attacker_data->mod_lock));
	  break;
	case COPY_PKMN:
	  apply_copy_pkmn(&(attacker_data->number), defender_data->number,
			  &(attacker_data->current_stats),
			  &(defender_data->current_stats),
			  &(attacker_data->stat_mods),
			  &(defender_data->stat_mods),
			  &(attacker_data->evade_mod),
			  defender_data->evade_mod,
			  &(attacker_data->accuracy_mod),
			  defender_data->accuracy_mod, attacker_data->type,
			  defender_data->type, attacker->move,
			  attacker->num_moves, attacker_data->move,
			  &attacker_data->num_moves,
			  defender_data->move, defender_data->num_moves);
	  break;
	case CONVERT_TYPE:
	  apply_convert_type(&(attacker_data->type[0]),
			     &(defender_data->type[0]));
	  break;
	case COPY_MOVE:
	  apply_copy_move(index, &(attacker_data->move[0]),
			  &(attacker->move[0]), &(defender_data->move[0]),
			  defender_data->num_moves);
	  break;
	case HEAL_HALF:
	  apply_heal_half(&attacker->current_hp, attacker->max_hp);
	  break;
	case SLEEP_AND_HEAL:
	  apply_sleep_and_heal(&attacker->current_hp, attacker->max_hp,
			       &(attacker->status),
			       &(attacker->sleep_infliction_mechanism));
	  break;
	case MAKE_SUB:
	  apply_make_sub(&attacker->current_hp, attacker->max_hp,
			 &(attacker_data->substitute));
	  break;
	case FLINCH:
	  apply_flinch(&(defender_data->flinched), defender->status,
		       defender_data->substitute, sub_broke, first_turn,
		       move_secondary_chance);
	  break;
	case DISABLE_MOVE:
	  apply_disable_move(&(defender_data->disabled_selection),
			     &(defender_data->disable_turns),
			     &(defender_data->move[0]),
			     defender_data->num_moves);
	  break;
	case ELIMINATE_CHANGES:
	  apply_eliminate_changes(&(attacker_data->stat_mods),
				  &(defender_data->stat_mods),
				  &(attacker_data->evade_mod),
				  &(defender_data->evade_mod),
				  &(attacker_data->accuracy_mod),
				  &(defender_data->accuracy_mod),
				  &(attacker_data->physical_halve),
				  &(defender_data->physical_halve),
				  &(attacker_data->special_halve),
				  &(attacker_data->special_halve),
				  &(attacker_data->critical_hit_up),
				  &(defender_data->critical_hit_up),
				  &(attacker_data->mod_lock),
				  &(defender_data->mod_lock),
				  &(attacker_data->confusion_turns),
				  &(defender_data->confusion_turns),
				  &(attacker_data->seeded),
				  &(defender_data->seeded),
				  &(attacker_data->toxic_turns),
				  &(defender_data->toxic_turns),
				  &(attacker->status), &(defender->status),
				  attacker->sleep_infliction_mechanism,
				  defender->sleep_infliction_mechanism,
				  session.enforced_clauses,
				  &attacking_user->sleep_count,
				  &defending_user->sleep_count,
				  &attacking_user->frozen_count,
				  &defending_user->frozen_count);
	  break;
	case GET_ENRAGED:
	  apply_get_enraged(&(attacker_data->enraged));
	  break;
	case START_BIDING:
	  apply_start_biding(&(attacker_data->biding_turns),
			     &(attacker_data->bide_damage));
	  break;
	}

    }
  /* with the secondaries out of the way, we handle recurrent
     stuff */
  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS))
    {
      turn_events |=
	report_poison_burn_damage(attacker->status,
				  &(attacker_data->toxic_turns),
				  &attacker->current_hp, attacker->max_hp);
#if(DEBUG_ENGINE)
      fprintf(stderr, "***events after poison test: %x\n", turn_events);
#endif
    }

  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS))
    {
      turn_events |=
	report_leech_damage(attacker_data->seeded,
			    &(attacker_data->toxic_turns),
			    &attacker->current_hp, attacker->max_hp,
			    &defender->current_hp, defender->max_hp);

#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after leech damage test: %x\n", turn_events);
#endif
    }
  /* do trapping test and damage before the immobilization
     effect has been applied, otherwise it will double-deal
     on the first turn */

  if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS))
    {
      turn_events |=
	report_trapping_damage(attacker_data->trapping_turns,
			       attacker_data->trapping_move,
			       attacker_data->trapping_damage,
			       &defender->current_hp,
			       &damage_done);
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events after trap damage test: %x\n", turn_events);
#endif
    }

  /* once the trapping damage check has been done, put into
     trapping mode if applicable */
   if(!(turn_events & ATTACKER_FAINTS) && !(turn_events & DEFENDER_FAINTS) &&
      !(turn_events & ATTACKER_PREVENTED_FROM_ATTACKING) &&
      !(turn_events & ATTACKER_MOVE_MISSES))
     {
       if(move_secondary_effect == IMMOBILIZE)
	 {
	   apply_immobilize(&(attacker_data->trapping_damage),
			    &(attacker_data->trapping_move),
			    &(attacker_data->trapping_turns),
			    &(attacker_data->repeated_move_index), index,
			    use_move, damage_done,
			    &(defender_data->original_trapping_victim),
			    &(defender_data->recharging));
	 }
     }

   if(turn_events & ATTACKER_FAINTS)
    {
      /* if the attacker faints, don't have the defender flinch when the new
	 pkmn is sent in */
      defender_data->flinched = 0;
    }
#if(DEBUG_ENGINE)
  fprintf(stderr,"***defender takes %i damage\n", damage_done);
#endif
  /* record the damage taken as a result of normal damage */
  defender_data->damage_taken = damage_done;
  defender_data->damage_is_counterable =
    (move_type == NORMAL || move_type == FIGHTING);
#if(DEBUG_ENGINE)
  fprintf(stderr, "***defender takes counterable damage? %i\n",
	  defender_data->damage_is_counterable);
#endif
  /* if attacker did damage, up defender's bide damage */
  if(damage_done > 0 && defender_data->biding_turns > 0) {

    defender_data->bide_damage += damage_done;
    log_event(BUILDING_ENERGY, DEFENDER_ACTOR, 0,
		    CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
#if(DEBUG_ENGINE)
    fprintf(stderr, "***defender takes %i damage for bide total %i\n",
	    damage_done, defender_data->bide_damage);
#endif
  }
#if(DEBUG_ENGINE)
  fprintf(stderr, "***events for this turn %x\n", turn_events);
#endif

  if(turn_events & ATTACKER_FAINTS)
    {
      make_faint(attacker, enforced_clauses, &attacking_user->frozen_count,
		 &attacking_user->sleep_count);
    }
  if(turn_events & DEFENDER_FAINTS)
    {
      make_faint(defender, enforced_clauses, &defending_user->frozen_count,
		 &defending_user->sleep_count);
    }

  return turn_events;
}


/* compute_round

 * TAKES
 * command0,1 the command to use such as SWITCH or ATTACK
 * option0,1 the arguement to the command, such as to whom
 *           or which move
 * previous_status the result of the last round

 * RETURNS a code based on the outcome of the round
 */
unsigned int compute_round(int command0, int option0, int command1,
			   int option1)
{
  Pkmn * switch_to;
  int first_attacker;
  unsigned int ret_val = NORMAL_ROUND;

  unsigned turn_events[2] = {NO_EVENTS, NO_EVENTS};
  unsigned move_events[2] = {NO_EVENTS, NO_EVENTS};
  unsigned int previous_status = session.status;

  int move_parameters[2];
  int moves[2];

  Event move_event_buffers[2][NUM_EVENTS];
  unsigned int move_event_counters[2] = {0, 0};
  int num_translated_events = 0;

  unsigned int chosen_index[2];

  LOG_RESET();

  /* check if a user ran */
  if(command0 == RUNNING && command1 == RUNNING)
    {
      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_0, 0, DATABASE_LOG_OUTPUT);
      log_event(RAN_AWAY, ABSOLUTE_ACTOR_0, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);
      log_event(RAN_AWAY, ABSOLUTE_ACTOR_1, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(USERS_TIE, 0, 0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      ret_val = TIE;
    }
  else if(command0 == RUNNING)
    {
      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_0, 0, DATABASE_LOG_OUTPUT);
      log_event(RAN_AWAY, ABSOLUTE_ACTOR_0, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(USER_LOSES, ABSOLUTE_ACTOR_0, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(USER_WINS, ABSOLUTE_ACTOR_1, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      ret_val = LOSS_0;
    }
  else if(command1 == RUNNING)
    {
      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);
      log_event(RAN_AWAY, ABSOLUTE_ACTOR_1, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(USER_LOSES, ABSOLUTE_ACTOR_1, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(USER_WINS, ABSOLUTE_ACTOR_0, 0,
		CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      ret_val = LOSS_1;
    }
  else
    {
      /* Now with that out of the way, we can proceed to
	 correcting commands based on battle conditions */

      /* If the user is not allowed to switch make certain it is attacking */
      if(session.user[0].active_data.recharging ||
	 session.user[0].active_data.charging_move != NONE ||
	 session.user[0].active_data.enraged ||
	 session.user[0].active_data.thrashing_turns > 0)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***corrected user 0's command from %i\n",
		  command0);
#endif
	  command0 = ATTACKING;
	}
      if(session.user[1].active_data.recharging ||
	 session.user[1].active_data.charging_move != NONE ||
	 session.user[1].active_data.enraged ||
	 session.user[1].active_data.thrashing_turns > 0)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***corrected user 1's command from"
		  " %i\n", command0);
#endif
	  command1 = ATTACKING;
	}
      /* Depending on who fainted, it may be necessary to force switch */
      /* Do this correction after the previous one, otherwise fainted pkmn will
	 get to use their multiturn attacks */
#if(DEBUG_ENGINE)
      fprintf(stderr, "***previous status is: %x\n", previous_status);
#endif
      correct_commands_for_faint(previous_status, &command0, &command1);
#if(DEBUG_ENGINE)
      fprintf(stderr, "***commands are now %i %i\n", command0, command1);
#endif

      /* Take care of switching first, as switch precedes attack */
      if(command0 == SWITCHING)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***user 0 is switching\n");
#endif
	  /* only switch to a valid pkmn, return NULL if none
	     are valid */

	  if((switch_to = can_switch(option0, session.user[0].total,
				     &session.user[0].pkmn
				     [session.user[0].num_active],
				     session.user[0].pkmn)) != NULL)
	    {
	      /* log a turn delimiter */
	      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_0, 0, DATABASE_LOG_OUTPUT);

	      num_translated_events++;
#if(DEBUG_ENGINE)
	      fprintf(stderr, "***changing to valid switch %i\n",
		      switch_to->roster_number);
#endif
	      /* make the validated switch occur */
	      do_switch(switch_to, &session.user[0].pkmn
			[session.user[0].num_active],
			&session.user[0].num_active,
			&(session.user[0].active_data),
			session.enforced_clauses, &session.user[0].sleep_count,
			&session.user[0].frozen_count);

	      log_event(SENT_OUT, ABSOLUTE_ACTOR_0,
			0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      /* do not translate event, it is already absolute */

	      num_translated_events++;
	    }
	  else /* no valid switches */
	    {
#if(DEBUG_ENGINE)
	      fprintf(stderr, "***no switch available, attacking\n");
#endif
	      command0 = ATTACKING; /* force to attack */
	    }
	}
      if(command1 == SWITCHING)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***user 1 is switching\n");
#endif
	  /* only switch to a valid pkmn */

	  if((switch_to = can_switch(option1, session.user[1].total,
				     &session.user[1].pkmn
				     [session.user[1].num_active],
				     session.user[1].pkmn)) != NULL)
	    {
	      /* log a turn delimiter */
	      log_event(BEGIN_TURN, ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);

	      num_translated_events++;
#if(DEBUG_ENGINE)
	      fprintf(stderr, "***changing to valid switch %i\n",
		      switch_to->roster_number);
#endif
	      do_switch(switch_to,
			&session.user[1].pkmn[session.user[1].num_active],
			&session.user[1].num_active,
			&(session.user[1].active_data),
			session.enforced_clauses, &session.user[1].sleep_count,
			&session.user[1].frozen_count);

	      log_event(SENT_OUT, ABSOLUTE_ACTOR_1,
			0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      /* do not translate event, it is already absolute */
	      num_translated_events++;
	    }
	  else /* no valid switches */
	    {
#if(DEBUG_ENGINE)
	      fprintf(stderr, "***no switch available, attacking\n");
#endif
	      command1 = ATTACKING; /* force to attack */
	    }
	}

      /* user 0 attacks, user 1 has switched or can't attack */
      if(command0 == ATTACKING && command1 != ATTACKING)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***user 0 is attacking\n");
#endif
	  /* log a turn delimiter */
	  log_event(BEGIN_TURN, ABSOLUTE_ACTOR_0, 0,
		    DATABASE_LOG_OUTPUT);

	  num_translated_events++;

	  /* Sanitize the client input */
	  chosen_index[0] = abs(option0) %
	    session.user[0].active_data.num_moves;

	  turn_events[1] =
	    select_move(&moves[0], &chosen_index[0],
			session.user[0].active_data.num_moves,
			session.user[0].active_data.move,
			session.user[0].active_data.charging_move,
			session.user[0].active_data.trapping_move,
			session.user[0].active_data.trapping_turns,
			session.user[1].active_data.original_trapping_victim,
			session.user[0].active_data.repeated_move_index,
			session.user[0].active_data.thrashing_move,
			session.user[0].active_data.thrashing_turns,
			session.user[0].active_data.enraged,
			session.user[0].active_data.biding_turns);

	  turn_events[1] = compute_turn(&session.user[0],
					&session.user[0].pkmn
					[session.user[0].num_active],
					&(session.user[0].active_data),
					&session.user[1],
					&session.user[1].pkmn
					[session.user[1].num_active],
					&(session.user[1].active_data),
					session.enforced_clauses,
					moves[0], chosen_index[0], 0,
					turn_events[1], move_event_buffers[0],
					move_event_counters[0]);

	  translate_relative_events(log_register + num_translated_events,
				    log_counter, 0);

	  ret_val = process_events(turn_events[1], 0,
				   &(session.user[0].fainted),
				   session.user[0].total,
				   &(session.user[1].fainted),
				   session.user[1].total);
	}
      /* user 1 attacks, user 1 has switched or can't attack */
      else if(command1 == ATTACKING && command0 != ATTACKING)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***user 1 is attacking\n");
#endif
	  /* log a turn delimiter */
	  log_event(BEGIN_TURN, ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);

	  num_translated_events++;

	  chosen_index[1] = abs(option1) %
	    session.user[1].active_data.num_moves;

	  turn_events[1] =
	    select_move(&moves[1], &chosen_index[1],
			session.user[1].active_data.num_moves,
			session.user[1].active_data.move,
			session.user[1].active_data.charging_move,
			session.user[1].active_data.trapping_move,
			session.user[1].active_data.trapping_turns,
			session.user[0].active_data.original_trapping_victim,
			session.user[1].active_data.repeated_move_index,
			session.user[1].active_data.thrashing_move,
			session.user[1].active_data.thrashing_turns,
			session.user[1].active_data.enraged,
			session.user[1].active_data.biding_turns);

	  turn_events[1] = compute_turn(&session.user[1],
					&session.user[1].pkmn
					[session.user[1].num_active],
					&(session.user[1].active_data),
					&session.user[0],
					&session.user[0].pkmn
					[session.user[0].num_active],
					&(session.user[0].active_data),
					session.enforced_clauses,
					moves[1], chosen_index[1], 0,
					turn_events[1], move_event_buffers[1],
					move_event_counters[1]);


	  translate_relative_events(log_register + num_translated_events,
				    log_counter, 1);

	  ret_val = process_events(turn_events[1], 1,
				   &(session.user[1].fainted),
				   session.user[1].total,
				   &(session.user[0].fainted),
				   session.user[0].total);
	}

      /* both are attacking */
      else if(command0 == ATTACKING && command1 == ATTACKING)
	{
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***both users are attacking\n");
#endif
	  /* Sanatize user input for each user */
	  chosen_index[0] = abs(option0) %
	    session.user[0].active_data.num_moves;
	  chosen_index[1] = abs(option1) %
	    session.user[1].active_data.num_moves;

	  /* Check the move that is going to be used, correcting for
	     preexisting factors */
	  move_events[0] =
	    select_move(&moves[0], &chosen_index[0],
			session.user[0].active_data.num_moves,
			session.user[0].active_data.move,
			session.user[0].active_data.charging_move,
			session.user[0].active_data.trapping_move,
			session.user[0].active_data.trapping_turns,
			session.user[1].active_data.original_trapping_victim,
			session.user[0].active_data.repeated_move_index,
			session.user[0].active_data.thrashing_move,
			session.user[0].active_data.thrashing_turns,
			session.user[0].active_data.enraged,
			session.user[0].active_data.biding_turns);

	  if(get_move_parameter(moves[0], &move_parameters[0]) == -1)
	    {
	      fprintf(stderr, "Could not get move parameter.\n");
	      exit(3);
	    }

	  move_events[1] =
	    select_move(&moves[1], &chosen_index[1],
			session.user[1].active_data.num_moves,
			session.user[1].active_data.move,
			session.user[1].active_data.charging_move,
			session.user[1].active_data.trapping_move,
			session.user[1].active_data.trapping_turns,
			session.user[0].active_data.original_trapping_victim,
			session.user[1].active_data.repeated_move_index,
			session.user[1].active_data.thrashing_move,
			session.user[1].active_data.thrashing_turns,
			session.user[1].active_data.enraged,
			session.user[1].active_data.biding_turns);

	  if(get_move_parameter(moves[1], &move_parameters[1]) == -1)
	    {
	      fprintf(stderr, "Could not get move parameter.\n");
	      exit(3);
	    }

	  /* determine who goes first */
	  first_attacker =
	    compute_attack_order(calc_true_speed(session.user[0].active_data.
						 current_stats.speed,
						 session.user[0].active_data.
						 stat_mods.speed,
						 session.user[0].pkmn
						 [session.user[0].num_active].
						 status),
				 priority_of(move_parameters[0]),
				 calc_true_speed(session.user[1].active_data.
						 current_stats.speed,
						 session.user[1].active_data.
						 stat_mods.speed,
						 session.user[1].pkmn
						 [session.user[1].num_active].
						 status),
				 priority_of(move_parameters[1]));

	  /* Run the first turn, keeping track of the turn events that
	     occur. */
	  log_event(BEGIN_TURN, (!first_attacker) ? ABSOLUTE_ACTOR_0 :
		    ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);

	  num_translated_events++;

	  turn_events[0] =
	    compute_turn(&session.user[first_attacker],
			 &session.user[first_attacker].pkmn
			 [session.user[first_attacker].num_active],
			 &(session.user[first_attacker].active_data),
			 &session.user[!first_attacker],
			 &session.user[!first_attacker].pkmn
			 [session.user[!first_attacker].num_active],
			 &(session.user[!first_attacker].active_data),
			 session.enforced_clauses, moves[first_attacker],
			 chosen_index[first_attacker], 1,
			 move_events[first_attacker],
			 move_event_buffers[first_attacker],
			 move_event_counters[first_attacker]);

	  /* take the events generated by the turn, these are in a form
	     such that the attacker and defender is noted, and translate
	     then to the absolute user numbers (user 0, user 1) */
	  translate_relative_events(log_register + num_translated_events,
				    log_counter, first_attacker);
	  num_translated_events = log_counter;


	  /* take these events and determine if we can continue */
	  ret_val = process_events(turn_events[0], first_attacker,
				   &(session.user[first_attacker].fainted),
				   session.user[first_attacker].total,
				   &(session.user[!first_attacker].fainted),
				   session.user[!first_attacker].total);

	  /* if nothing such as fainting has made the round unusual,
	     continue with the other user's attacking turn */
	  if(ret_val == NORMAL_ROUND)
	    {
	      log_event(BEGIN_TURN, (first_attacker) ? ABSOLUTE_ACTOR_0 :
			ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);

	      num_translated_events++;

	      turn_events[1] =
		compute_turn(&session.user[!first_attacker],
			     &session.user[!first_attacker].pkmn
			     [session.user[!first_attacker].num_active],
			     &(session.user[!first_attacker].active_data),
			     &session.user[first_attacker],
			     &session.user[first_attacker].pkmn
			     [session.user[first_attacker].num_active],
			     &(session.user[first_attacker].active_data),
			     session.enforced_clauses, moves[!first_attacker],
			     chosen_index[!first_attacker], 0,
			     turn_events[0] | move_events[!first_attacker],
			     move_event_buffers[!first_attacker],
			     move_event_counters[!first_attacker]);

	      translate_relative_events(log_register + num_translated_events,
					log_counter, !first_attacker);

	      ret_val = process_events(turn_events[1], !first_attacker,
				       &(session.user[!first_attacker].fainted),
				       session.user[!first_attacker].total,
				       &(session.user[first_attacker].fainted),
				       session.user[first_attacker].total);

	    }
	}

      /* after both sides have gone decrement trapping turns */
      if(session.user[0].active_data.trapping_turns > 0)
	{
	  session.user[0].active_data.trapping_turns--;
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***brought 0's trapping turns to %i\n",
		  session.user[0].active_data.trapping_turns);
#endif

	}
      if(session.user[1].active_data.trapping_turns > 0)
	{
	  session.user[1].active_data.trapping_turns--;
#if(DEBUG_ENGINE)
	  fprintf(stderr, "***brought 1's trapping turns to %i\n",
		  session.user[1].active_data.trapping_turns);
#endif
	}
    }
  session.status = ret_val;
  return ret_val;
}

unsigned int process_events(unsigned int events, unsigned int attacker_id,
			    unsigned int * attacker_fainted,
			    unsigned int attacker_total,
			    unsigned int * defender_fainted,
			    unsigned int defender_total)
{
  int ret_val = NORMAL_ROUND;
  if((events & ATTACKER_FAINTS) && (events & DEFENDER_FAINTS))
    {
      (*attacker_fainted)++;
      (*defender_fainted)++;
      ret_val = FAINTED_BOTH;
      log_event(FAINTED, 0, 0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
      log_event(FAINTED, 1, 0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
    }
  else if(events & ATTACKER_FAINTS)
    {
      (*attacker_fainted)++;
      ret_val = (attacker_id == 0)? FAINTED_0 : FAINTED_1;
      log_event(FAINTED, attacker_id, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
    }
  else if(events & DEFENDER_FAINTS)
    {
      (*defender_fainted)++;
      ret_val = (attacker_id == 0) ? FAINTED_1 : FAINTED_0;
      log_event(FAINTED, !attacker_id, 0, CLIENT_LOG_OUTPUT |
		DATABASE_LOG_OUTPUT);
    }
  if(ret_val == FAINTED_0 || ret_val == FAINTED_1 || ret_val == FAINTED_BOTH)
    {
      if(*attacker_fainted == attacker_total &&
	 *defender_fainted == defender_total)
	{
	  log_event(USERS_TIE, 0, 0, CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	  return TIE;
	}
      else if(*attacker_fainted == attacker_total)
	{
	  if(attacker_id == 0)
	    {
	      log_event(USER_LOSES, ABSOLUTE_ACTOR_0, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      log_event(USER_WINS, ABSOLUTE_ACTOR_1, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      return LOSS_0;
	    }
	  else
	    {
	      log_event(USER_LOSES, ABSOLUTE_ACTOR_1, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      log_event(USER_WINS, ABSOLUTE_ACTOR_0, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      return LOSS_1;
	    }
	}
      else if(*defender_fainted == defender_total)
	{
	  if(attacker_id == 0)
	    {
	      log_event(USER_LOSES, ABSOLUTE_ACTOR_1, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      log_event(USER_WINS, ABSOLUTE_ACTOR_0, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      return LOSS_1;
	    }
	  else
	    {
	      log_event(USER_LOSES, ABSOLUTE_ACTOR_0, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      log_event(USER_WINS, ABSOLUTE_ACTOR_1, 0,
			CLIENT_LOG_OUTPUT | DATABASE_LOG_OUTPUT);
	      return LOSS_0;
	    }
	}
    }
  return ret_val;
}

void do_sendout(int starter0, int starter1)
{
  log_event(BEGIN_TURN, ABSOLUTE_ACTOR_0, 0, DATABASE_LOG_OUTPUT);

  log_event(SENT_OUT, ABSOLUTE_ACTOR_0, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);

  session.user[0].num_active = starter0;
  write_active_data(&session.user[0].pkmn[starter0],
		    &session.user[0].active_data);

  log_event(BEGIN_TURN, ABSOLUTE_ACTOR_1, 0, DATABASE_LOG_OUTPUT);

  log_event(SENT_OUT, ABSOLUTE_ACTOR_1, 0, CLIENT_LOG_OUTPUT |
	    DATABASE_LOG_OUTPUT);

  /* all our events are absolute, so don't bother translating */

  session.user[1].num_active = starter1;
  write_active_data(&session.user[1].pkmn[starter1],
		    &session.user[1].active_data);
}
