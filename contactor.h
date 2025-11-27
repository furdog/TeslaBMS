#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "async.h"

#define TBOX_CONTACTOR_INIT_TIME           5000
//#define TBOX_CONTACTOR_MAX_VOLTAGE_DEFAULT 4.30
#define TBOX_CONTACTOR_MAX_VOLTAGE_DEFAULT 4.25
#define TBOX_CONTACTOR_MIN_VOLTAGE_DEFAULT 2.5
#define TBOX_CONTACTOR_MAX_CURRENT_DEFAULT 150
#define TBOX_CONTACTOR_MAX_TEMP_DEFAULT    50

enum tbox_contactor_event {
	TBOX_CONTACTOR_EVENT_NONE,
	TBOX_CONTACTOR_EVENT_OPEN,
	TBOX_CONTACTOR_EVENT_CLOSE
};

struct tbox_contactor {
	async async_state;
	async async_conditions_state;
	
	enum tbox_contactor_event last_event;
	
	float max_voltage;
	float min_voltage;
	float max_current;
	float max_temp;
	
	//If any condition has been violated, set this to true;
	bool condition_violation;

	int  voltage_violation: 1;
	int  current_violation: 1;
	int  temp_violation:    1;
	int  faults_violation:  1;

	bool ready;
	
	//timers
	clock_t timer;
};

void tbox_contactor_init(struct tbox_contactor *self)
{
	self->async_state	     = 0;
	self->async_conditions_state = 0;
	
	self->last_event = TBOX_CONTACTOR_EVENT_NONE;
	
	self->max_voltage = TBOX_CONTACTOR_MAX_VOLTAGE_DEFAULT;
	self->min_voltage = TBOX_CONTACTOR_MIN_VOLTAGE_DEFAULT;
	self->max_current = TBOX_CONTACTOR_MAX_CURRENT_DEFAULT;
	self->max_temp    = TBOX_CONTACTOR_MAX_TEMP_DEFAULT;

	//Set all to true to ensure all conditions were acked after start;
	self->voltage_violation = 1;
	self->current_violation = 1;
	self->temp_violation    = 1;
	self->faults_violation  = 1;

	self->timer = 0;
}

/* Tell contactor module that we ready for it to be closed.
   otherwise it will open immediately. */
void tbox_contactor_ack_ready(struct tbox_contactor *self, bool ready)
{
	self->ready = ready;

	if (!ready)
		self->async_conditions_state = 0;
}

void tbox_contactor_ack_voltage(struct tbox_contactor *self, float minv,
				float maxv)
{
	self->voltage_violation = (maxv >= self->max_voltage ||
				   minv <= self->min_voltage) ? 1 : 0;
}

void tbox_contactor_ack_current(struct tbox_contactor *self, float current)
{
	self->current_violation = (fabs(current) > self->max_current) ? 1 : 0;
}

void tbox_contactor_ack_temp(struct tbox_contactor *self, float temp)
{
	self->temp_violation = (temp > self->max_temp) ? 1 : 0;
}

void tbox_contactor_ack_faults(struct tbox_contactor *self, bool has_faults)
{
	self->faults_violation = has_faults ? 1 : 0;
}

bool tbox_contactor_has_condition_violation(struct tbox_contactor *self)
{
	return (self->voltage_violation || self->current_violation ||
		self->temp_violation || self->faults_violation);
}

enum tbox_contactor_event tbox_contactor_set_event(
					struct tbox_contactor *self,
					enum tbox_contactor_event event)
{
	if (self->last_event == event)
		return TBOX_CONTACTOR_EVENT_NONE;
	
	self->last_event = event;
	
	return event;
}

enum tbox_contactor_event tbox_contactor_get_event(struct tbox_contactor *self)
{
	return self->last_event;
}

enum tbox_contactor_event tbox_contactor_update_conditions(
				struct tbox_contactor *self)
{
	ASYNC_DISPATCH(self->async_conditions_state);

	ASYNC_AWAIT(self->ready, return TBOX_CONTACTOR_EVENT_OPEN);

	//Wait until condition violation, forever
	ASYNC_AWAIT(tbox_contactor_has_condition_violation(self),
		    return TBOX_CONTACTOR_EVENT_CLOSE);

	//If condition been violated - open contactor
	ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_OPEN);

	while (true) {
		//Reset timer if still in violation
		if (tbox_contactor_has_condition_violation(self))
			self->timer = 0;

		//If not in violation for 30 second
		if (self->timer >= 30000)
			break;

		ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_OPEN);
	}
	
	//Reset this routine
	ASYNC_RESET(return TBOX_CONTACTOR_EVENT_OPEN);
}

enum tbox_contactor_event tbox_contactor_update(struct tbox_contactor *self,
						clock_t delta)
{
	self->timer += delta;
	
	ASYNC_DISPATCH(self->async_state);

	ASYNC_YIELD(return tbox_contactor_set_event(self,
						   TBOX_CONTACTOR_EVENT_OPEN));

	//START TIMER
	self->timer = 0;
	ASYNC_AWAIT(self->timer >= TBOX_CONTACTOR_INIT_TIME,
		    return TBOX_CONTACTOR_EVENT_NONE);

	while (true)
		ASYNC_YIELD(return tbox_contactor_set_event(self,
				   tbox_contactor_update_conditions(self)));
}

//////////////////// DEBUG ////////////////////
#ifdef   TBOX_DEBUG
const char *tbox_contactor_get_event_name(enum tbox_contactor_event event)
{
	switch (event) {
	case TBOX_CONTACTOR_EVENT_NONE:  return "NONE";
	case TBOX_CONTACTOR_EVENT_OPEN:  return "OPEN";
	case TBOX_CONTACTOR_EVENT_CLOSE: return "CLOSE";
	}
	
	return NULL;
}
#endif
