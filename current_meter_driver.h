#include "INA226.h"

#define TBOX_CURRENT_METER_DRIVER_INA226_ADDR    0x40
#define TBOX_CURRENT_METER_DRIVER_INA226_SDA     2
#define TBOX_CURRENT_METER_DRIVER_INA226_SCL     15
#define TBOX_CURRENT_METER_DRIVER_MAX_CURRENT_A  150
#define TBOX_CURRENT_METER_DRIVER_ACK_TIMEOUT_MS 1000

INA226 INA(TBOX_CURRENT_METER_DRIVER_INA226_ADDR);

#include "current_meter.h"

void tbox_current_meter_driver_init(struct tbox_current_meter *self)
{
	tbox_current_meter_init(self);
	tbox_current_meter_set_max_current_A(self,
				      TBOX_CURRENT_METER_DRIVER_MAX_CURRENT_A);
	tbox_current_meter_set_ack_timeout_ms(self,
				     TBOX_CURRENT_METER_DRIVER_ACK_TIMEOUT_MS);
	
	Serial.print("INA226_LIB_VERSION: ");
	Serial.println(INA226_LIB_VERSION);

	Wire.begin(TBOX_CURRENT_METER_DRIVER_INA226_SDA,
		   TBOX_CURRENT_METER_DRIVER_INA226_SCL);

	bool ina_fail = !INA.begin();
	if (ina_fail)
		Serial.println("could not connect to INA. Fix and Reboot");
	  
	int err = INA.setMaxCurrentShunt(200, 0.000375, false);
	if (err != INA226_ERR_NONE)
		Serial.println(String() + "INA226 ERROR: " + String(err));

	//If no errors from ina, ack ready
	if (!ina_fail && err == INA226_ERR_NONE)
		tbox_current_meter_ack_ready(self, true);
}

void tbox_current_meter_driver_update(struct tbox_current_meter *self,
				      clock_t delta)
{
	tbox_current_meter_ack_current_A(self, INA.getCurrent());
	tbox_current_meter_update(self, delta);
}
