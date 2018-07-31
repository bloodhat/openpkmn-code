#ifndef __OPENPKMN_ENGINE_UTILS__
#define __OPENPKMN_ENGINE_UTILS__ 1

#include "datatypes.h"

enum round_statuses { NORMAL_ROUND = 0,
		      FAINTED_0, FAINTED_1,
		      FAINTED_BOTH,
		      LOSS_0, LOSS_1,
		      TIE};

unsigned int get_normal_count(Session * session,
			      unsigned int user_id);

unsigned int get_status_count(Session * session,
			      unsigned int user_id);

unsigned int get_hp_bars(Pkmn * pkmn);

int get_user_change_from_status(unsigned int status,
				unsigned int user_id);
#endif
