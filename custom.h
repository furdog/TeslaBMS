#include <stdio.h>
#include <math.h>

bool detailed_ui = false;

/* GLOBALS & SETTERS */
static lv_obj_t *soc_arc;
static lv_obj_t *soc_label;
static lv_obj_t *timeleft_label;
static lv_obj_t *cell_bar[12];
static lv_obj_t *soc_bar;
static lv_obj_t *voltage_label;
static lv_obj_t *current_bar_label;
static lv_obj_t *current_label;
static lv_obj_t *current_bar;
static lv_obj_t *capacity_max_label;
static lv_obj_t *capacity_label;
static lv_obj_t *power_label;
static lv_obj_t *cycles_label;
static lv_obj_t *temp_bar;
static lv_obj_t *temp_label;
static lv_obj_t *fet_label;
static lv_obj_t *status_label;

static lv_obj_t *minvolt_label;
static lv_obj_t *maxvolt_label;
static lv_obj_t *difvolt_label;

static void tbms_ui_set_status(uint8_t stat, bool overvolt, bool undervolt,
			       bool meter_err, bool other_faults)
{
	if (!detailed_ui)
		return;

	static char buf[32];

	snprintf(buf, 32, "СТАТУС: %s %02x",
		 stat || overvolt || undervolt || meter_err || other_faults ?
			(meter_err ? "ПОМИЛКА INA" : "ПОМИЛКА") : "ДОБРЕ", stat);

	lv_label_set_text(status_label, buf);
}

static void tbms_ui_set_timeleft(int hours, int minutes)
{
	if (detailed_ui)
		return;

	if (hours < 99) {
		static char buf[16];
		snprintf(buf, 16, "%02d : %02d", hours, minutes);

		lv_label_set_text(timeleft_label, buf);
	} else {
		lv_label_set_text(timeleft_label, "??:??");
	}
}

static void tbms_ui_set_timeleft_seconds(int seconds)
{
	int hours = seconds / 3600;
	int minutes = (seconds % 3600) / 60;

	tbms_ui_set_timeleft(hours, minutes);
}

static void tbms_ui_set_voltage(float val)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "%2.1f", val);
	
	lv_label_set_text(voltage_label, buf);
}

static void tbms_ui_set_fet_status(bool enabled)
{
	if (!detailed_ui)
		return;
	
	lv_label_set_text(fet_label, enabled ? "УВІМКН" : "ВИМКН");
}

static void tbms_ui_set_current(float val)
{
	if (!detailed_ui) {/*
		if (val < 0) {
			lv_bar_set_start_value(current_bar, val, LV_ANIM_OFF);
			lv_bar_set_value(current_bar, 0, LV_ANIM_OFF);
		} else {
			lv_bar_set_start_value(current_bar, 0, LV_ANIM_OFF);
			lv_bar_set_value(current_bar, val, LV_ANIM_OFF);
		}*/

		if (abs(val) > 1)
			lv_label_set_text(current_bar_label, val < 0 ? "Розряд" : "Заряд");
		else
			lv_label_set_text(current_bar_label, "Струм");

		lv_bar_set_value(current_bar, abs(val), LV_ANIM_OFF);
	}

	char buf[16];
	snprintf(buf, 16, "%2.1f%s", val, detailed_ui ? "" : " A");
	
	lv_label_set_text(current_label, buf);
}

static void tbms_ui_set_power(float val)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "%2.1f", val);

	lv_label_set_text(power_label, buf);
}

static void tbms_ui_set_temp(float val)
{
	if (isnan(val))
		val = 0;

	if (!detailed_ui)
		lv_bar_set_value(temp_bar, (int)val, LV_ANIM_OFF);
	
	char buf[16];
	snprintf(buf, 16, "%i%s", (int)val, detailed_ui ? "" : "°C");
	lv_label_set_text(temp_label, buf);
}

static void labeled_bar_set(void * obj, float v, const char *units);
static void tbms_ui_set_soc(float value)
{
	if (detailed_ui) {
		labeled_bar_set(soc_bar, value, "%");
		return;
	}
	
	lv_arc_set_value(soc_arc, value);

	char buf[16];
	snprintf(buf, 16, "%2.0f%%", value);
	lv_label_set_text(soc_label, buf);
}

static void tbms_ui_set_capacity_wh(float current, float max)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "%2.1f", current);
	lv_label_set_text(capacity_label, buf);
	
	snprintf(buf, 16, "%2.0fwH", max);
	lv_label_set_text(capacity_max_label, buf);
}

static void tbms_ui_set_cycles(float cycles)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "%2.1f", cycles);
	lv_label_set_text(cycles_label, buf);
}


static void tbms_ui_set_cell(int cell, float voltage)
{
	if (!detailed_ui)
		return;

	labeled_bar_set(cell_bar[cell], voltage, "");
}

static void tbms_ui_set_min_cell_voltage(float voltage)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "мін %1.2fV", voltage);
	lv_label_set_text(minvolt_label, buf);
}

static void tbms_ui_set_max_cell_voltage(float voltage)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "макс %1.2fV", voltage);
	lv_label_set_text(maxvolt_label, buf);
}

static void tbms_ui_set_dif_cell_voltage(float voltage)
{
	if (!detailed_ui)
		return;

	char buf[16];
	snprintf(buf, 16, "різн %1.2fV", voltage);
	lv_label_set_text(difvolt_label, buf);
}

static void tbms_ui_set_default()
{
	//tbms_ui_set_timeleft(1, 5);

	//tbms_ui_set_voltage(50);
	//tbms_ui_set_current(100);
	//tbms_ui_set_power(5000);
	//tbms_ui_set_temp(21);
	
	//tbms_ui_set_capacity_ah(66.666, 76.844);
	//tbms_ui_set_cycles(150);

	//for (int i = 0; i < 6; i++)
	//	tbms_ui_set_cell(i, 3.7);

	//tbms_ui_set_soc(88);
	
	//tbms_ui_set_current(-150);
}

///////////////////////////////////////////////////////////////////////////////
/* STYLES */
static lv_style_t style_layout_inv;
static lv_style_t style_layout;
static lv_style_t style_volume;
static lv_style_t style_indic;
static lv_style_t style_temp_bar;
static lv_style_t style_miniscr;

void styles_init()
{
	lv_style_init(&style_layout_inv);
	lv_style_set_bg_opa(&style_layout_inv, LV_OPA_TRANSP);
	lv_style_set_border_width(&style_layout_inv, 0);
	lv_style_set_shadow_width(&style_layout_inv, 0);
	lv_style_set_pad_all(&style_layout_inv, 0);
	lv_style_set_pad_column(&style_layout_inv, 2);
	lv_style_set_text_color(&style_layout_inv, lv_color_hex(0xffffff));

	lv_style_init(&style_layout);
	lv_style_set_pad_all(&style_layout, 5);
	lv_style_set_bg_opa(&style_layout, LV_OPA_COVER);
	lv_style_set_bg_color(&style_layout, lv_color_hex(0x444444));
	lv_style_set_bg_grad_color(&style_layout, lv_color_hex(0x666666));
	lv_style_set_bg_grad_dir(&style_layout, LV_GRAD_DIR_VER);
	lv_style_set_shadow_width(&style_layout, 10);
	lv_style_set_shadow_ofs_x(&style_layout, 5);
	lv_style_set_shadow_ofs_y(&style_layout, 5);
	lv_style_set_shadow_color(&style_layout, lv_color_black());
	lv_style_set_border_color(&style_layout, lv_color_hex(0x444444));
	lv_style_set_pad_row(&style_layout, 2);
	lv_style_set_pad_column(&style_layout, 2);
	lv_style_set_text_color(&style_layout, lv_color_hex(0xffffff));

	lv_style_init(&style_volume);
	lv_style_set_pad_all(&style_volume, 3);
	lv_style_set_bg_grad_color(&style_volume, lv_color_hex(0x555555));
	lv_style_set_bg_color(&style_volume, lv_color_hex(0x777777));
	lv_style_set_bg_grad_dir(&style_volume, LV_GRAD_DIR_HOR);
	lv_style_set_border_opa(&style_volume, LV_OPA_TRANSP);
	lv_style_set_radius(&style_volume, 100);
	
	lv_style_init(&style_miniscr);
	lv_style_set_pad_all(&style_miniscr, 2);
	lv_style_set_bg_color(&style_miniscr, lv_color_hex(0x002200));
	lv_style_set_border_opa(&style_miniscr, LV_OPA_TRANSP);
	lv_style_set_radius(&style_miniscr, 100);

	lv_style_init(&style_indic);
	lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
	lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_YELLOW));
	lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

	lv_style_init(&style_temp_bar);
	lv_style_set_bg_opa(&style_temp_bar, LV_OPA_COVER);
	lv_style_set_bg_color(&style_temp_bar, lv_color_hex(0x440000));
	lv_style_set_bg_grad_color(&style_temp_bar, lv_color_hex(0x000000));
	lv_style_set_bg_grad_dir(&style_temp_bar, LV_GRAD_DIR_VER);
	lv_style_set_border_opa(&style_miniscr, LV_OPA_TRANSP);
	lv_style_set_pad_all(&style_temp_bar, 6);
	lv_style_set_radius(&style_temp_bar, 10);
}

/* OBJECTS */
lv_obj_t *layout_create_inv(lv_obj_t *parent, int w, int h)
{
	lv_obj_t *layout = lv_obj_create(parent);
	lv_obj_set_size(layout, w, h);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_clear_flag(layout, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_style(layout, &style_layout_inv, 0);

	return layout;
}

lv_obj_t *layout_create_inv_table(lv_obj_t *parent)
{
	lv_obj_t *layout = layout_create_inv(parent, LV_SIZE_CONTENT, LV_PCT(100));

	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(layout, 5, 0);
	lv_obj_set_style_pad_row(layout, 0, 0);
	lv_obj_set_style_pad_column(layout, 0, 0);
	lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_CENTER,
			      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	
	return layout;
}

lv_obj_t *layout_create(lv_obj_t *parent, int w, int h)
{
	lv_obj_t *layout = lv_obj_create(parent);
	lv_obj_set_size(layout, w, h);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_clear_flag(layout, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_style(layout, &style_layout, 0);

	return layout;
}

lv_obj_t *volume_create(lv_obj_t *parent, int w, int h)
{
	lv_obj_t *volume = lv_obj_create(parent);
	lv_obj_set_size(volume, w, h);
	lv_obj_set_flex_flow(volume, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(volume, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_clear_flag(volume, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_style(volume, &style_volume, 0);

	return volume;
}

lv_obj_t *label_create_simple(lv_obj_t *parent, const char *text)
{
	lv_obj_t *label = lv_label_create(parent);
	//lv_obj_set_style_text_color(parent, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label, text);
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_align(label, LV_ALIGN_CENTER, 0);
	//lv_obj_set_style_border_width(label, 0, 0);
	//lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
	return label;
}

lv_obj_t *label_create_simple_colored(lv_obj_t *parent, const char *text, uint32_t color)
{
	lv_obj_t *label = lv_label_create(parent);
	lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
	lv_label_set_text(label, text);
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_align(label, LV_ALIGN_CENTER, 0);
	//lv_obj_set_style_border_width(label, 0, 0);
	//lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
	return label;
}

lv_obj_t *label_create(lv_obj_t *parent, const char *text)
{
	lv_obj_t *volume = volume_create(parent, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_align(volume, LV_ALIGN_CENTER, 0);
	
	lv_obj_t *miniscr = lv_obj_create(volume);
	lv_obj_add_style(miniscr, &style_miniscr, 0);
	lv_obj_set_size(miniscr, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	
	lv_obj_t *label = lv_label_create(miniscr);
	lv_obj_set_style_text_color(label, lv_color_hex(0x00ff00), 0);
	lv_label_set_text(label, text);
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_align(label, LV_ALIGN_CENTER, 0);

	return label;
}

//percentage bar
//static void labeled_bar_set_after(lv_timer_t * timer)
static void labeled_bar_set_after(lv_obj_t *obj)
{
	//lv_obj_t *obj   = (lv_obj_t *)lv_timer_get_user_data(timer);
	lv_obj_t *label = (lv_obj_t *)lv_obj_get_child(obj, 0);

	bool vertical = lv_obj_get_height(obj) > lv_obj_get_width(obj) ? true : false;

	lv_area_t indic_area;
	lv_obj_get_coords(obj, &indic_area);

	lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
	lv_obj_set_style_bg_opa(label, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(label, 2, 0);

	int indic_actual_width  = lv_area_get_width(&indic_area)  * lv_bar_get_value(obj) / lv_bar_get_max_value(obj);
	//int indic_actual_height = lv_area_get_height(&indic_area) * lv_bar_get_value(obj) / lv_bar_get_max_value(obj);

	//lv_obj_move_foreground(label);
	
	if (vertical) {
		//int label_offset = indic_actual_height - 5;
		
		//if(lv_area_get_height(&indic_area) - indic_actual_height - 5 < lv_obj_get_width(label))
		//	label_offset -= lv_obj_get_width(label) + 10;

		lv_obj_set_style_transform_angle(label, 900*3, LV_PART_MAIN); // 900 = 90 degrees (in tenths of a degree)
		lv_obj_set_style_text_align(label, LV_ALIGN_BOTTOM_MID, 0);
		lv_obj_set_style_align(label, LV_ALIGN_BOTTOM_MID, 0);
		lv_obj_set_pos(label, 5, -lv_area_get_height(&indic_area) + lv_obj_get_width(label) * 2 + 5);
	} else {
		int label_offset = indic_actual_width - 5;
		
		if(lv_area_get_width(&indic_area) - indic_actual_width - 5 < lv_obj_get_width(label))
			label_offset -= lv_obj_get_width(label) + 10;

		lv_obj_set_style_text_align(label, LV_ALIGN_LEFT_MID, 0);
		lv_obj_set_style_align(label, LV_ALIGN_LEFT_MID, 0);
		lv_obj_set_pos(label, label_offset, 0);
	}
}

static void labeled_bar_set(void * obj, float v, const char *units)
{
	lv_bar_set_value((lv_obj_t *)obj, v * 1000, LV_ANIM_OFF);
	
	char buf[16];
	snprintf(buf, sizeof(buf), "%3.2f%s", v, units);
	lv_obj_t * label = lv_obj_get_child((lv_obj_t *)obj, 0);
	lv_label_set_text(label, buf);

	labeled_bar_set_after((lv_obj_t *)obj);
	//lv_timer_t * timer = lv_timer_create(labeled_bar_set_after, 100, obj);
	//lv_timer_set_repeat_count(timer, 2);
}

lv_obj_t *labeled_bar_create(lv_obj_t *parent, float min, float max)
{
	lv_obj_t *bar = lv_bar_create(parent);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_add_style(bar, &style_temp_bar, 0);
	lv_bar_set_range(bar, min * 1000, max * 1000);
	lv_obj_set_size(bar, LV_PCT(100), LV_PCT(100));
	lv_bar_set_value(bar, 0, LV_ANIM_OFF);

	lv_obj_t * label = lv_label_create(bar);
	lv_label_set_text(label, "0");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

	return bar;
}

///////////////////////////////////////////////////////////////////////////////

void temp_bar_create(lv_obj_t *parent)
{
	lv_obj_t *layout = layout_create(parent, 60, LV_PCT(95));

	label_create_simple(layout, "Темп");

	//BAR
	lv_obj_t *volume = volume_create(layout, 30, LV_PCT(80));
	lv_obj_t *bar = lv_bar_create(volume);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
	lv_obj_set_style_bg_grad_color(bar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
	lv_obj_add_style(bar, &style_temp_bar, 0);
	lv_obj_set_width(bar, 20);
	lv_bar_set_range(bar, -20, 70);
	//lv_bar_set_orientation(bar, LV_BAR_ORIENTATION_VERTICAL);
	lv_obj_set_flex_grow(bar, 1);

	temp_label = label_create(layout, "?? °C");

	temp_bar = bar;
}

void central_gauge_create(lv_obj_t *parent)
{
	lv_obj_t *layout = layout_create(parent, LV_SIZE_CONTENT, LV_PCT(100));
	lv_obj_set_flex_grow(layout, 2);

	lv_obj_t *volume = volume_create(layout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_t *layout_round = lv_obj_create(volume);
	lv_obj_set_style_radius(layout_round, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_size(layout_round, LV_SIZE_CONTENT, LV_SIZE_CONTENT); 
	lv_obj_set_style_bg_color(layout_round, lv_color_hex(0x001100), 0);
	lv_obj_set_style_pad_all(layout_round, 5, 0);
	//lv_obj_set_flex_grow(layout, 1);

	/*Create an Arc*/
	lv_obj_t * arc = lv_arc_create(layout_round);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_size(arc, 140, 140);
	lv_arc_set_rotation(arc, 135);
	lv_arc_set_bg_angles(arc, 0, 270);
	lv_arc_set_value(arc, 0);
	lv_obj_center(arc);
	lv_obj_set_style_border_color(layout_round, lv_color_hex(0x444444), 0);
	lv_obj_set_style_arc_color(arc, lv_color_hex(0x444444), 0);
	//lv_obj_add_style(arc, &style_indic, 0);

	soc_label = label_create_simple(layout_round, "??%");
	lv_obj_set_style_text_font(soc_label, &lv_font_montserrat_28, 0);
	lv_obj_set_style_text_color(layout_round, lv_color_hex(0x00ff00), 0);

	timeleft_label = label_create(layout, "?? : ??");
	lv_obj_set_style_text_font(timeleft_label, &lv_font_montserrat_28, 0);

	soc_arc = arc;
}

void current_bar_create(lv_obj_t *parent)
{
	lv_obj_t *layout = layout_create(parent, 60, LV_PCT(95));

	current_bar_label = label_create_simple(layout, "Струм");

	//BAR
	lv_obj_t *volume = volume_create(layout, 30, LV_PCT(80));
	lv_obj_t *bar = lv_bar_create(volume);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_add_style(bar, &style_temp_bar, 0);
	lv_obj_set_width(bar, 20);
	//lv_bar_set_range(bar, -200, 200);
	lv_bar_set_range(bar, 0, 150);
	//lv_bar_set_orientation(bar, LV_BAR_ORIENTATION_VERTICAL);
	lv_obj_set_flex_grow(bar, 1);

	//lv_bar_set_mode(bar, LV_BAR_MODE_RANGE);
	lv_bar_set_value(bar, 100, LV_ANIM_OFF);
	lv_bar_set_start_value(bar, 0, LV_ANIM_OFF);
	lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(bar, lv_color_hex(0x001100), 0);
	lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
	lv_obj_set_style_bg_grad_color(bar, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);

	/*Create style*/
	/*static lv_style_t style_line;
	lv_style_init(&style_line);
	lv_style_set_line_width(&style_line, 5);
	lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_line_rounded(&style_line, true);

	static lv_point_precise_t line_points[] = { {0, 0}, {20, 0} };
	
	lv_obj_t * line1;
	line1 = lv_line_create(bar);
	lv_line_set_points(line1, line_points, 2);     //Set the points
	lv_obj_add_style(line1, &style_line, 0);
	lv_obj_center(line1);
	*/

	current_label = label_create(layout, "?? A");

	current_bar = bar;
}

void build_main_interface(lv_obj_t *cont)
{	
	//Температура    
	temp_bar_create(cont);

	//Центр
	central_gauge_create(cont);

	//Потужність
	current_bar_create(cont);

	tbms_ui_set_default();
}

///////////////////////////////////////////////////////////////////////////////

void build_general_stats(lv_obj_t *parent)
{
	lv_obj_t *layout = layout_create(parent, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_style_pad_all(layout, 0, 0);
	lv_obj_set_flex_grow(layout, 2);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_ROW);
	
	//LEFT CONTENT LAYOUT
	lv_obj_t *lcont = layout_create_inv(layout, LV_PCT(50), LV_PCT(100));
	lv_obj_set_style_pad_all(lcont, 0, 0);
	lv_obj_set_flex_grow(lcont, 1);
	lv_obj_set_flex_flow(lcont, LV_FLEX_FLOW_ROW);

	lv_obj_set_flex_align(lcont, LV_FLEX_ALIGN_SPACE_EVENLY,
			      LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	
	lv_obj_t *lcont_UIPT0 = layout_create_inv_table(lcont);
	lv_obj_set_style_text_color(lcont_UIPT0, lv_color_hex(0xFF7777), 0);
	
	label_create_simple(lcont_UIPT0, "U");
	label_create_simple(lcont_UIPT0, "I");
	label_create_simple(lcont_UIPT0, "P");
	label_create_simple(lcont_UIPT0, "T");

	lv_obj_t *lcont_UIPT1 = layout_create_inv_table(lcont);
	lv_obj_set_style_text_color(lcont_UIPT1, lv_color_hex(0xFFFFFF), 0);
	
	voltage_label = label_create_simple(lcont_UIPT1, "??.?");
	current_label = label_create_simple(lcont_UIPT1, "??.?");
	power_label   = label_create_simple(lcont_UIPT1, "??.?");
	temp_label    = label_create_simple(lcont_UIPT1, "??");

	lv_obj_t *lcont_UIPT2 = layout_create_inv_table(lcont);
	lv_obj_set_style_text_color(lcont_UIPT2, lv_color_hex(0xFFFF77), 0);
	
	label_create_simple(lcont_UIPT2, " V");
	label_create_simple(lcont_UIPT2, " A");
	label_create_simple(lcont_UIPT2, " W");
	label_create_simple(lcont_UIPT2, "°C");

	//RIGHT CONTENT LAYOUT
	lv_obj_t *rcont = layout_create_inv(layout, LV_PCT(50), LV_PCT(100));
	lv_obj_set_style_pad_all(rcont, 0, -15);
	lv_obj_set_flex_grow(rcont, 2);
	lv_obj_set_flex_flow(rcont, LV_FLEX_FLOW_COLUMN);

	//RIGHT CONTENT LAYOUT UPPER PART
	lv_obj_t *rcont_up = layout_create_inv(rcont, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_pad_all(rcont_up, 0, 0);
	lv_obj_set_flex_grow(rcont_up, 1);
	lv_obj_set_flex_flow(rcont_up, LV_FLEX_FLOW_ROW);

	lv_obj_t *rcont_CC0 = layout_create_inv_table(rcont_up);
	lv_obj_set_style_text_color(rcont_CC0, lv_color_hex(0xFF7777), 0);
	label_create_simple(rcont_CC0, "ЄМН");
	label_create_simple(rcont_CC0, "ЦИК");
	
	lv_obj_t *rcont_CC1 = layout_create_inv_table(rcont_up);
	lv_obj_set_style_text_color(rcont_CC1, lv_color_hex(0xFFFF77), 0);
	capacity_label = label_create_simple(rcont_CC1, "????");
	cycles_label  = label_create_simple(rcont_CC1, "????");
	
	lv_obj_t *rcont_CC2 = layout_create_inv_table(rcont_up);
	lv_obj_set_style_text_color(rcont_CC2, lv_color_hex(0xFF7777), 0);
	label_create_simple(rcont_CC2, "/");
	label_create_simple(rcont_CC2, "FET");

	lv_obj_t *rcont_CC3 = layout_create_inv_table(rcont_up);
	lv_obj_set_style_text_color(rcont_CC3, lv_color_hex(0xFFFF77), 0);
	capacity_max_label = label_create_simple(rcont_CC3, "????");
	fet_label = label_create_simple(rcont_CC3, "??");
	
	//RIGHT CONTENT LAYOUT LOWER STATUS PART
	lv_obj_t *rcont_down = layout_create(rcont, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_pad_all(rcont_down, 0, 0);
	lv_obj_set_flex_grow(rcont_down, 1);
	lv_obj_set_style_bg_grad_color(rcont_down, lv_color_hex(0xAA4444), 0);
	status_label = label_create_simple(rcont_down, "СТАТУС ??");
}

void build_cell(lv_obj_t *parent, int cell_num)
{
	char buf[3];
	snprintf(buf, 3, "%i", cell_num + 1);

	lv_obj_t *layout = layout_create(parent, 20, LV_PCT(100));
	lv_obj_set_flex_grow(layout, 1);

	label_create_simple(layout, buf);

	//BAR
	lv_obj_t *volume = volume_create(layout, 15, LV_PCT(95));
	cell_bar[cell_num] = labeled_bar_create(volume, 2.7, 4.2);
	lv_obj_set_width(cell_bar[cell_num], 10);
	labeled_bar_set(cell_bar[cell_num], 0, "");
}

void build_cells(lv_obj_t *parent, int cell_num)
{
	lv_obj_t *layout = layout_create(parent, LV_PCT(100), 0);
	lv_obj_set_style_pad_all(layout, 0, 0);
	lv_obj_set_flex_grow(layout, 4);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);

	lv_obj_t *cell_stats = layout_create_inv(layout, LV_PCT(100), 0);
	lv_obj_set_style_pad_all(cell_stats, 0, 0);
	lv_obj_set_flex_grow(cell_stats, 1);
	lv_obj_set_flex_flow(cell_stats, LV_FLEX_FLOW_ROW);

	minvolt_label = label_create_simple(cell_stats, "мін ?.??V");
	maxvolt_label = label_create_simple(cell_stats, "макс ?.??V");
	difvolt_label = label_create_simple(cell_stats, "різн ?.??V");

	lv_obj_t *cell_voltages = layout_create_inv(layout, LV_PCT(100), 0);
	lv_obj_set_style_pad_all(cell_voltages, 0, 0);
	lv_obj_set_flex_grow(cell_voltages, 5);
	lv_obj_set_flex_flow(cell_voltages, LV_FLEX_FLOW_ROW);

	for (int i = 0; i < cell_num; i++)
		build_cell(cell_voltages, i);
}

void build_extra_interface(lv_obj_t *parent)
{
	lv_obj_t *layout;

	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(parent, 2, 0);
	lv_obj_set_style_pad_row(parent, 2, LV_PART_MAIN);
	
	build_general_stats(parent);
	build_cells(parent, 6);

	layout = layout_create(parent, LV_PCT(100), 0);
	lv_obj_set_style_pad_all(layout, 0, 0);
	lv_obj_set_flex_grow(layout, 1);

	//SOC BAR
	lv_obj_t *volume = volume_create(layout, LV_PCT(100), LV_PCT(100));
	soc_bar = labeled_bar_create(volume, 0, 100);
	labeled_bar_set(soc_bar, 0, "%");

	tbms_ui_set_default();
}

///////////////////////////////////////////////////////////////////////////////
static void screen_click_event_cb(lv_event_t * e);
lv_obj_t *create_screen_click_notifier(lv_obj_t *parent)
{
	//Screen click notifier
	lv_obj_t *screen_cover = lv_obj_create(parent);
	lv_obj_set_size(screen_cover, LV_HOR_RES, LV_VER_RES);
	lv_obj_clear_flag(screen_cover, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_clear_flag(screen_cover, LV_OBJ_FLAG_IGNORE_LAYOUT); // Ігнорування розташування
	lv_obj_add_flag(screen_cover, LV_OBJ_FLAG_FLOATING); 
	lv_obj_set_style_bg_opa(screen_cover, LV_OPA_TRANSP, 0);
	lv_obj_set_style_pad_all(screen_cover, 0, 0);
	lv_obj_set_style_border_width(screen_cover, 0, 0);
	lv_obj_add_event_cb(screen_cover, screen_click_event_cb, LV_EVENT_CLICKED, NULL);

	return screen_cover;
}

void build_user_interface(void);

/*static void screen_click_delayed_create(lv_timer_t * timer)
{
	lv_obj_t *cont = (lv_obj_t *)lv_timer_get_user_data(timer);
	create_screen_click_notifier(cont);
}*/

static void screen_click_event_cb(lv_event_t * e)
{
	detailed_ui = !detailed_ui;

	lv_obj_t *cont = lv_screen_active();

	lv_obj_clean(cont);
	
	build_user_interface();
}

void build_user_interface(void)
{
	//Фон
	lv_obj_t *cont = lv_screen_active();
	lv_obj_set_style_bg_color(cont, lv_color_hex(0x002200), 0);
	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER,
				    LV_FLEX_ALIGN_CENTER,
				    LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_all(cont, 5, 0);
	lv_obj_set_style_pad_column(cont, 5, LV_PART_MAIN);
	lv_obj_set_style_text_color(cont, lv_color_hex(0xffffff), 0);

	styles_init();

	if (!detailed_ui)
		build_main_interface(cont);
	else
		build_extra_interface(cont);
	
	create_screen_click_notifier(cont);
	//lv_timer_t * timer = lv_timer_create(screen_click_delayed_create, 1000, cont);
	//lv_timer_set_repeat_count(timer, 1);
}
