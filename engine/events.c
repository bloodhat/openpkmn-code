#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "events.h"


#define DEBUG_EVENTS 1

Event log_register[NUM_EVENTS];
unsigned int log_counter = 0;

#if(0)
void consume_buffer(Event * event_buffer, unsigned int event_count)
{
  int i;
  for(i = 0; i < event_count; i++) {
    if(log_counter < NUM_EVENTS)
      {
	memcpy(&log_register[log_counter], &event_buffer[i],
	       sizeof(log_register[0]));
	log_counter++;
      }
    else
      {
	fprintf(stderr, "Log overflow.\n");
	exit(1);
      }
  }
}
#endif

void log_event(unsigned int event_id, unsigned int actor_id,
	       unsigned int details, unsigned int outputs)
{
  if(log_counter < NUM_EVENTS)
    {
#if(DEBUG_EVENTS)
      fprintf(stderr, "~~~added event %i\n", event_id);
#endif
      log_register[log_counter].message_id = event_id;
      log_register[log_counter].absolute_actor_id =
	log_register[log_counter].relative_actor_id = actor_id;
      log_register[log_counter].details = details;
      log_register[log_counter].outputs = outputs;
      log_counter++;
    }
  else
    {
      fprintf(stderr, "Log overflow.\n");
      exit(3);
    }
}

#if(0)
void buffer_event(unsigned int event_id,
		  unsigned int actor_id,
		  unsigned int details,
		  unsigned int outputs,
		  Event * buffer,
		  unsigned int * counter)
{
#if(DEBUG_EVENTS)
    if(*counter < NUM_EVENTS)
    {
      fprintf(stderr, "~~~buffering event %i\n",
	      event_id);
#endif
      buffer[*counter].message_id =
	event_id;
      buffer[*counter].absolute_actor_id =
	buffer[*counter].relative_actor_id =
	actor_id;
      buffer[*counter].details =
	details;
      buffer[*counter].outputs =
	outputs;
      (*counter)++;
    }
  else
    {
      fprintf(stderr, "Log overflow.\n");
      exit(1);
    }
}
#endif

/* each event is marked as being completed by the turn's
   attacker or the defender, this translates them
   into the absolute (challenger and opponent) roles */
void translate_relative_events(Event * events, unsigned int num_events,
			       unsigned int round_actor) {
  int i;
  for(i = 0; i < num_events; i++)
    {
      events[i].absolute_actor_id =
	!(events[i].relative_actor_id ^ round_actor);
    }
}
  
