#include <stdio.h>

#define TBOX_DEBUG
#include "contactor.h"
#include "counter.h"

struct tbox_contactor contactor;
struct tbox_counter counter;
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

	return 0;
}
