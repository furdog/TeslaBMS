#include <LittleFS.h>
#include "counter.h"

struct tbox_filesystem {
	bool commit;
	
	struct tbox_counter *counter;
	
	clock_t timer;
};

void tbox_filesystem_init(struct tbox_filesystem *self,
			  struct tbox_counter *counter)
{
	self->commit = false;
	
	self->counter = counter;

	self->timer = 0;
	
#if defined(ESP32)	
	if (!LittleFS.begin(true)) {
#else
	if (!LittleFS.begin()) {
#endif
		Serial.println(F("Failed to init filesystem"));
		return;
	}
}

void tbox_filesystem_save(struct tbox_filesystem *self)
{
	if (!self->commit)
		return;

	self->commit = false;

	File file = LittleFS.open("/state.bin", "w");
	if (!file) {
		Serial.println(F("Failed to open file for writing"));

		return;
	}

	file.write((uint8_t *)self->counter, sizeof(struct tbox_counter));

	file.close();
}

void tbox_filesystem_load(struct tbox_filesystem *self) {
	File file = LittleFS.open("/state.bin", "r");
	if (!file) {
		Serial.println(F("Failed to open file for reading"));
		
		self->commit = true;
		tbox_filesystem_save(self);
		
		return;
	}

	struct tbox_counter counter;
	file.read((uint8_t *)&counter, sizeof(struct tbox_counter));

	self->counter->total_consumed_wh     = counter.total_consumed_wh;
	self->counter->remaining_capacity_wh = counter.remaining_capacity_wh;

	file.close();
}

void tbox_filesystem_commit(struct tbox_filesystem *self)
{
	self->commit = true;
}

void tbox_filesystem_update(struct tbox_filesystem *self, clock_t delta)
{
	self->timer += delta;
	
	if (self->timer >= 30000) {
		if (self->commit)
			tbox_filesystem_save(self);

		self->timer = 0;
	}
}
