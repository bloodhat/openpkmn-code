#include <stdio.h>
#include <stdlib.h>

#include "eutil.h"
#include "environment.h"
#include "datatypes.h"
#include "packet.h"

unsigned int get_normal_count(Session * session,
			      unsigned int user_id)
{
  int i;
  int ct = 0;
  for(i = 0; i < session->user[user_id].total; i++)
    {
      /* we must check for 0 hp rather than fainted status because
	 the fainted status is only applied on the switch out */
      if(session->user[user_id].pkmn[i].status == NRM &&
	 session->user[user_id].pkmn[i].current_hp > 0)
	{
	  ct++;
	}
    }
  return ct;
}

unsigned int get_status_count(Session * session,
			      unsigned int user_id)
{
  int i;
  int ct = 0;
  for(i = 0; i < session->user[user_id].total; i++)
    {
      /* consider statused to be not normal status and not fainted */
      /* we must check for 0 hp rather than fainted status because
	 the fainted status is only applied on the switch out */
      if(session->user[user_id].pkmn[i].status != NRM &&
	 session->user[user_id].pkmn[i].current_hp > 0) {
	ct++;
      }
    }
  return ct;
}

unsigned int get_hp_bars(Pkmn * pkmn)
{
  /* get out of 255 (8 bits)*/
  return (int)(255 * ((float)pkmn->current_hp/
		      (float)pkmn->max_hp));
}

int get_user_change_from_status(unsigned int status,
				unsigned int user_id)
{
  int ret_status;
  switch(status)
    {
    case NORMAL_ROUND:
      ret_status = MS_NORMAL_ROUND;
      break;
     case FAINTED_0:
      if(user_id == 0)
	{
	  ret_status = MS_OWN_FAINT;
	}
      else
	{
	  ret_status = MS_OPP_FAINT;
	}
      break;
    case FAINTED_1:
      if(user_id == 1)
	{
	  ret_status = MS_OWN_FAINT;
	}
      else
	{
	  ret_status = MS_OPP_FAINT;
	}
      break;
    case FAINTED_BOTH:
      ret_status = MS_BOTH_FAINT;
      break;
    case LOSS_0:
      if(user_id == 0)
	{
	  ret_status = MS_OWN_LOSS;
	}
      else
	{
	  ret_status = MS_OPP_LOSS;
	}
      break;
    case LOSS_1:
      if(user_id == 1)
	{
	  ret_status = MS_OWN_LOSS;
	}
      else
	{
	  ret_status = MS_OPP_LOSS;
	}
      break;
    case TIE:
      ret_status = MS_TIE;
      break;
    default:
      fprintf(stderr, "BAD STATUS\n");
      return -1;
      break;
    }
  return ret_status;
}
