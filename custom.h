static lv_obj_t *temp_bar;
static lv_obj_t *power_bar;

static void temp_set(int32_t val)
{
	lv_bar_set_value(temp_bar, val, LV_ANIM_OFF);
}

static void power_set(int32_t val)
{
	lv_bar_set_value(power_bar, val, LV_ANIM_OFF);
}

static lv_style_t style_indic;
void style_indic_init()
{	
	lv_style_init(&style_indic);
	lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
	lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);
}

void temp_bar_create(lv_obj_t *parent)
{
	lv_obj_t *layout = lv_obj_create(parent);
	lv_obj_set_size(layout, 60, LV_PCT(90)); 
	lv_obj_set_style_bg_color(layout, lv_color_hex(0x001100), 0);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	//lv_obj_set_scrollbar_mode(layout, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_pad_all(layout, 5, 0);
	lv_obj_clear_flag(layout, LV_OBJ_FLAG_SCROLLABLE);

	//LABEL
	lv_obj_t *label = lv_label_create(layout);
	lv_obj_set_style_text_color(layout, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label, "Temp");
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);

	//BAR
	lv_obj_t *bar = lv_bar_create(layout);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_set_width(bar, 20);
	lv_bar_set_range(bar, -20, 70);
	//lv_bar_set_orientation(bar, LV_BAR_ORIENTATION_VERTICAL);
	lv_obj_set_flex_grow(bar, 1);

	//LABEL2
	lv_obj_t *label2 = lv_label_create(layout);
	lv_obj_set_style_text_color(layout, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label2, "21 °C");
	lv_obj_set_style_text_align(label2, LV_ALIGN_CENTER, 0);

	temp_bar = bar;
}

void central_gauge_create(lv_obj_t *parent)
{
	lv_obj_t *layout = lv_obj_create(parent);
	lv_obj_set_size(layout, LV_SIZE_CONTENT, LV_PCT(90)); 
	lv_obj_set_style_bg_color(layout, lv_color_hex(0x001100), 0);
	lv_obj_set_style_pad_all(layout, 5, 0);
	//lv_obj_set_flex_grow(layout, 1);

	lv_obj_t *layout_round = lv_obj_create(layout);
	lv_obj_set_style_radius(layout_round, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_size(layout_round, LV_SIZE_CONTENT, LV_SIZE_CONTENT); 
	lv_obj_set_style_bg_color(layout_round, lv_color_hex(0x001100), 0);
	lv_obj_set_style_pad_all(layout_round, 5, 0);
	//lv_obj_set_flex_grow(layout, 1);

	/*Create an Arc*/
	lv_obj_t * arc = lv_arc_create(layout_round);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_size(arc, 150, 150);
	lv_arc_set_rotation(arc, 135);
	lv_arc_set_bg_angles(arc, 0, 270);
	lv_arc_set_value(arc, 10);
	lv_obj_center(arc);

	//LABEL
	lv_obj_t *label = lv_label_create(layout_round);
	lv_obj_set_style_text_color(layout_round, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label, "88%");
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_align(label, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);  /*Set a larger font*/

	//LABEL2
	lv_obj_t *label2 = lv_label_create(layout);
	lv_obj_set_style_text_color(layout, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label2, "1h 5m");
	lv_obj_set_style_text_align(label2, LV_ALIGN_CENTER, 0);
	lv_obj_set_style_align(label2, LV_ALIGN_BOTTOM_MID, 0);
	lv_obj_set_style_text_font(label2, &lv_font_montserrat_28, 0);  /*Set a larger font*/
}

void power_bar_create(lv_obj_t *parent)
{
	lv_obj_t *layout = lv_obj_create(parent);
	lv_obj_set_size(layout, 60, LV_PCT(90)); 
	lv_obj_set_style_bg_color(layout, lv_color_hex(0x001100), 0);
	lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	//lv_obj_set_scrollbar_mode(layout, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_pad_all(layout, 5, 0);
	lv_obj_clear_flag(layout, LV_OBJ_FLAG_SCROLLABLE);
	//lv_obj_set_scroll_snap_x(layout, LV_SCROLL_SNAP_END);

	//LABEL
	lv_obj_t *label = lv_label_create(layout);
	lv_obj_set_style_text_color(layout, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label, "Power");
	lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, 0);

	//BAR
	lv_obj_t *bar = lv_bar_create(layout);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_set_width(bar, 20);
	lv_bar_set_range(bar, 0, 200);
	//lv_bar_set_orientation(bar, LV_BAR_ORIENTATION_VERTICAL);
	lv_obj_set_flex_grow(bar, 1);
	
	//LABEL2
	lv_obj_t *label2 = lv_label_create(layout);
	lv_obj_set_style_text_color(layout, lv_color_hex(0xffffff), 0);
	lv_label_set_text(label2, "33 A");
	lv_obj_set_style_text_align(label2, LV_ALIGN_CENTER, 0);

	power_bar = bar;
}

void build_user_interface(void)
{
	style_indic_init();
	
	//Фон
	lv_obj_t *cont = lv_screen_active();
	lv_obj_set_style_bg_color(cont, lv_color_hex(0x1a1a1a), 0);          // Початковий колір (темно-сірий)
	//lv_obj_set_style_bg_grad_color(cont, lv_color_hex(0x000000), 0);     // Кінцевий колір (чорний)
	//lv_obj_set_style_bg_grad_dir(cont, LV_GRAD_DIR_VER, 0);              // Вертикальний напрямок градієнта
	//lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);                      // Повна непрозорість фону
	lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

	//Температура    
	temp_bar_create(cont);

	//Центр
	central_gauge_create(cont);

	//Потужність
	power_bar_create(cont);

	//Інше
	temp_set(33);
	power_set(150);
}
