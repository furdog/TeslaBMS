#ifndef  TBOX_COUNTER_GUARD
#define  TBOX_COUNTER_GUARD

#include <time.h>
#include <stdio.h>
#include <assert.h>

#define TBOX_COUNTER_SAMPLE_PERIOD_MS 1000
#define TBOX_COUNTER_MAX_CELL_VOLTAGE 4.18

enum tbox_counter_event {
	TBOX_COUNTER_EVENT_NONE,
	TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT
};

struct tbox_counter {
	float total_consumed_wh;
	float total_capacity_wh;
	float remaining_capacity_wh;

	bool sample_been_read;
	bool max_capacity_reached;

	clock_t sample_timer;
};

void tbox_counter_init(struct tbox_counter *self)
{
	self->total_consumed_wh     = 0;
	self->total_capacity_wh     = 0;
	self->remaining_capacity_wh = 0;

	self->sample_been_read     = false;
	self->max_capacity_reached = false;

	//float watts_per_second;
	self->sample_timer = 0;
}

float tbox_counter_clampf(float value, float min, float max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	
	return value;
}

//set/get total consumed_wh
void tbox_counter_set_total_consumed_wh(struct tbox_counter *self, float wh)
{
	self->total_consumed_wh = wh;
}

float tbox_counter_get_total_consumed_wh(struct tbox_counter *self)
{
	return self->total_consumed_wh;
}

//set/get total capacity
void tbox_counter_set_total_capacity_wh(struct tbox_counter *self,
					float wh)
{
	self->total_capacity_wh = wh;
}

float tbox_counter_get_total_capacity_wh(struct tbox_counter *self)
{
	return self->total_capacity_wh;
}

//set/get remaining_capacity
void tbox_counter_set_remaining_capacity_wh(struct tbox_counter *self,
					    float wh)
{
	self->remaining_capacity_wh = wh;
}

float tbox_counter_get_remaining_capacity_wh(struct tbox_counter *self)
{
	return self->remaining_capacity_wh;
}

//other getters
float tbox_counter_get_cycles(struct tbox_counter *self)
{
	if (self->total_capacity_wh <= 0.0)
		return 0.0;
		
	return self->total_consumed_wh / self->total_capacity_wh;
}

float tbox_counter_get_soc_normalized(struct tbox_counter *self)
{
	if (self->total_capacity_wh <= 0.0)
		return 0.0;
	
	return self->remaining_capacity_wh / self->total_capacity_wh;
}

void tbox_counter_ack_voltage_and_current(struct tbox_counter *self,
					  float voltage, float current)
{
	if (isnan(voltage) || isnan(current))
		return;
	
	if (self->sample_been_read)
		return;

	float delta = (voltage * current) *               //MS IN HOUR
		((double)TBOX_COUNTER_SAMPLE_PERIOD_MS / (1000 * 60 * 60));

	if (self->max_capacity_reached) {
		self->remaining_capacity_wh = self->total_capacity_wh;
	} else {
		self->remaining_capacity_wh += delta;

		self->remaining_capacity_wh = tbox_counter_clampf(
			self->remaining_capacity_wh, 0,
			self->total_capacity_wh);
	}

	self->total_consumed_wh += delta;
	
	self->sample_been_read = true;
}

void tbox_counter_ack_max_cell(struct tbox_counter *self, float voltage)
{
	self->max_capacity_reached = (voltage > TBOX_COUNTER_MAX_CELL_VOLTAGE);
}

enum tbox_counter_event tbox_counter_update(struct tbox_counter *self,
					    clock_t delta)
{
	self->sample_timer += delta;

	if (self->sample_timer >= TBOX_COUNTER_SAMPLE_PERIOD_MS) {
		self->sample_been_read = false;

		self->sample_timer -= TBOX_COUNTER_SAMPLE_PERIOD_MS;
	}
	
	if (self->sample_been_read == false)
		return TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT;
	
	return TBOX_COUNTER_EVENT_NONE;
}

//////////////////// DEBUG ////////////////////
#ifdef   TBOX_DEBUG
const char *tbox_counter_get_event_name(enum tbox_counter_event event)
{
	switch (event) {
	case TBOX_COUNTER_EVENT_NONE:                return "NONE";
	case TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT: return 
		"QUERY_VOLTAGE_AND_CURRENT";
	}
	
	return NULL;
}
#endif

#endif //TBOX_COUNTER_GUARD
