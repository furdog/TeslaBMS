#include <stdio.h>

#define TBOX_DEBUG
#include "contactor.h"

struct tbox_contactor contactor;
int main()
{
	tbox_contactor_init(&contactor);
	
	tbox_contactor_ack_voltage(&contactor, 3.7, 3.7);
	tbox_contactor_ack_current(&contactor, 0);
	tbox_contactor_ack_faults(&contactor, false);
	tbox_contactor_ack_temp(&contactor, 21);
	
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
			printf("%i: %s\n", i,
				tbox_contactor_get_event_name(ev));
		}
		
		ev_delta = ev;
	}
	
	return 0;
}
