#ifndef futura_weather_redux_main_h
#define futura_weather_redux_main_h

#include "pebble.h"

#define ALL_UNITS SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT | MONTH_UNIT | YEAR_UNIT

void load_preferences();
void save_preferences();
void send_preferences();

int get_resource_for_weather_conditions(uint32_t conditions);
uint32_t get_resource_for_weather_conditions_20(uint32_t conditions);
uint32_t get_resource_for_battery_state(BatteryChargeState battery);

//void change_preferences(Preferences *old_prefs, Preferences *new_prefs);
void update_weather_info(Weather *weather);

void out_sent_handler(DictionaryIterator *sent, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);

int main();
static void init();
static void window_load(Window *window);
static void window_unload(Window *window);
static void deinit();

void handle_tick(struct tm *now, TimeUnits units_changed);
//void handle_battery(BatteryChargeState battery);

void force_tick(TimeUnits units_changed);

#endif
