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
#define TBOX_CURRENT_METER_PIN 34
#define TBOX_CONTACTOR_PIN 25
#include "contactor.h"

#include "custom.h"
#include "tesla_bms_driver.h"

struct tbox_contactor contactor;

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
	pinMode(TBOX_CURRENT_METER_PIN, INPUT);

	Serial.println( "Setup done" );
}

void loop()
{	
	clock_t delta = get_delta_time_ms();

	tb.io.timeout = 1000; //Set bigger timeout for io
	tesla_bms_driver_update(delta);

	float minv = 999;
	float maxv = 0;
	static float curr = 0;
	static float current_offset = 0;
	static float current_mult   = 1.1194029850746268656716417910448;
	float calibrated_current = (curr - current_offset) * current_mult;

	for (int i = 0; i < 6; i++) {
		float v = tbms_get_module_cell_voltage(&tb, 0, i);
		tbms_ui_set_cell(i, v);
	
		if (v < minv)
			minv = v;
	
		if (v > maxv)
			maxv = v;
	}
	
	//CONTACTOR MANAGMENT:
	//TODO method instead of !tb.modules[0].exist;
	bool has_faults = !tbms_is_ready(&tb);// || abs(current_offset) > 10;

	tbox_contactor_ack_voltage(&contactor, minv, maxv);
	tbox_contactor_ack_current(&contactor, calibrated_current);
	tbox_contactor_ack_faults(&contactor, has_faults);

	static bool contactor_closed = false;
	static clock_t auto_calibration_delay = 0;
	auto_calibration_delay += delta;
	
	//TODO more temps
	tbox_contactor_ack_temp(&contactor, tbms_get_module_temp1(&tb, 0));
	
	enum tbox_contactor_event cont_ev =
		tbox_contactor_update(&contactor, delta);

	if (cont_ev == TBOX_CONTACTOR_EVENT_OPEN) {
		printf("CONTACTOR OPEN EVENT\n");
		digitalWrite(TBOX_CONTACTOR_PIN, LOW);
		contactor_closed = false;
		auto_calibration_delay = 0;
	} else if (cont_ev == TBOX_CONTACTOR_EVENT_CLOSE) {
		printf("CONTACTOR CLOSE EVENT\n");
		digitalWrite(TBOX_CONTACTOR_PIN, HIGH);
		contactor_closed = true;
	}

	//UI (update less time to let other tasks run smoothly)
	static clock_t ui_update_timer = 0;
	ui_update_timer += delta;

	if (ui_update_timer < 100)
		return;
	
	ui_update_timer = 0;

	lv_task_handler(); /* let the GUI do its work */
	//delay(5); /* let this time pass */
	
	tbms_ui_set_temp(tbms_get_module_temp1(&tb, 0));
	tbms_ui_set_voltage(tbms_get_module_voltage(&tb, 0));
	tbms_ui_set_current(calibrated_current);
	tbms_ui_set_power(calibrated_current *
			  tbms_get_module_voltage(&tb, 0));
	
	tbms_ui_set_min_cell_voltage(minv);
	tbms_ui_set_max_cell_voltage(maxv);
	tbms_ui_set_dif_cell_voltage(maxv - minv);

	//6.7, 7.5
	float alpha = 0.05; // коефіцієнт згладжування, де 0 < alpha <= 1
	curr = alpha * ((analogRead(TBOX_CURRENT_METER_PIN) - 2048) / 10.0) +
	       (1 - alpha) * curr;
	       
	//PERFORM CURRENT AUTOCALIBRATION
	if (!contactor_closed)
	{
		current_offset = curr;
		auto_calibration_delay = 0;
	}
}
