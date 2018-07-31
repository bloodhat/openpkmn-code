#include <stdio.h>

#include "db_iface.h"
#include "stats.h"

int hp_lower_bound(int base, int level)
{
  float fBase = base;
  float fDV = 0;
  float fLevel = level;
  float fStatExp = 0;
  
  return (int)((fLevel * (fBase + fDV +
			  (fStatExp / 2048.0) + 50.0) / 50.0) + 10.0);
}

int hp_upper_bound(int base, int level)
{
  float fBase = base;
  float fDV = 15;
  float fLevel = level;
  float fStatExp = 65535;
  
  return (int)((fLevel * (fBase + fDV +
			  (fStatExp / 2048.0) + 50.0) / 50.0) + 10.0);
}

int stat_lower_bound(int base, int level)
{
  float fBase = base;
  float fDV = 0;
  float fLevel = level;
  float fStatExp = 0;
  
  return (int)((fLevel * (fBase + fDV +
			  (fStatExp / 2048.0)) / 50.0) + 5.0);
}

int stat_upper_bound(int base, int level)
{
  float fBase = base;
  float fDV = 15;
  float fLevel = level;
  float fStatExp = 65535;
  
  return (int)((fLevel * (fBase + fDV +
			  (fStatExp / 2048.0)) / 50.0) + 5.0);
}

int hp_stat_valid(int test_hp, int id, int level)
{
  int base;
  int lower;
  int upper;
  if(get_base_hp(id, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", id);
      return 0;
    }
  
  lower = hp_lower_bound(base, level);
  upper = hp_upper_bound( base, level);
  fprintf(stderr, "%i <= %i <= %i\n", lower, test_hp, upper); 
  return test_hp >= lower && test_hp <= upper;
}

int attack_stat_valid(int test_attack, int id, int level)
{
  int base;
  int lower;
  int upper;
  if(get_base_attack(id, &base) < -1)
    {
      fprintf(stderr, "could not get base attack for %i\n", id);
      return 0;
    }
  lower = stat_lower_bound(base, level);
  upper = stat_upper_bound( base, level);
  fprintf(stderr, "%i <= %i <= %i\n", lower, test_attack, upper); 
  return test_attack >= lower && test_attack <= upper;
}

int defense_stat_valid(int test_defense, int id, int level)
{
  int base;
  int lower;
  int upper;
  if(get_base_defense(id, &base) < -1)
    {
      fprintf(stderr, "could not get base defense for %i\n", id);
      return 0;
    }
  lower = stat_lower_bound(base, level);
  upper = stat_upper_bound( base, level);
  fprintf(stderr, "%i <= %i <= %i\n", lower, test_defense, upper); 
  return test_defense >= lower && test_defense <= upper;
}

int speed_stat_valid(int test_speed, int id, int level)
{
  int base;
  int lower;
  int upper;
  if(get_base_speed(id, &base) < -1)
    {
      fprintf(stderr, "could not get base speed for %i\n", id);
      return 0;
    }
  lower = stat_lower_bound(base, level);
  upper = stat_upper_bound( base, level);
  fprintf(stderr, "%i <= %i <= %i\n", lower, test_speed, upper); 
  return test_speed >= lower && test_speed <= upper;
}

int special_stat_valid(int test_special, int id, int level)
{
  int base;
  int lower;
  int upper;
  if(get_base_special(id, &base) < -1)
    {
      fprintf(stderr, "could not get base special for %i\n", id);
      return 0;
    }
  lower = stat_lower_bound(base, level);
  upper = stat_upper_bound( base, level);
  fprintf(stderr, "%i <= %i <= %i\n", lower, test_special, upper); 
  return test_special >= lower && test_special <= upper;
}


int get_max_hp(int species, int * max_hp)
{
  int base;
  if(get_base_hp(species, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", species);
      return -1;
    }
  *max_hp = hp_upper_bound(base, 100);
  return 1;
}

int get_max_attack(int species, int * max_attack)
{
  int base;
  if(get_base_attack(species, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", species);
      return -1;
    }
  *max_attack = stat_upper_bound(base, 100);
  return 1;
}

int get_max_defense(int species, int * max_defense)
{
  int base;
  if(get_base_defense(species, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", species);
      return -1;
    }
  *max_defense = stat_upper_bound(base, 100);
  return 1;
}

int get_max_speed(int species, int * max_speed)
{
  int base;
  if(get_base_speed(species, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", species);
      return -1;
    }
  *max_speed = stat_upper_bound(base, 100);
  return 1;
}

int get_max_special(int species, int * max_special)
{
  int base;
  if(get_base_special(species, &base) < -1)
    {
      fprintf(stderr, "could not get base hp for %i\n", species);
      return -1;
    }
  *max_special = stat_upper_bound(base, 100);
  return 1;
}
