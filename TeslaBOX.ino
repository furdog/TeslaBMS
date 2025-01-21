#include "filesystem.h"
//#include "web_server.h"
/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

#include <lvgl.h>

#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif

/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 *Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
 *as the examples and demos are now part of the main LVGL library. */

//#include <examples/lv_examples.h>
//#include <demos/lv_demos.h>

lv_display_t * disp;

/*Set to your screen resolution*/
#define TFT_HOR_RES   320
#define TFT_VER_RES   240

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print( lv_log_level_t level, const char * buf )
{
	LV_UNUSED(level);
	Serial.println(buf);
	Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
	/*Copy `px map` to the `area`*/

	/*For example ("my_..." functions needs to be implemented by you)
	uint32_t w = lv_area_get_width(area);
	uint32_t h = lv_area_get_height(area);

	my_set_window(area->x1, area->y1, w, h);
	my_draw_bitmaps(px_map, w * h);
	 */

	/*Call it to tell LVGL you are ready*/
	lv_disp_flush_ready(disp);
}

typedef struct {
	TFT_eSPI * tft;
} lv_tft_espi_t;

/*Read the touchpad*/
void my_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data )
{
	/*For example  ("my_..." functions needs to be implemented by you) */
	uint16_t x, y;

	bool touched = ((lv_tft_espi_t *)lv_display_get_driver_data(disp))->tft->getTouch(&x, &y, 600); // read the touchpad

	if(!touched) {
		data->state = LV_INDEV_STATE_RELEASED;
	} else {
		data->state = LV_INDEV_STATE_PRESSED;

		data->point.x = TFT_HOR_RES - x;
		data->point.y = y;
	}
}

///////////////////////////////////////////////////////////////////////////////
static clock_t timestamp_prev = 0;
static clock_t timestamp      = 0;

clock_t get_delta_time_ms()
{
	clock_t delta;
	
	timestamp = millis();
	delta = timestamp - timestamp_prev;
	timestamp_prev = timestamp;
	
	return delta;
}
//#define TBMS_DEBUG
#define TBOX_BATTERY_TOTAL_CAPACITY 4000
#define TBOX_BATTERY_TIMELEFT_WATT_THRESHOLD 10
#define TBOX_CURRENT_METER_PIN 34
#define TBOX_CONTACTOR_PIN 25
#include "contactor.h"
#include "counter.h"

#include "custom.h"
#include "tesla_bms_driver.h"
#include "current_meter_driver.h"

struct tbox_filesystem    filesystem;
struct tbox_contactor     contactor;
struct tbox_counter       counter;
struct tbox_current_meter meter;

void setup()
{
	String LVGL_Arduino = "Hello Arduino! ";
	LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

	Serial.begin( 115200 );
	Serial.println( LVGL_Arduino );

	lv_init();

	/*Set a tick source so that LVGL will know how much time elapsed. */
	lv_tick_set_cb(millis);

	/* register print function for debugging */
#if LV_USE_LOG != 0
	lv_log_register_print_cb( my_print );
#endif

#if LV_USE_TFT_ESPI
	/*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
	disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
#else
	/*Else create a display yourself*/
	disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
	lv_display_set_flush_cb(disp, my_disp_flush);
	lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

	/*Initialize the (dummy) input device driver*/
	lv_indev_t * indev = lv_indev_create();
	lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
	lv_indev_set_read_cb(indev, my_touchpad_read);

	/* Create a simple label
	 * ---------------------
	 lv_obj_t *label = lv_label_create( lv_scr_act() );
	 lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
	 lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

	 * Try an example. See all the examples
	 *  - Online: https://docs.lvgl.io/master/examples.html
	 *  - Source codes: https://github.com/lvgl/lvgl/tree/master/examples
	 * ----------------------------------------------------------------

	 lv_example_btn_1();

	 * Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS
	 * -------------------------------------------------------------------------------------------

	 lv_demo_widgets();
	 */

	//lv_demo_widgets();
	build_user_interface();
	
	/* lv_obj_t *label = lv_label_create( lv_scr_act() );
	lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
	lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 ); */

	//delay(2000);
	tesla_bms_driver_init();
	
	//CONTACTOR
	tbox_contactor_init(&contactor);
	pinMode(TBOX_CONTACTOR_PIN, OUTPUT);
	digitalWrite(TBOX_CONTACTOR_PIN, LOW);
	//CURRENT
	//pinMode(TBOX_CURRENT_METER_PIN, INPUT);

	tbox_current_meter_driver_init(&meter);

	tbox_counter_init(&counter);
	tbox_counter_set_total_capacity_wh(&counter,
					   TBOX_BATTERY_TOTAL_CAPACITY);
	tbox_counter_set_remaining_capacity_wh(&counter, TBOX_BATTERY_TOTAL_CAPACITY / 2);

	tbox_filesystem_init(&filesystem, &counter);
	tbox_filesystem_load(&filesystem);

	//tbox_web_server_init();
	
	Serial.println( "Setup done" );
}

void loop()
{	
	clock_t delta = get_delta_time_ms();

	#ifdef TBMS_DEBUG
	tb.tb.io.timeout = 5000; //Set bigger timeout for io
	#else
	tb.io.timeout = 5000; //Set bigger timeout for io
	#endif

	tesla_bms_driver_update(delta); //TODO suspend other tasks, while busy

	tbox_current_meter_driver_update(&meter, delta);

	float minv = 999;
	float maxv = 0;

	static float current = tbox_current_meter_get_current_A(&meter);

	for (int i = 0; i < 6; i++) {
		float v = tbms_get_module_cell_voltage(&tb, 0, i);
		tbms_ui_set_cell(i, v);
	
		if (v < minv)
			minv = v;
	
		if (v > maxv)
			maxv = v;
	}
	
	float power = current * tbms_get_module_voltage(&tb, 0);
	
	//CONTACTOR MANAGMENT:
	//TODO method instead of !tb.modules[0].exist;
	bool has_faults = !tbms_is_ready(&tb) ||
			  !tbox_current_meter_is_safe_to_use(&meter);

	tbox_contactor_ack_voltage(&contactor, minv, maxv);
	tbox_contactor_ack_current(&contactor, current);
	tbox_contactor_ack_faults(&contactor, has_faults);
	
	//SAVE FAULTS STATES
	static bool first_startup = false;
	static uint8_t tbms_saved_faults;
	static uint8_t tbms_saved_cov_faults;
	static uint8_t tbms_saved_cuv_faults;
	static bool tbms_saved_unsafe_to_use;
	static bool tbms_saved_has_faults;
	
	// Set fault flags (for ui) permamently if error occur
	// Othervise only update status before first startup
	if (has_faults || !first_startup) {
		tbms_saved_faults = tb.modules[0].faults;
		tbms_saved_cov_faults = tb.modules[0].cov_faults;
		tbms_saved_cuv_faults = tb.modules[0].cuv_faults;
		tbms_saved_unsafe_to_use = !tbox_current_meter_is_safe_to_use(&meter);
		tbms_saved_has_faults = has_faults;
	}
	//SAVE FAULTS STATES END

	static bool contactor_closed = false;
	static clock_t auto_calibration_timer = 0;
	auto_calibration_timer += delta;
	
	//TODO more temps
	tbox_contactor_ack_temp(&contactor, tbms_get_module_temp1(&tb, 0));
	
	enum tbox_contactor_event cont_ev =
		tbox_contactor_update(&contactor, delta);

	if (cont_ev == TBOX_CONTACTOR_EVENT_OPEN) {
		//printf("CONTACTOR OPEN EVENT\n");
		digitalWrite(TBOX_CONTACTOR_PIN, LOW);
		contactor_closed = false;
		auto_calibration_timer = 0;
	} else if (cont_ev == TBOX_CONTACTOR_EVENT_CLOSE) {
		//printf("CONTACTOR CLOSE EVENT\n");
		digitalWrite(TBOX_CONTACTOR_PIN, HIGH);
		contactor_closed = true;
		first_startup = true;
	}
	
	//COUNTER:
	if (tbox_counter_update(&counter, delta) ==
	    TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT &&
	    tbox_contactor_get_event(&contactor) ==
	    TBOX_CONTACTOR_EVENT_CLOSE && !has_faults)
	{	
		tbox_counter_ack_max_cell(&counter, maxv);
		tbox_counter_ack_voltage_and_current(&counter,
			tbms_get_module_voltage(&tb, 0), current);
		//printf("TBOX_COUNTER_QUERY_VOLTAGE_AND_CURRENT\n");
		tbox_filesystem_commit(&filesystem);
	}
	
	tbms_ui_set_fet_status(tbox_contactor_get_event(&contactor) ==
			       TBOX_CONTACTOR_EVENT_CLOSE);

	//FILESYSTEM update
	tbox_filesystem_update(&filesystem, delta);

	//WEB (4 min for web update process)
	/* static clock_t web_idle_time = 0;
	web_idle_time += delta;
	static bool web_enabled = true;
	if (web_idle_time >= 60000 * 4 && web_enabled) {
		WiFi.disconnect(true);
		WiFi.mode(WIFI_OFF);
		web_enabled = false;
	} else {
		tbox_web_server_update();
	}*/

	//UI and other tasks (update less time to let other tasks run smoothly)
	static clock_t lv_task_handler_update_interval = 0;
	static clock_t ui_update_timer = 0;
	lv_task_handler_update_interval += delta;
	ui_update_timer += delta;

	if (lv_task_handler_update_interval < 250)
		return;
	
	lv_task_handler_update_interval = 0;

	lv_task_handler(); /* let the GUI do its work */
	//delay(5); /* let this time pass */
	
	//UI update interval
	if (ui_update_timer < 1000)
		return;
	
	ui_update_timer = 0;
	
	tbms_ui_set_temp(tbms_get_module_temp1(&tb, 0));
	tbms_ui_set_voltage(tbms_get_module_voltage(&tb, 0));
	tbms_ui_set_current(current);
	tbms_ui_set_power(power);
	
	//printf("temp: %f, has_faults: %s\n",tbms_get_module_temp1(&tb, 0), has_faults ? "true" : "false");
	
	//TODO REFACTOR
	tbms_ui_set_status(tbms_saved_faults, tbms_saved_cov_faults,
			   tbms_saved_cuv_faults, tbms_saved_unsafe_to_use,
			   tbms_saved_has_faults);
	
	tbms_ui_set_min_cell_voltage(minv);
	tbms_ui_set_max_cell_voltage(maxv);
	tbms_ui_set_dif_cell_voltage(maxv - minv);
	
	tbms_ui_set_capacity_wh(
		tbox_counter_get_remaining_capacity_wh(&counter),
		tbox_counter_get_total_capacity_wh(&counter));
	tbms_ui_set_soc(tbox_counter_get_soc_normalized(&counter) * 100);
	tbms_ui_set_cycles(tbox_counter_get_cycles(&counter));

	// Charge / discharge timeleft (seconds)
	static float timeleft_seconds = 0;
        float alpha2 = 0.1; // коефіцієнт згладжування, де 0 < alpha <= 1
	
	if (power < -TBOX_BATTERY_TIMELEFT_WATT_THRESHOLD) {
		float discharge_time_seconds = 
			(tbox_counter_get_remaining_capacity_wh(&counter) /
			(-power)) * 3600;
		
		timeleft_seconds = alpha2  * discharge_time_seconds +
			      (1 - alpha2) * timeleft_seconds;
		
		tbms_ui_set_timeleft_seconds(timeleft_seconds);
	} else if (power > TBOX_BATTERY_TIMELEFT_WATT_THRESHOLD) {
		float charge_time_seconds =
			((tbox_counter_get_total_capacity_wh(&counter) -
			  tbox_counter_get_remaining_capacity_wh(&counter)) /
			power) * 3600;

		timeleft_seconds = alpha2  * charge_time_seconds +
			      (1 - alpha2) * timeleft_seconds;

		tbms_ui_set_timeleft_seconds(timeleft_seconds);
	} else {
		timeleft_seconds = 0;
		tbms_ui_set_timeleft_seconds(360000);
	}

	tbox_contactor_ack_ready(&contactor,
				 tbox_current_meter_is_safe_to_use(&meter));
}
