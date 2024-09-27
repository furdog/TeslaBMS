#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "async.h"

#define TBOX_CONTACTOR_INIT_TIME           10000
#define TBOX_CONTACTOR_MAX_VOLTAGE_DEFAULT 4.25
#define TBOX_CONTACTOR_MIN_VOLTAGE_DEFAULT 2.5
#define TBOX_CONTACTOR_MAX_CURRENT_DEFAULT 200
#define TBOX_CONTACTOR_MAX_TEMP_DEFAULT    50

enum tbox_contactor_event {
	TBOX_CONTACTOR_EVENT_NONE,
	TBOX_CONTACTOR_EVENT_OPEN,
	TBOX_CONTACTOR_EVENT_CLOSE
};

struct tbox_contactor {
	async state;
	async substate;
	
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
	
	//timers
	clock_t timer;
};

void tbox_contactor_init(struct tbox_contactor *self)
{
	self->state = 0;
	self->substate = 0;
	
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

void tbox_contactor_ack_voltage(struct tbox_contactor *self, float minv,
				float maxv)
{
	self->voltage_violation = (maxv > self->max_voltage ||
				   minv < self->min_voltage) ? 1 : 0;
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

enum tbox_contactor_event tbox_contactor_update_sub(
				struct tbox_contactor *self)
{
	ASYNC_DISPATCH(self->substate);

	if (!tbox_contactor_has_condition_violation(self)) {		
		//Looks like everything fine at this point
		//we can close contactor
		ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_CLOSE);
		
		//Wait until condition violation, forever
		ASYNC_AWAIT(tbox_contactor_has_condition_violation(self),
			    return TBOX_CONTACTOR_EVENT_NONE);
	}

	//If condition been violated - open contactor
	ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_OPEN);

	while (true) {
		//Reset timer if still in violation
		if (tbox_contactor_has_condition_violation(self))
			self->timer = 0;

		//If not in violation for 30 second
		if (self->timer >= 30000)
			break;

		ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_NONE);
	}
	
	//Reset this routine
	ASYNC_RESET(return TBOX_CONTACTOR_EVENT_NONE);
}

enum tbox_contactor_event tbox_contactor_update(struct tbox_contactor *self,
						clock_t delta)
{
	self->timer += delta;
	
	ASYNC_DISPATCH(self->state);
	
	ASYNC_YIELD(return TBOX_CONTACTOR_EVENT_OPEN);

	//Wait for startup
	self->timer = 0;
	ASYNC_AWAIT(self->timer >= TBOX_CONTACTOR_INIT_TIME,
		    return TBOX_CONTACTOR_EVENT_NONE);

	while (true)
		ASYNC_YIELD(return tbox_contactor_update_sub(self));
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
