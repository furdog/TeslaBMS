#include <stdio.h>

#define TBOX_DEBUG
#include "contactor.h"
#include "counter.h"
#include "current_meter.h"

struct tbox_contactor contactor;
struct tbox_counter counter;
struct tbox_current_meter cmeter;

int main()
{
	printf("TESTING CONTACTOR\n");

	tbox_contactor_init(&contactor);
	
	tbox_contactor_ack_voltage(&contactor, 3.7, 3.7);
	tbox_contactor_ack_current(&contactor, 0);
	tbox_contactor_ack_faults(&contactor, false);
	tbox_contactor_ack_temp(&contactor, 21);
	tbox_contactor_ack_ready(&contactor, true);
	
	enum tbox_contactor_event ev_delta = TBOX_CONTACTOR_EVENT_NONE;
	
	for (int i = 0; i < 100000; i++) {
		enum tbox_contactor_event ev =
			tbox_contactor_update(&contactor, 1);
		
		if (i == 5005) {
			tbox_contactor_ack_faults(&contactor, true);
		}

		if (i == 5010) {
			tbox_contactor_ack_faults(&contactor, false);
		}

		if (ev != ev_delta) {
			printf("%i: %s\n", i + 1,
				tbox_contactor_get_event_name(ev));
		}
		
		ev_delta = ev;
	}

	printf("TESTING METRICS\n");
	tbox_counter_init(&counter);

	tbox_counter_set_total_capacity_wh(&counter, 1);
	//tbox_counter_ack_max_cell(&counter, 4.7);

	for (int i = 0; i < 500 * 60 * 60; i++) {
		enum tbox_counter_event ev =
			tbox_counter_update(&counter, 1);

		if (ev == TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT)
			tbox_counter_ack_voltage_and_current(&counter, 1, 1);

		/*if (ev) {
			printf("%i: %s\n", i + 1,
				tbox_counter_get_event_name(ev));
		}*/
	}
	
	printf("SOC: %f\n", tbox_counter_get_soc_normalized(&counter) * 100.0);

	printf("TESTING CURRENT METER\n");
	tbox_current_meter_init(&cmeter);
	tbox_current_meter_ack_ready(&cmeter, true);
	
	#define PRINT_SAFE \
	printf("safe to use: %s\n", \
		tbox_current_meter_is_safe_to_use(&cmeter) ? "true" : "false")

	#define PRINT_UPDATE(time) \
	printf("update event: %s\n", tbox_current_meter_get_event_name( \
				    tbox_current_meter_update(&cmeter, time)));

	//must be false when init
	PRINT_SAFE;

	tbox_current_meter_ack_current_A(&cmeter, 0);

	//Must be false if not configured
	PRINT_SAFE;
	
	//Must tell timeout since no timeout configured
	PRINT_UPDATE(0);

	tbox_current_meter_set_ack_timeout_ms(&cmeter, 100);

	//must tell overcurrent, since no limit configured
	PRINT_UPDATE(0);

	tbox_current_meter_set_max_current_A(&cmeter, 100);

	//Must be ok
	PRINT_UPDATE(0);

	PRINT_SAFE;

	//Must be overcurrent and not safe to use
	tbox_current_meter_ack_current_A(&cmeter, 100);

	PRINT_UPDATE(0);

	PRINT_SAFE;

	return 0;
}
