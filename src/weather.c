#include "pebble.h"
#include "config.h"
#include "weather.h"

#define DEBUGLOG 1

Weather* weather_load_cache() {
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather load cache ");
	static Weather weather1 = {
		.last_update_time = 0,
		.temperature = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions = 0,
		.name = "",
		.country ="",
		.temperature0 = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions0 = 0,	
		.time0 =0,
		.temperature6 = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions6 = 0,		
		.time6 =0,
		.temperature12 = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions12 = 0,		
		.time12 = 0,
		.temperature18 = -46100,				// Below absolute zero in Kelvin, Celsius, Fahrenheit, Rankine, Delisle, Newton, Rèaumur, and Rømer
		.conditions18 = 0,
		.time18 =0
	};
	if(persist_exists(PK_WEATHER)){
//		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read settings weather %d",sizeof(weather1));
		persist_read_data(PK_WEATHER, &weather1, sizeof(weather1) );
	}
	return &weather1;
}

bool weather_save_cache(Weather *weather) {
//	int result = 0;
//    result = 
	persist_write_data(PK_WEATHER, weather, sizeof(weather) );
//    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into settings", result); 
	return true;
}
		
		

bool weather_needs_update(Weather *weather, time_t update_freq) {
	time_t now = time(NULL);
	return now - weather->last_update_time >= update_freq;
}

void weather_request_update(int8_t set,int8_t cnt, char *type, int8_t period ) {
    if (set == 1 || (set==0 && strlen(type)>0)){
		if(cnt==4 || cnt==5 || cnt==6){cnt=4;}
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
    
		Tuplet request = TupletInteger(REQUEST_WEATHER_MSG_KEY, 1);
		dict_write_tuplet(iter, &request);
		Tuplet get_type = TupletInteger(REQUEST_WEATHER_GET_MSG_KEY,set);
		dict_write_tuplet(iter, &get_type);
		Tuplet get_location = TupletCString(REQUEST_WEATHER_LOCATION_MSG_KEY,type);
		dict_write_tuplet(iter, &get_location);
		Tuplet get_count = TupletInteger(REQUEST_WEATHER_COUNT,cnt);
		dict_write_tuplet(iter, &get_count);
		Tuplet set_period = TupletInteger(547, period);
		dict_write_tuplet(iter, &set_period);
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, " weather request position %d %s",set,type);
    app_message_outbox_send();
	} else {
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, " weather request position : not request");
	}
}


void weather_set(Weather *weather, DictionaryIterator *iter) {
	Tuple *conditions = dict_find(iter, WEATHER_CONDITIONS_MSG_KEY);
	Tuple *temperature = dict_find(iter, WEATHER_TEMPERATURE_MSG_KEY);
	Tuple *conditions0 = dict_find(iter, 701);
	Tuple *temperature0 = dict_find(iter, 700);
	Tuple *conditions6 = dict_find(iter, 703);
	Tuple *temperature6 = dict_find(iter, 702);
	Tuple *conditions12 = dict_find(iter, 705);
	Tuple *temperature12 = dict_find(iter, 704);
	Tuple *conditions18 = dict_find(iter, 707);
	Tuple *temperature18 = dict_find(iter, 706);
	
	Tuple *time0 = dict_find(iter, 708);
	Tuple *time6 = dict_find(iter, 709);	
	Tuple *time12 = dict_find(iter, 710);
	Tuple *time18 = dict_find(iter, 711);
	
	Tuple *name = dict_find(iter, WEATHER_NAME_MSG_KEY);
	Tuple *country = dict_find(iter, WEATHER_COUNTRY_MSG_KEY);	

	if(conditions)
		weather->conditions = conditions->value->int32;
	if(temperature)
		weather->temperature = temperature->value->int32;	
	
	if(conditions0)
		weather->conditions0 = conditions0->value->int32;
	if(temperature0)
		weather->temperature0 = temperature0->value->int32;
		
	if(conditions6)
		weather->conditions6 = conditions6->value->int32;
	if(temperature6)
		weather->temperature6 = temperature6->value->int32;
		
	if(conditions12)
		weather->conditions12 = conditions12->value->int32;
	if(temperature12)
		weather->temperature12 = temperature12->value->int32;
		
	if(conditions18)
		weather->conditions18 = conditions18->value->int32;
	if(temperature18)
		weather->temperature18 = temperature18->value->int32;
		
	if(time0)
		weather->time0 = time0->value->int32;
		
	if(time6)
		weather->time6 = time6->value->int32;		
		
	if(time12)
		weather->time12 = time12->value->int32;		
		
	if(time18)
		weather->time18 = time18->value->int32;		
		
	if(name){
		strncpy(weather->name, name->value->cstring, 39);
	}
	if(country){
		strncpy(weather->country, country->value->cstring,39);
	}
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",weather->name);
	time_t now = time(NULL);
	weather->last_update_time = now;
//	int result = 
	persist_write_data(PK_WEATHER, weather, sizeof(*weather) );
//    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into settings sizeof %d", result, sizeof(*weather)); 
/*	if(persist_exists(PK_WEATHER)){
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "PersistExists PK_Weather"); 
		Weather weather1;
		persist_read_data(PK_WEATHER, &weather1, sizeof(weather1) );
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read settings t12 weather1 %d",weather1.temperature12);
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_set location %s",weather1.name);
	}*/
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
//	if (DEBUGLOG) {APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown temperature format %d, using Kelvin", format);}
	return true_temperature;
}

