//Proxy layer for current meter

#ifndef  TBOX_CURRENT_METER_GUARD
#define  TBOX_CURRENT_METER_GUARD

struct tbox_current_meter {
	bool ready;
	bool safe_to_use;

	float current;
	float max_input_current;
	float max_output_current;

	uint32_t ack_timer;
	uint32_t ack_timeout;
};

void tbox_current_meter_init(struct tbox_current_meter *self)
{
	self->ready       = false;
	self->safe_to_use = false;

	self->current            = 0;
	self->max_input_current  = 0;
	self->max_output_current = 0;

	self->ack_timer   = 0;
	self->ack_timeout = 0;
}

//////////////////// SETUP ////////////////////
void tbox_current_meter_set_max_input_current_A(
			    struct tbox_current_meter *self, float max_current)
{
	self->max_input_current = max_current;
}

void tbox_current_meter_set_max_output_current_A(
			    struct tbox_current_meter *self, float max_current)
{
	self->max_output_current = max_current;
}

void tbox_current_meter_set_max_current_A(struct tbox_current_meter *self,
					  float max_current)
{
	self->max_input_current  = max_current;
	self->max_output_current = max_current;
}

void tbox_current_meter_set_ack_timeout_ms(struct tbox_current_meter *self,
					   clock_t ack_timeout)
{
	self->ack_timeout = ack_timeout;
}

//////////////////// API ////////////////////
void tbox_current_meter_ack_ready(struct tbox_current_meter *self, bool ready)
{
	self->ready = ready;

	if (!ready) {
		self->safe_to_use = false;
		
		self->ack_timer = 0;
	}
}

void tbox_current_meter_ack_current_A(struct tbox_current_meter *self,
				     float current)
{	
	self->current = current;
}

bool tbox_current_meter_is_safe_to_use(struct tbox_current_meter *self)
{
	return self->safe_to_use;
}

float tbox_current_meter_get_current_A(struct tbox_current_meter *self)
{
	return self->current;
}

//////////////////// UPDATE ////////////////////
enum tbox_current_meter_event {
	TBOX_CURRENT_METER_EVENT_NONE,
	TBOX_CURRENT_METER_EVENT_NOT_READY,
	TBOX_CURRENT_METER_EVENT_ACK_TIMEOUT,
	TBOX_CURRENT_METER_EVENT_INPUT_OVERCURRENT,
	TBOX_CURRENT_METER_EVENT_OUTPUT_OVERCURRENT
};

enum tbox_current_meter_event tbox_current_meter_update(
				struct tbox_current_meter *self, clock_t delta)
{
	self->safe_to_use = false;

	if (!self->ready)
		return TBOX_CURRENT_METER_EVENT_NOT_READY;

	if (self->ack_timer < self->ack_timeout)
		self->ack_timer += delta;

	if (self->ack_timer >= self->ack_timeout)
		return TBOX_CURRENT_METER_EVENT_ACK_TIMEOUT;
	
	if (self->current >= self->max_input_current)
		return TBOX_CURRENT_METER_EVENT_INPUT_OVERCURRENT;

	if (-self->current >= self->max_output_current) 
		return TBOX_CURRENT_METER_EVENT_OUTPUT_OVERCURRENT;
	
	self->safe_to_use = true;

	self->ack_timer = 0;

	return TBOX_CURRENT_METER_EVENT_NONE;
}

//////////////////// DEBUG ////////////////////
const char *tbox_current_meter_get_event_name(enum tbox_current_meter_event ev)
{
	switch (ev) {
	case TBOX_CURRENT_METER_EVENT_NONE:        return "NONE";
	case TBOX_CURRENT_METER_EVENT_NOT_READY:   return "NOT_READY";
	case TBOX_CURRENT_METER_EVENT_ACK_TIMEOUT: return "ACK_TIMEOUT";
	case TBOX_CURRENT_METER_EVENT_INPUT_OVERCURRENT:
		return "INPUT_OVERCURRENT";
	case TBOX_CURRENT_METER_EVENT_OUTPUT_OVERCURRENT:
		return "OUTPUT_OVERCURRENT";
	default: return "INVALID";
	}
}

#endif //TBOX_CURRENT_METER_GUARD

