/* turn off wrapping for this file */
#include "mechanics.h"


/* [attacking type][defending type] */
const float type_chart[TYPE_MAX][TYPE_MAX]=
  {	{1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  },
	{1  ,NVE,NVE,SE ,1  ,SE ,1  ,1  ,1  ,1  ,1  ,NVE,SE ,1  ,1  ,NVE},
	{1  ,SE ,NVE,NVE,1  ,1  ,1  ,1  ,1  ,1  ,SE ,SE ,1  ,1  ,1  ,NVE},
	{1  ,NVE,SE ,NVE,1  ,1  ,1  ,1  ,1  ,NVE,SE ,SE ,NVE,NVE,1  ,NVE},
	{1  ,1  ,SE ,NVE,NVE,1  ,1  ,1  ,1  ,SE ,0  ,1  ,1  ,1  ,1  ,NVE},
	{1  ,1  ,NVE,SE ,1  ,NVE,1  ,1  ,1  ,SE ,SE ,1  ,1  ,1  ,1  ,SE },
	{1  ,1  ,1  ,1  ,1  ,1  ,NVE,1  ,SE ,1  ,1  ,1  ,1  ,SE ,1  ,1  },
	{1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,NVE,1  ,1  ,0  ,1  },
	{1  ,1  ,1  ,1  ,1  ,SE ,NVE,SE ,1  ,NVE,1  ,SE ,NVE,NVE,0  ,1  },
	{1  ,1  ,1  ,SE ,NVE,1  ,1  ,1  ,SE ,1  ,1  ,NVE,SE ,1  ,1  ,1  },
	{1  ,SE ,1  ,NVE,SE ,1  ,1  ,1  ,1  ,0  ,1  ,SE ,NVE,SE ,1  ,1  },
	{1  ,SE ,1  ,1  ,1  ,SE ,1  ,1  ,NVE,SE ,NVE,1  ,SE ,1  ,1  ,1  },
	{1  ,NVE,1  ,SE ,1  ,1  ,SE ,1  ,NVE,NVE,1  ,1  ,1  ,SE ,1  ,1  },
	{1  ,1  ,1  ,SE ,1  ,1  ,1  ,1  ,1  ,1  ,NVE,NVE,SE ,NVE,NVE,1  },
	{1  ,1  ,1  ,1  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,1  ,1  ,SE ,1  },
	{1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  }
  };

const float hit_mods_val[8]={0.0, 1.0/3.0, 2.0/4.0, 2.5/4.5,
			     3.0/5.0, 4.0/6.0, 5.0/7.0, 1.0};

const float stat_mod_val[13]={1.0/4.0,2.0/7.0,1.0/3.0,
			      2.0/5.0,1.0/2.0,2.0/3.0,
			      1.0,1.5,2.0,2.5,3.0,3.5,4.0};

unsigned int calc_true_attack(unsigned int base_attack,
			       unsigned int mod_val,
			       unsigned int status)
{
  return base_attack * stat_mod_val[mod_val]
    * ((status == BRN)? .5 : 1);
}

unsigned int calc_true_defense(unsigned int base_defense,
			       unsigned int mod_val,
			       unsigned int reflect_active)
{
  return base_defense * stat_mod_val[mod_val] * ((reflect_active)? 2 : 1);
}

unsigned int calc_true_speed(unsigned int base_speed,
			     unsigned int mod_val,
			     unsigned int status)
{
  return base_speed * stat_mod_val[mod_val]
    * ((status == PAR)? .25 : 1);
}

unsigned int calc_true_special(unsigned int base_special, unsigned int mod_val,
			       unsigned int light_screen_active)
{
  return base_special * stat_mod_val[mod_val] * (light_screen_active? 2 : 1);
}

unsigned int get_attacking_stat(int type)
{
  return ((type >= NORMAL || type == 0) ? ATTACK : SPECIAL);
}

unsigned int get_defending_stat(int type)
{
  return ((type >= NORMAL || type == 0) ? DEFENSE : SPECIAL);
}
