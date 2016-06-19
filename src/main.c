#include <pebble.h>

/*
Settings from the Clay framework come in as CStrings, but
are immediately converted to coded integer equivilents to save
processor power later, as well as memory space. The following
three blocks lay the foundation for this process
*/

//color settings
#define SELECTED_COLORS 0
#define TRUE_RANDOM 1
#define DARK 2
#define LIGHT 3
#define HOT 4
#define COLD 5

//complication settings
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

//center line settings
#define BATTERY 0
#define CONSTANT 1
#define NONE 2

//for modifying graphics 
#define gap_width               1  //pixels (the gap between the rings)
#define line_one_offset        21  //pixels (For font GOTHIC_18_BOLD)
#define line_one_offset_small  18  //pixels (For font GOTHIC_14_BOLD))
#define line_two_offset         3  //pixels (For font GOTHIC_18_BOLD)
#define line_two_offset_small   1  //pixels (For font GOTHIC_14_BOLD))

//number of random colors in each group
#define num_dark_colors 8
#define num_light_colors 6
#define num_hot_colors 6
#define num_cold_colors 6

//variables for app function
static Window *main_window;
static Layer *ring_layer, *battery_layer, *bt_icon_layer;
static TextLayer *line_one_layer, *line_two_layer;
static uint8_t battery_level;
static bool bt_connected;

//variables for configuration
static GColor background_color, foreground_color, bg_color_settings, fg_color_settings;
static bool bluetooth_vibes_bool, bluetooth_icon_bool;
static uint8_t color_setting, line_one_setting, line_two_setting, center_line_setting;

/*
The following blocks lay out the exact colors
in each random color configuration
*/

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
  /*
  This function sets the global variables background_color and foreground_color
  according to the user's specified color setting
  */
  int random_picker;  //for picking random colors from variable length lists
  
  switch(color_setting){
    case SELECTED_COLORS:
      background_color = bg_color_settings;
      foreground_color = fg_color_settings;
    break;
    case TRUE_RANDOM:
      background_color = (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000) };
      foreground_color = gcolor_legible_over(background_color);
    break;
    case DARK:
      random_picker = rand() % num_dark_colors;
      background_color = (GColor)(dark_colors[random_picker]);
      foreground_color = GColorWhite;
    break;
    case LIGHT:
      random_picker = rand() % num_light_colors;
      background_color = (GColor)(light_colors[random_picker]);
      foreground_color = GColorBlack;
    break;
    case HOT:
      random_picker = rand() % num_hot_colors;
      background_color = (GColor)(hot_colors[random_picker]);
      foreground_color = gcolor_legible_over(background_color);
    break;
    case COLD:
      random_picker = rand() % num_cold_colors;
      background_color = (GColor)(cold_colors[random_picker]);
      foreground_color = gcolor_legible_over(background_color);
    break;
    default:
      background_color = GColorBlack;
      foreground_color = GColorWhite;
    break;
  }
  
}

static void update_colors(void){
  /*
  This function marks the appropriate layers dirty so that the
  whole face is redrawn with new colors
  */
  set_colors();

  //foreground
  layer_mark_dirty(battery_layer);
  text_layer_set_text_color(line_one_layer, foreground_color);
  text_layer_set_text_color(line_two_layer, foreground_color);
  
  //background
  window_set_background_color(main_window, background_color);
}

static void update_lines(int setting, int layer){
  /*
  This function updates the text in either line one or line two.
  The setting input comes from the configuration page, and the
  layer input determines which layer. A value of 0 selects 
  line_one_layer and a value of 1 selects line_two_layer
  */
  
  static char text_buffer_one[16], text_buffer_two[16];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  char text_buffer[16];
  
  switch(setting){
    case DIGITAL:
      strftime(text_buffer, sizeof(text_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
    break;
    case MONTH:
      strftime(text_buffer, sizeof(text_buffer), "%b", t);
    break;
    case DATE:
      strftime(text_buffer, sizeof(text_buffer), "%e", t);
    break;
    case WEEKDAY:
      strftime(text_buffer, sizeof(text_buffer), "%a", t);
    break;
    case MONTH_DATE:
      strftime(text_buffer, sizeof(text_buffer), "%m/%e", t);
    break;
    case DATE_MONTH:
      strftime(text_buffer, sizeof(text_buffer), "%e/%m", t);
    break;
    case WEEKDAY_DATE:
      strftime(text_buffer, sizeof(text_buffer), "%a, %e" , t);
    break;
    case STEPS:
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    break;
    case METERS:
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricWalkedDistanceMeters));
    break;
    case FEET:
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)((3.28084*(int)health_service_sum_today(HealthMetricWalkedDistanceMeters))));
    break;
    case CALORIES:
      snprintf(text_buffer, sizeof(text_buffer), "%d", (int)health_service_sum_today(HealthMetricActiveKCalories));
    break;
    default:
      strcpy(text_buffer, "error");
    break;
  }

  strcpy(layer ? text_buffer_two : text_buffer_one, text_buffer);
  text_layer_set_text(layer ? line_two_layer : line_one_layer, layer ? text_buffer_two : text_buffer_one);
}

static void bt_icon_update_proc(Layer *layer, GContext *ctx){
  /*
  This update proc handles the bluetooth disconnect icon. If the user
  has specified in the settings that they would like to see the icon
  it will draw an icon on disconnect
  */
  if((!bt_connected) && (bluetooth_icon_bool)){
    GRect bounds = layer_get_bounds(layer);
    GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
    
    //background circle
    graphics_context_set_fill_color(ctx, foreground_color);
    graphics_fill_circle(ctx, center, bounds.size.w/6 - gap_width);
    
    //constructing bluetooth symbol
    GPoint top_left, top_center, top_right, bottom_left, bottom_right, bottom_center;
    uint8_t symbol_width = bounds.size.w * 0.06;
    top_center = GPoint(center.x, center.y - bounds.size.w/8);
    bottom_center = GPoint(center.x, center.y + bounds.size.w/8);
    top_left = GPoint(center.x - symbol_width, center.y - bounds.size.w/16);
    top_right = GPoint(center.x + symbol_width, center.y - bounds.size.w/16);
    bottom_left = GPoint(center.x - symbol_width, center.y + bounds.size.w/16);
    bottom_right = GPoint(center.x + symbol_width, center.y + bounds.size.w/16);
    
    graphics_context_set_stroke_color(ctx, background_color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, top_center, bottom_center);
    graphics_draw_line(ctx, top_center, top_right);
    graphics_draw_line(ctx, bottom_center, bottom_right);
    graphics_draw_line(ctx, top_left, bottom_right);
    graphics_draw_line(ctx, bottom_left, top_right);
  }
}

static void battery_update_proc(Layer *layer, GContext *ctx){
  /*
  This update draws the center line dependent on the user's
  choice, based on the global variable center_line_setting
  */
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  int half_bar_length = 0;
  bool draw_line;
  
  switch(center_line_setting){
    case BATTERY: 
      half_bar_length = bounds.size.w/6 * (battery_level / 100.0);
      draw_line = true;
    break;
    case CONSTANT: 
      half_bar_length = bounds.size.w/6;  
      draw_line = true;
    break;
    case NONE:
      draw_line = false;
    break;
    default:
      draw_line = false;
    break;
  }
  
  if(draw_line){
    GPoint left_point =  GPoint(center.x - half_bar_length, center.y);
    GPoint right_point = GPoint(center.x + half_bar_length, center.y);
  
    graphics_context_set_stroke_color(ctx, foreground_color);
    graphics_draw_line(ctx, left_point, right_point);
  }
}

static void ring_update_proc(Layer *layer, GContext *ctx){
  /*
  This update proc draws the rings which show the time
  */
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
  
  //to handle random color switches on the hour
  if((color_setting != 0) && (minute == 59))
    update_colors();
  
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, foreground_color);
  
  graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(minute_angle));
  graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(hour_angle));
    
}

static void battery_callback(BatteryChargeState state){
  battery_level = state.charge_percent;  //update global variable
  layer_mark_dirty(battery_layer);
}

static void bluetooth_callback(bool connected){
  if((!connected) && (bluetooth_vibes_bool)){
    vibes_double_pulse();
  }
  bt_connected = connected;  //update global variable
  layer_mark_dirty(bt_icon_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  //update complications
  update_lines(line_one_setting, 0);
  update_lines(line_two_setting, 1);
  
  //redraw time
  layer_mark_dirty(ring_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context){
  //Tuples pull the values from the settings page
  Tuple *color_setting_t = dict_find(iter, MESSAGE_KEY_colorSetting);
  Tuple *background_color_t = dict_find(iter, MESSAGE_KEY_backgroundColor);
  Tuple *foreground_color_t = dict_find(iter, MESSAGE_KEY_foregroundColor);
  Tuple *top_line_t = dict_find(iter, MESSAGE_KEY_topLineSetting);
  Tuple *bottom_line_t = dict_find(iter, MESSAGE_KEY_bottomLineSetting);
  Tuple *center_line_t = dict_find(iter, MESSAGE_KEY_centerLineSetting);
  Tuple *bluetooth_vibes_t = dict_find(iter, MESSAGE_KEY_bluetoothVibes);
  Tuple *bluetooth_icon_t = dict_find(iter, MESSAGE_KEY_bluetoothIcon);
  
  if(color_setting_t){
    char *buffer = color_setting_t->value->cstring;  //The value returned from the settings page is a cstring
    
    //The cstring is immediately converted to an integer
    if(!strcmp(buffer, "selectedColors"))
      color_setting = SELECTED_COLORS;
    else if(!strcmp(buffer, "trueRandom"))
       color_setting = TRUE_RANDOM;
    else if(!strcmp(buffer, "dark"))
       color_setting = DARK;
    else if(!strcmp(buffer, "light"))
       color_setting = LIGHT;
    else if(!strcmp(buffer, "hot"))
       color_setting = DARK;
    else if(!strcmp(buffer, "cold"))
       color_setting = COLD;
       
    persist_write_int(MESSAGE_KEY_colorSetting, color_setting);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(background_color_t){
    int background_color_HEX = background_color_t->value->int32;   //The value returned is an integer
    persist_write_int(MESSAGE_KEY_backgroundColor, background_color_HEX);
    bg_color_settings = GColorFromHEX(background_color_HEX);      //converted to a GColor and stored in a global variable
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(foreground_color_t){
    int foreground_color_HEX = foreground_color_t->value->int32;  //The value returned is an integer
    persist_write_int(MESSAGE_KEY_foregroundColor, foreground_color_HEX);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);      //converted to a GColor and stored in a global veriable
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(top_line_t){
    char *buffer = top_line_t->value->cstring;  //The value returned from the settings page is a cstring
    
    //The cstring is immediately converted to an integer
    if(!strcmp(buffer, "digitalTime"))
      line_one_setting = DIGITAL;
    else if(!strcmp(buffer, "month"))
      line_one_setting = MONTH;
    else if(!strcmp(buffer, "date"))
      line_one_setting = DATE;
    else if(!strcmp(buffer, "weekday"))
      line_one_setting = WEEKDAY;
    else if(!strcmp(buffer, "monthDay"))
      line_one_setting = MONTH_DATE;
    else if(!strcmp(buffer, "dayMonth"))
      line_one_setting = DATE_MONTH;
    else if(!strcmp(buffer, "weekdayDate"))
      line_one_setting = WEEKDAY_DATE;
    else if(!strcmp(buffer, "steps"))
      line_one_setting = STEPS;
    else if(!strcmp(buffer, "meters"))
      line_one_setting = METERS;
    else if(!strcmp(buffer, "feet"))
      line_one_setting = FEET;
    else if(!strcmp(buffer, "calories"))
      line_one_setting = CALORIES;
      
    //some extra information for layer manipulation
    Layer *window_layer = window_get_root_layer(main_window);
    GRect bounds = layer_get_bounds(window_layer);
    GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
      
    //The WEEKDAY_DATE setting is too large to fit, so its font size must be reduced and its layer must be adjusted!  
    if(line_one_setting == WEEKDAY_DATE){
      text_layer_set_font(line_one_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
      layer_set_frame(text_layer_get_layer(line_one_layer), GRect(
        center.x - bounds.size.w/6,
        center.y - line_one_offset_small,
        bounds.size.w/3,
        line_one_offset_small));
      layer_mark_dirty(text_layer_get_layer(line_one_layer));
    }
    else{
      text_layer_set_font(line_one_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      layer_set_frame(text_layer_get_layer(line_one_layer), GRect(
        center.x - bounds.size.w/6,
        center.y - line_one_offset,
        bounds.size.w/3,
        line_one_offset));
      layer_mark_dirty(text_layer_get_layer(line_one_layer));
    }
      
    persist_write_int(MESSAGE_KEY_topLineSetting, line_one_setting);
    update_lines(line_one_setting, 0);
  }
  if(bottom_line_t){
    char *buffer = bottom_line_t->value->cstring;  //The value returned from the settings page is a cstring
    
    //The cstring is immediately converted to an integer
    if(!strcmp(buffer, "digitalTime"))
      line_two_setting = DIGITAL;
    else if(!strcmp(buffer, "month"))
      line_two_setting = MONTH;
    else if(!strcmp(buffer, "date"))
      line_two_setting = DATE;
    else if(!strcmp(buffer, "weekday"))
      line_two_setting = WEEKDAY;
    else if(!strcmp(buffer, "monthDay"))
      line_two_setting = MONTH_DATE;
    else if(!strcmp(buffer, "dayMonth"))
      line_two_setting = DATE_MONTH;
    else if(!strcmp(buffer, "weekdayDate"))
      line_two_setting = WEEKDAY_DATE;
    else if(!strcmp(buffer, "steps"))
      line_two_setting = STEPS;
    else if(!strcmp(buffer, "meters"))
      line_two_setting = METERS;
    else if(!strcmp(buffer, "feet"))
      line_two_setting = FEET;
    else if(!strcmp(buffer, "calories"))
      line_two_setting = CALORIES;
    
    //some extra information for layer manipulation
    Layer *window_layer = window_get_root_layer(main_window);
    GRect bounds = layer_get_bounds(window_layer);
    GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
      
    //The WEEKDAY_DATE setting is too large to fit, so its font size must be reduced and its layer must be adjusted!  
    if(line_two_setting == WEEKDAY_DATE){
      text_layer_set_font(line_two_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
      layer_set_frame(text_layer_get_layer(line_two_layer), GRect(
        center.x - bounds.size.w/6,
        center.y - line_two_offset_small,
        bounds.size.w/3,
        bounds.size.w/6));
      layer_mark_dirty(text_layer_get_layer(line_two_layer));
    }
    else{
      text_layer_set_font(line_two_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      layer_set_frame(text_layer_get_layer(line_two_layer), GRect(
        center.x - bounds.size.w/6,
        center.y - line_two_offset,
        bounds.size.w/3,
        bounds.size.w/6));
      layer_mark_dirty(text_layer_get_layer(line_two_layer));
    }
      
    persist_write_int(MESSAGE_KEY_bottomLineSetting, line_two_setting);
    update_lines(line_two_setting, 1);
  }
  if(center_line_t){
    char *buffer = center_line_t->value->cstring;  //The value returned from the settings page is a cstring
    
    //The cstring is immediately converted to an integer
    if(!strcmp(buffer, "battery"))
      center_line_setting = BATTERY;
    else if(!strcmp(buffer, "constant"))
      center_line_setting = CONSTANT;
    else if(!strcmp(buffer, "none"))
      center_line_setting = NONE;
      
    persist_write_int(MESSAGE_KEY_centerLineSetting, center_line_setting);
    layer_mark_dirty(battery_layer);
  }
  if(bluetooth_vibes_t){
    bluetooth_vibes_bool = bluetooth_vibes_t->value->uint8;
    persist_write_bool(MESSAGE_KEY_bluetoothVibes, bluetooth_vibes_bool);
  }
  if(bluetooth_icon_t){
    bluetooth_icon_bool = bluetooth_icon_t->value->uint8;
    persist_write_bool(MESSAGE_KEY_bluetoothIcon, bluetooth_icon_bool);
  }
}

static void window_load(Window *window){
  
  //Loading all settings from persistant storage
  if(persist_exists(MESSAGE_KEY_backgroundColor)){
    int background_color_HEX = persist_read_int(MESSAGE_KEY_backgroundColor);
    bg_color_settings = GColorFromHEX(background_color_HEX);
  }
  else{
    bg_color_settings = GColorBlue;
    
  }
  if(persist_exists(MESSAGE_KEY_foregroundColor)){
    int foreground_color_HEX = persist_read_int(MESSAGE_KEY_foregroundColor);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);
  }
  else{
    fg_color_settings = GColorWhite;
  }
  color_setting = (persist_exists(MESSAGE_KEY_colorSetting)) ? persist_read_int(MESSAGE_KEY_colorSetting) : DARK;
  line_one_setting = (persist_exists(MESSAGE_KEY_topLineSetting)) ? persist_read_int(MESSAGE_KEY_topLineSetting) : MONTH_DATE;
  line_two_setting = (persist_exists(MESSAGE_KEY_bottomLineSetting)) ? persist_read_int(MESSAGE_KEY_bottomLineSetting) : STEPS;
  center_line_setting = (persist_exists(MESSAGE_KEY_centerLineSetting)) ? persist_read_int(MESSAGE_KEY_centerLineSetting) : BATTERY;
  bluetooth_vibes_bool = (persist_exists(MESSAGE_KEY_bluetoothVibes)) ? persist_read_bool(MESSAGE_KEY_bluetoothVibes) : true;//vibrate
  bluetooth_icon_bool = (persist_exists(MESSAGE_KEY_bluetoothIcon)) ? persist_read_bool(MESSAGE_KEY_bluetoothIcon) : true;//show
  
  //with all settings loaded, the global color variables can be set
  set_colors();
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  
  window_set_background_color(main_window, background_color);
  
  ring_layer = layer_create(bounds);
  layer_set_update_proc(ring_layer, ring_update_proc);
  layer_add_child(window_layer, ring_layer);
   
  //The layer's bounds are dependent on what it is displaying
  line_one_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - ((line_one_setting == WEEKDAY_DATE) ? line_one_offset_small : line_one_offset),
    bounds.size.w/3,
    ((line_one_setting == WEEKDAY_DATE) ? line_one_offset_small : line_one_offset)
  ));
  text_layer_set_text_color(line_one_layer, foreground_color);
  text_layer_set_background_color(line_one_layer, GColorClear);
   //text font must be smaller for WEEKDAY_DATE
    text_layer_set_font(line_one_layer, 
                        fonts_get_system_font((line_one_setting == WEEKDAY_DATE) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(line_one_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(line_one_layer));
  
  //The layer's bounds are dependent on what it is displaying
  line_two_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - ((line_two_setting == WEEKDAY_DATE) ? line_two_offset_small : line_two_offset),
    bounds.size.w/3,
    bounds.size.w/6
  ));
  text_layer_set_text_color(line_two_layer, foreground_color);
  text_layer_set_background_color(line_two_layer, GColorClear);
   //text font must be smaller for WEEKDAY_DATE
    text_layer_set_font(line_two_layer, 
                        fonts_get_system_font((line_two_setting == WEEKDAY_DATE) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(line_two_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(line_two_layer));
  
  battery_layer = layer_create(bounds);
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);
  
  bt_icon_layer = layer_create(bounds);
  layer_set_update_proc(bt_icon_layer, bt_icon_update_proc);
  layer_add_child(window_layer, bt_icon_layer);
  
  //these callbacks are run so they are accurate from load time
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  battery_callback(battery_state_service_peek());
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  tick_handler(t, MINUTE_UNIT);
}

static void window_unload(Window *window){
  //sweet destruction
  layer_destroy(battery_layer);
  layer_destroy(ring_layer);
  text_layer_destroy(line_one_layer);
  text_layer_destroy(line_two_layer);
}

static void init(void){
  srand(time(NULL));
  
  //subscribing to the necesary services
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
  
  //data from appmessage is not registered as freed
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 0);
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