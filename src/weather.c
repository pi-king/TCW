#include "pebble.h"
#include "config.h"
#include "weather.h"

#define DEBUGLOG 0

Weather* weather_load_cache() {
	static Weather weather = {
		.last_update_time = 0,
		.temperature = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions = 0,
		.name = "",
		.country =""
	};
	
	if(persist_exists(WEATHER_CACHE_LAST_UPDATE_PERSIST_KEY))
		weather.last_update_time = persist_read_int(WEATHER_CACHE_LAST_UPDATE_PERSIST_KEY);
	if(persist_exists(WEATHER_CACHE_TEMPERATURE_PERSIST_KEY))
		weather.temperature = persist_read_int(WEATHER_CACHE_TEMPERATURE_PERSIST_KEY);
	if(persist_exists(WEATHER_CACHE_CONDITIONS_PERSIST_KEY))
		weather.conditions = persist_read_int(WEATHER_CACHE_CONDITIONS_PERSIST_KEY);
	if(persist_exists(WEATHER_CACHE_NAME_PERSIST_KEY)){
		char wname[40];
		char wname1[40];
		persist_read_string(WEATHER_CACHE_NAME_PERSIST_KEY,wname,40);	
		snprintf(wname1,sizeof(wname1),"%s",wname);
		strcpy(weather.name,wname1);
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location wname %s",weather.name);}
	}
	if(persist_exists(WEATHER_CACHE_COUNTRY_PERSIST_KEY)){
		char wname2[40];
		char wname3[40];
		persist_read_string(WEATHER_CACHE_COUNTRY_PERSIST_KEY,wname2,40);	
		snprintf(wname3,sizeof(wname3),"%s",wname2);
		strcpy(weather.country,wname3);
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location wname %s",weather.country);}
	}
	
	return &weather;
}

bool weather_save_cache(Weather *weather) {
	status_t save_last_update = persist_write_int(WEATHER_CACHE_LAST_UPDATE_PERSIST_KEY, (int)weather->last_update_time);
	status_t save_temperature = persist_write_int(WEATHER_CACHE_TEMPERATURE_PERSIST_KEY, weather->temperature);
	status_t save_conditions = persist_write_int(WEATHER_CACHE_CONDITIONS_PERSIST_KEY, weather->conditions);
	char wname[40];
	snprintf(wname,sizeof(wname),"%s",weather->name);
//	status_t save_name = persist_write_string(WEATHER_CACHE_NAME_PERSIST_KEY, weather->name);
	status_t save_name = persist_write_string(WEATHER_CACHE_NAME_PERSIST_KEY, wname);
	char wname1[40];
	snprintf(wname1,sizeof(wname1),"%s",weather->country);
//	status_t save_name = persist_write_string(WEATHER_CACHE_NAME_PERSIST_KEY, weather->name);
	status_t save_country = persist_write_string(WEATHER_CACHE_COUNTRY_PERSIST_KEY, wname1);

	
	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",wname);}
	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",wname1);}
	if(save_last_update < 0 || save_temperature < 0 || save_conditions < 0 || save_name <0 || save_country<0) {
		if (DEBUGLOG) {APP_LOG(APP_LOG_LEVEL_WARNING, "Failed to save weather cache");}
		return false;
	}
	return true;
}

bool weather_in_cache(){
	return (
		persist_exists(WEATHER_CACHE_LAST_UPDATE_PERSIST_KEY) &&
		persist_exists(WEATHER_CACHE_TEMPERATURE_PERSIST_KEY) &&
		persist_exists(WEATHER_CACHE_CONDITIONS_PERSIST_KEY) &&
		persist_exists(WEATHER_CACHE_NAME_PERSIST_KEY) &&
		persist_exists(WEATHER_CACHE_COUNTRY_PERSIST_KEY) 
	);
}		
		

bool weather_needs_update(Weather *weather, time_t update_freq) {
	time_t now = time(NULL);
	return now - weather->last_update_time >= update_freq;
}

void weather_request_update(int8_t set, char *type) {
    if (set == 1 || (set==0 && strlen(type)>0)){
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    Tuplet request = TupletInteger(REQUEST_WEATHER_MSG_KEY, 1);
    dict_write_tuplet(iter, &request);
	Tuplet get_type = TupletInteger(REQUEST_WEATHER_GET_MSG_KEY,set);
    dict_write_tuplet(iter, &get_type);
	Tuplet get_location = TupletCString(REQUEST_WEATHER_LOCATION_MSG_KEY,type);
    dict_write_tuplet(iter, &get_location);
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, " weather request position %d %s",set,type);
    app_message_outbox_send();
	} else {
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, " weather request position : not request");
	}
}

void weather_set(Weather *weather, DictionaryIterator *iter) {
	Tuple *conditions = dict_find(iter, WEATHER_CONDITIONS_MSG_KEY);
	Tuple *temperature = dict_find(iter, WEATHER_TEMPERATURE_MSG_KEY);
	Tuple *name = dict_find(iter, WEATHER_NAME_MSG_KEY);
	Tuple *country = dict_find(iter, WEATHER_COUNTRY_MSG_KEY);	
	
	if(conditions)
		weather->conditions = conditions->value->int32;
	if(temperature)
		weather->temperature = temperature->value->int32;
	if(name)
		strcpy(weather->name, name->value->cstring);
	if(country)
	    strcpy(weather->country, country->value->cstring);
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",name->value->cstring);	
	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",weather->name);}
	
	time_t now = time(NULL);
	weather->last_update_time = now;
	weather_save_cache(weather);
}



int weather_convert_temperature(int kelvin_temperature, TempFormat format) {
	float true_temperature = kelvin_temperature / 100.0f;				// We receive the temperature as an int for simplicity, but *100 to maintain accuracy
	switch(format) {
//		case TEMP_FORMAT_DEFAULT:
//			return true_temperature - 273.15f;
		case TEMP_FORMAT_CELCIUS:
			return true_temperature - 273.15f;
		case TEMP_FORMAT_FAHRENHEIT:
			return (9.0f/5.0f)*true_temperature - 459.67f;
	}
	
//	return true_temperature - 273.15f;
	if (DEBUGLOG) {APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown temperature format %d, using Kelvin", format);}
	return true_temperature;
}

