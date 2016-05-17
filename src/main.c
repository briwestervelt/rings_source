#include <pebble.h>

#define KEY_COLOR_SETTING 0
#define KEY_BACKGROUND_COLOR 1
#define KEY_FOREGROUND_COLOR 2
#define KEY_TOP_LINE_SETTING 3
#define KEY_BOTTOM_LINE_SETTING 4
#define KEY_CENTER_LINE_SETTING 5
#define KEY_BLUETOOTH_VIBES 6

#define SELECTED_COLORS 0
#define TRUE_RANDOM 1
#define DARK 2
#define LIGHT 3
#define HOT 4
#define COLD 5

#define DIGITAL 0
#define MONTH 1
#define DATE 2
#define WEEKDAY 3
#define MONTH_DATE 4
#define DATE_MONTH 5
#define WEEKDAY_DATE 6
#define STEPS 7
#define METERS 8
#define FEET 9
#define CALORIES 10

#define BATTERY 0
#define CONSTANT 1
#define NONE 2

#define gap_width          1  //pixels
#define line_one_offset    17 //pixels (for day of week)
#define line_two_offset    2  //pixels (for day of month)

#define num_dark_colors 8
#define num_light_colors 6
#define num_hot_colors 6
#define num_cold_colors 6

static Window *main_window;
static Layer *ring_layer, *battery_layer;
static TextLayer *date_line_one_layer, *date_line_two_layer;
static uint8_t battery_level;

static GColor background_color, foreground_color, bg_color_settings, fg_color_settings;
static bool bluetooth_vibes_bool;
static char color_setting[16], line_one_setting[16], line_two_setting[16], center_line_setting[16];

const uint8_t dark_colors[num_dark_colors] = {
  GColorOxfordBlueARGB8,
  GColorBlueARGB8,
  GColorDarkGreenARGB8,
  GColorArmyGreenARGB8,
  GColorBulgarianRoseARGB8,
  GColorDarkCandyAppleRedARGB8,
  GColorRedARGB8,
  GColorPurpleARGB8
};

const uint8_t light_colors[num_light_colors] = {
  GColorChromeYellowARGB8,
  GColorGreenARGB8,
  GColorPictonBlueARGB8,
  GColorMagentaARGB8,
  GColorYellowARGB8,
  GColorBabyBlueEyesARGB8
};

const uint8_t hot_colors[num_hot_colors] = {
  GColorRedARGB8,
  GColorYellowARGB8,
  GColorOrangeARGB8,
  GColorChromeYellowARGB8,
  GColorRajahARGB8,
  GColorFashionMagentaARGB8
};

const uint8_t cold_colors[num_cold_colors] = {
  GColorBlueARGB8,
  GColorDarkGreenARGB8,
  GColorPurpleARGB8,
  GColorIslamicGreenARGB8,
  GColorCyanARGB8,
  GColorScreaminGreenARGB8
};

static void set_colors(void) {
  int random_picker;  //for picking random colors from variable length lists
  
    //user settings
    if(!strcmp(color_setting, "selectedColors")){
      background_color = bg_color_settings;
      foreground_color = fg_color_settings;
    }
    
    //true random
    else if(!strcmp(color_setting, "trueRandom")){ 
      background_color = (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000) };
      foreground_color = gcolor_legible_over(background_color);
    }
    
    //dark
    else if(!strcmp(color_setting, "dark")){
      random_picker = rand() % num_dark_colors;
      background_color = (GColor)(dark_colors[random_picker]);
      foreground_color = GColorWhite;
    }
  
    //light
    else if(!strcmp(color_setting, "light")){ 
      random_picker = rand() % num_light_colors;
      background_color = (GColor)(light_colors[random_picker]);
      foreground_color = GColorBlack;
    }
    
    //hot
    else if(!strcmp(color_setting, "hot")){
      random_picker = rand() % num_hot_colors;
      background_color = (GColor)(hot_colors[random_picker]);
      foreground_color = gcolor_legible_over(background_color);
    }
    
    //cold
    else if(!strcmp(color_setting, "cold")){
      random_picker = rand() % num_cold_colors;
      background_color = (GColor)(cold_colors[random_picker]);
      foreground_color = gcolor_legible_over(background_color);
    }
    
    else{
      background_color = GColorBlack;
      foreground_color = GColorWhite;
    }
  
}

/*
static GColor random_color(void){
  //GColors take the form 0b11xxxxxx
  //I kind of stole this from a post by /u/chaos750 on /r/pebbledevelopers
  //thanks man!
   return (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000) };
}

static void set_colors(void){
  background_color = (random_color_bool) ? random_color() : bg_color_settings;
  foreground_color = (random_color_bool) ? gcolor_legible_over(background_color) : fg_color_settings;
}
*/

static void update_colors(void){
  set_colors();

  //foreground
  layer_mark_dirty(battery_layer);
  text_layer_set_text_color(date_line_one_layer, foreground_color);
  text_layer_set_text_color(date_line_two_layer, foreground_color);
  
  //background
  window_set_background_color(main_window, background_color);
}

static void update_lines(char* setting, int layer){
  
  static char text_buffer_one[16];
  static char text_buffer_two[16];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  static char text_buffer[16];
  
    //time
    if(!strcmp(setting, "digitalTime")){
      strftime(text_buffer, sizeof(text_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
    }
  
    //month
    else if(!strcmp(setting, "month")){
      strftime(text_buffer, sizeof(text_buffer), "%b", t);
    }
    
    //day of month
    else if(!strcmp(setting, "date")){
      strftime(text_buffer, sizeof(text_buffer), "%e", t);
    }
  
    //weekday
    else if(!strcmp(setting, "weekday")){
      strftime(text_buffer, sizeof(text_buffer), "%a", t);
    }
      
    //month/date
    else if(!strcmp(setting, "monthDay")){
      strftime(text_buffer, sizeof(text_buffer), "%m/%e", t);
    }
    
    //date/month
    else if(!strcmp(setting, "dayMonth")){
      strftime(text_buffer, sizeof(text_buffer), "%e/%m", t);
    }
   
    //weekday, date
    else if(!strcmp(setting, "weekdayDate")){
      strftime(text_buffer, sizeof(text_buffer), "%a, %e" , t);
    }
    
    //steps
    else if(!strcmp(setting, "steps")){
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    }
            
    //meters
    else if(!strcmp(setting, "meters")){
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricWalkedDistanceMeters));
    }
    
    //feet
    else if(!strcmp(setting, "feet")){
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)((3.28084*(int)health_service_sum_today(HealthMetricWalkedDistanceMeters))));
    }
    
    //calories
    else if(!strcmp(setting, "calories")){
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricActiveKCalories));
    }
            
    else{
      strcpy(text_buffer, "error");
    } 

  strcpy(layer ? text_buffer_two : text_buffer_one, text_buffer);
  text_layer_set_text(layer ? date_line_two_layer : date_line_one_layer, layer ? text_buffer_two : text_buffer_one);
}

static void ring_update_proc(Layer *layer, GContext *ctx){
  GRect outer_bounds = layer_get_bounds(layer);
  GRect inner_bounds = grect_inset(outer_bounds, GEdgeInsets(outer_bounds.size.w/6));
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  int minute = t->tm_min;
  int hour = t->tm_hour;
  
  //corrects for 12 hour format
  if(hour >= 12) hour -= 12;
  
  int minute_angle = 360 * (minute / 60.0);
  //int hour_angle = 360 * ((hour / 12.0) + (0.1 * (minute / 60.0)));
  int hour_angle = 360 * (( hour + (minute / 60.0)) / 12.0); //better algorithm
  
  if((color_setting != 0) && (minute == 0))
    update_colors();
  
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, foreground_color);
  
  graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(minute_angle));
  graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(hour_angle));
    
}

static void battery_callback(BatteryChargeState state){
  battery_level = state.charge_percent;
  layer_mark_dirty(battery_layer);
}

static void bluetooth_callback(bool connected){
  if((!connected) && (bluetooth_vibes_bool)){
    vibes_double_pulse();
  }
}

static void battery_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  int half_bar_length = 0;
  bool draw_line;
  
    if(!strcmp(center_line_setting, "battery")){ 
      half_bar_length = bounds.size.w/6 * (battery_level / 100.0);
      draw_line = true;
    }

    else if(!strcmp(center_line_setting, "constant")){ 
      half_bar_length = bounds.size.w/6;  
      draw_line = true;
    }
    
    else if(!strcmp(center_line_setting, "none")){
      draw_line = false;
    }
  
    else{
      draw_line = false;
    }
  
  if(draw_line){
    GPoint left_point =  GPoint(center.x - half_bar_length, center.y);
    GPoint right_point = GPoint(center.x + half_bar_length, center.y);
  
    graphics_context_set_stroke_color(ctx, foreground_color);
    graphics_draw_line(ctx, left_point, right_point);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_lines(line_one_setting, 0);
  update_lines(line_two_setting, 1);
  layer_mark_dirty(ring_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context){
  Tuple *color_setting_t = dict_find(iter, KEY_COLOR_SETTING);
  Tuple *background_color_t = dict_find(iter, KEY_BACKGROUND_COLOR);
  Tuple *foreground_color_t = dict_find(iter, KEY_FOREGROUND_COLOR);
  Tuple *top_line_t = dict_find(iter, KEY_TOP_LINE_SETTING);
  Tuple *bottom_line_t = dict_find(iter, KEY_BOTTOM_LINE_SETTING);
  Tuple *center_line_t = dict_find(iter, KEY_CENTER_LINE_SETTING);
  Tuple *bluetooth_vibes_t = dict_find(iter, KEY_BLUETOOTH_VIBES);
  
  if(color_setting_t){
    char *buffer = color_setting_t->value->cstring;
    persist_write_string(KEY_COLOR_SETTING, buffer);
    strcpy(color_setting, buffer);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(background_color_t){
    int background_color_HEX = background_color_t->value->int32;
    persist_write_int(KEY_BACKGROUND_COLOR, background_color_HEX);
    bg_color_settings = GColorFromHEX(background_color_HEX);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(foreground_color_t){
    int foreground_color_HEX = foreground_color_t->value->int32;
    persist_write_int(KEY_FOREGROUND_COLOR, foreground_color_HEX);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(top_line_t){
    char *buffer = top_line_t->value->cstring;
    persist_write_string(KEY_TOP_LINE_SETTING, buffer);
    strcpy(line_one_setting, buffer);
    update_lines(line_one_setting, 0);
  }
  if(bottom_line_t){
    char *buffer = bottom_line_t->value->cstring;
    persist_write_string(KEY_BOTTOM_LINE_SETTING, buffer);
    strcpy(line_two_setting, buffer);
    update_lines(line_two_setting, 1);
  }
  if(center_line_t){
    char *buffer = center_line_t->value->cstring;
    persist_write_string(KEY_CENTER_LINE_SETTING, buffer);
    strcpy(center_line_setting, buffer);
    layer_mark_dirty(battery_layer);
  }
  if(bluetooth_vibes_t){
    bluetooth_vibes_bool = bluetooth_vibes_t->value->uint8;
    persist_write_bool(KEY_BLUETOOTH_VIBES, bluetooth_vibes_bool);
  }
}

static void window_load(Window *window){
  
  if(persist_exists(KEY_BACKGROUND_COLOR)){
    int background_color_HEX = persist_read_int(KEY_BACKGROUND_COLOR);
    bg_color_settings = GColorFromHEX(background_color_HEX);
  }
  else{
    bg_color_settings = GColorBlue;
    
  }
  if(persist_exists(KEY_FOREGROUND_COLOR)){
    int foreground_color_HEX = persist_read_int(KEY_FOREGROUND_COLOR);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);
  }
  else{
    fg_color_settings = GColorWhite;
  }
  
  if(persist_exists(KEY_COLOR_SETTING)) 
    persist_read_string(KEY_COLOR_SETTING, color_setting, sizeof(color_setting));
  else
    strcpy(color_setting, "dark");
  
  if(persist_exists(KEY_TOP_LINE_SETTING)) 
    persist_read_string(KEY_TOP_LINE_SETTING, line_one_setting, sizeof(line_one_setting));
  else
    strcpy(line_one_setting, "weekdayDate");
  
  if(persist_exists(KEY_BOTTOM_LINE_SETTING)) 
    persist_read_string(KEY_BOTTOM_LINE_SETTING, line_two_setting, sizeof(line_two_setting));
  else
    strcpy(line_two_setting, "steps");
  
  if(persist_exists(KEY_CENTER_LINE_SETTING)) 
    persist_read_string(KEY_CENTER_LINE_SETTING, center_line_setting, sizeof(center_line_setting));
  else
    strcpy(center_line_setting, "battery");
  
  bluetooth_vibes_bool = (persist_exists(KEY_BLUETOOTH_VIBES)) ? persist_read_bool(KEY_BLUETOOTH_VIBES) : true;//vibrate
  
  set_colors();
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  
  window_set_background_color(main_window, background_color);
  
  ring_layer = layer_create(bounds);
  layer_set_update_proc(ring_layer, ring_update_proc);
  layer_add_child(window_layer, ring_layer);
  
  date_line_one_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - line_one_offset,
    bounds.size.w/3,
    line_one_offset
  ));
  text_layer_set_text_color(date_line_one_layer, foreground_color);
  text_layer_set_background_color(date_line_one_layer, GColorClear);
  text_layer_set_font(date_line_one_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(date_line_one_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_line_one_layer));
  
  date_line_two_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - line_two_offset,
    bounds.size.w/3,
    bounds.size.w/6
  ));
  text_layer_set_text_color(date_line_two_layer, foreground_color);
  text_layer_set_background_color(date_line_two_layer, GColorClear);
  text_layer_set_font(date_line_two_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(date_line_two_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_line_two_layer));
  
  battery_layer = layer_create(bounds);
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  tick_handler(t, MINUTE_UNIT);
}

static void window_unload(Window *window){
  layer_destroy(battery_layer);
  layer_destroy(ring_layer);
  text_layer_destroy(date_line_one_layer);
  text_layer_destroy(date_line_two_layer);
}

static void init(void){
  srand(time(NULL));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  main_window = window_create();
  
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(main_window, true);
  
  battery_callback(battery_state_service_peek());
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 0);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "App is running %d", 0);
}

static void deinit(void){
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  window_destroy(main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}