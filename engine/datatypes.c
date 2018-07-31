#include "datatypes.h"
#include "mechanics.h"

#include "../share/move_defs.h"

void init_active_data(Active_data * a_data)
{
  a_data->evade_mod = BASE_EVADE_MOD;
  a_data->accuracy_mod = BASE_ACCURACY_MOD;

  a_data->stat_mods.attack = BASE_STAT_MOD;
  a_data->stat_mods.defense = BASE_STAT_MOD;
  a_data->stat_mods.speed = BASE_STAT_MOD;
  a_data->stat_mods.special = BASE_STAT_MOD;

  a_data->physical_halve = 0;
  a_data->special_halve = 0;

  a_data->critical_hit_up = 0;

  a_data->mod_lock = 0;

  a_data->substitute = 0;

  a_data->toxic_turns = 0;
  a_data->confusion_turns = 0;

  a_data->seeded = 0;
  a_data->enraged = 0;
  a_data->flinched = 0;

  a_data->recharging = 0;

  a_data->disabled_selection = NONE_DISABLED;
  a_data->disable_turns = 0;

  a_data->trapping_move = NONE;
  a_data->trapping_damage = 0;
  a_data->trapping_turns = 0;

  a_data->original_trapping_victim = 0;

  a_data->thrashing_move = NONE;
  a_data->thrashing_turns = 0;

  a_data->charging_move = NONE;
  a_data->temp_invulnerable = 0;
  a_data->repeated_move_index = NONE;

  a_data->damage_taken = 0;
  a_data->damage_is_counterable = 0;

  a_data->biding_turns = 0;
  a_data->bide_damage = 0;

  a_data->last_move_used = NONE;
}
