#include <pebble.h>
#include "config.h"
#include "pebble_app_info.h"
extern const PebbleAppInfo __pbl_app_info;
//#include "preferences.h"
#include "weather.h"
#include "tcw.h"
#define DEBUGLOG 0
#define TRANSLOG 0
// for iOs
#define STATUS_SCREEN_APP 			NUM_APPS
#define SM_SCREEN_ENTER_KEY			0xFC0E
typedef enum {CALENDAR_APP, MUSIC_APP, GPS_APP, STOCKS_APP, BITCOIN_APP, CAMERA_APP, WEATHER_APP, URL_APP, FINDPHONE_APP, REMINDERS_APP, STATUS_SCREEN_APP} AppIDs;
/*
 * If you fork this code and release the resulting app, please be considerate and change all the appropriate values in appinfo.json 
 *
 * DESCRIPTION
 *  This watchface shows the current date and current time in the top 'half',
 *    and then a small calendar w/ 3 weeks: last, current, and next week, in the bottom 'half'
 *  The statusbar at the top shows the connection status, charging, and battery level - and it will vibrate on link lost.
 *  The settings for the face are configurable using the new PebbleKit JS configuration page
 * END DESCRIPTION Section
 *
 */

 
static Window *window;

static Layer *datetime_layer;
static Layer *calendar_layer;
static Layer *title_layer;

static Layer *weather_layer;
static GBitmap *batt_charging;
static GBitmap *batt_pluged;

static InverterLayer *inverter_layer;
//static InverterLayer *inverter_title;

//static InverterLayer *inverter_f41;
//static InverterLayer *inverter_f42;
//static InverterLayer *inverter_f43;
//static InverterLayer *inverter_f44;

// battery info, instantiate to 'worst scenario' to prevent false hopes
static uint8_t battery_percent = 10;
static bool battery_charging = false;

static uint8_t phone_battery_percent =0;
static bool phone_battery_charging = false;
static bool phone_battery_plugged = false;

static bool is_inet_connected = true;

static int8_t phoneAMS = -1;

static uint8_t pebble_on_phone_battery_percent =0;
static bool pebble_on_phone_battery_charging = false;

static uint8_t sms_incom_count = 0;
static uint8_t calls_incom_count = 0;

static bool battery_plugged = false;
static bool battery_blink =false;
static uint8_t sent_battery_percent = 10;
static bool sent_battery_charging = false;
static bool sent_battery_plugged = false;
static bool get_phone_battery_state =false;
static uint8_t current_page = 0;
static uint8_t wsample = 0;
static uint8_t mail_count = 0;
static uint8_t alarm_set = 0;

static int curversion =0;
static char alarm_time[20];

static bool is_accel_stoped = false;

static uint8_t sleepmode3 = 0;

AppTimer *battery_sending = NULL;
AppTimer *auto_back_fp = NULL;
AppTimer *at_battery_blink = NULL;
AppTimer *accel_start = NULL;
AppTimer *lostconnection = NULL;
AppTimer *weather_update = NULL;

// connected info
static bool bluetooth_connected = false;
// suppress vibration
static bool vibe_suppression = true;
static bool accel_started = false;
static bool issleepmode = false;
static bool isloading = true;
static char weather_update_a[2] = "a";

Weather *weather;
//static struct tm *curTime;

void rotate_first_page();
void accel_int(AccelAxisType axis, int32_t direction);
static void set_current_page();
void update_connection();
void generate_vibe(uint32_t vibe_pattern_number);
char* get_weather_conditions(uint32_t conditions);
void SetSleepMode();

//static GFont f20;
//static GFont f40;
static GFont f60;
static GFont s20;
static GFont w25;
static GFont w40;
static GFont w60;

// define the persistent storage key(s)
//#define PERSIST_DATA_MAX_LENGTH 1024
#define   PK_SETTINGS      10
#define    PK_LANG_GEN      11
#define    PK_LANG_DATETIME 12	
#define    PK_LANG_DATETIME1 13	
#define    PK_LANG_DATETIME2 14	
#define	   PK_VERSION 15	
#define	   PK_SELECT_PAGE 1017
#define    SM_SEND_VERSION 1018

#define AK_SEND_PHONE_BATTERY_REQUEST 200
#define AK_SEND_BATTERY_VALUE 211
#define	AK_SEND_BATTERY_CHARGING 212
#define AK_SEND_BATTERY_PLUGGED 213

// define the appkeys used for appMessages
#define AK_STYLE_INV     30
#define AK_STYLE_DAY_INV 31
#define AK_STYLE_GRID    32
#define AK_VIBE_HOUR     33
#define AK_INTL_DOWO     34
#define AK_INTL_FMT_DATE 35 // INCOMPLETE
#define AK_STYLE_AM_PM   36
#define AK_STYLE_DAY     37
#define AK_STYLE_WEEK    38
#define AK_INTL_FMT_WEEK 39
#define AK_VERSION       40 // UNUSED
#define AK_VIBE_PAT_DISCONNECT   41
#define AK_VIBE_PAT_CONNECT      42

//#define AK_MESSAGE_TYPE          99
#define AK_SEND_BATT_PERCENT    100
#define AK_SEND_BATT_CHARGING   101
#define AK_SEND_BATT_PLUGGED    102
#define AK_TIMEZONE_OFFSET      103
#define AK_MIN_PHONE_STATE      104


#define AK_TRANS_ABBR_SUNDAY    500
#define AK_TRANS_ABBR_MONDAY    501
#define AK_TRANS_ABBR_TUESDAY   502
#define AK_TRANS_ABBR_WEDSDAY   503
#define AK_TRANS_ABBR_THURSDAY  504
#define AK_TRANS_ABBR_FRIDAY    505
#define AK_TRANS_ABBR_SATURDAY  506
#define AK_TRANS_JANUARY    507
#define AK_TRANS_FEBRUARY   508
#define AK_TRANS_MARCH      509
#define AK_TRANS_APRIL      510
#define AK_TRANS_MAY        511
#define AK_TRANS_JUNE       512
#define AK_TRANS_JULY       513
#define AK_TRANS_AUGUST     514
#define AK_TRANS_SEPTEMBER  515
#define AK_TRANS_OCTOBER    516
#define AK_TRANS_NOVEMBER   517
#define AK_TRANS_DECEMBER   518
#define AK_TRANS_ALARM      519 // UNUSED
#define AK_TRANS_SUNDAY     520
#define AK_TRANS_MONDAY     521
#define AK_TRANS_TUESDAY    522
#define AK_TRANS_WEDSDAY    523
#define AK_TRANS_THURSDAY   524
#define AK_TRANS_FRIDAY     525
#define AK_TRANS_SATURDAY   526
#define AK_TRANS_CONNECTED     527
#define AK_TRANS_DISCONNECTED  528
#define AK_TRANS_TIME_AM    529
#define AK_TRANS_TIME_PM    530
#define AK_DEGREE_STYLE     531
#define AK_FIRST_PAGE       532
#define AK_WEATHER_UPD      533
#define AK_BATTERY_SHOW     534
#define AK_PHONE_BATTERY_SHOW     539
#define AK_AUTO_BACK        540
#define AK_DEFAULT_FONT     535 
#define AK_GET_POSITION     536
#define AK_TYPE_POSITION     537
#define AK_GETPOSITION       538
#define AK_DATA_OVER_TIME    541
#define AK_TRANS_WEEK		542
#define AK_SHOW_BT_CONNECTED	 543
#define AK_SHOW_LOCATION	544
#define AK_SHOW_UPDATE_TIME	545
#define AK_SHOWDAYWEATHER	546
#define AK_WEATHER_PERIOD	547
#define AK_SLEEPBEGIN		548
#define AK_SLEEPEND			549
#define AK_SELECT_PAGE		550
#define AK_BAT_PRC_VIEW		551
#define AK_SHOW_PHONE_STATE	552
#define AK_SHOW_GMAIL		554
#define AK_VIBE_DISCONNECT_DELAY 555
#define AK_SLEEP_MODE		556

// primary coordinates
#define DEVICE_WIDTH        144
#define DEVICE_HEIGHT       168
#define LAYOUT_STAT           0 // 20 tall
#define LAYOUT_SLOT_TOP      24 // 72 tall
#define LAYOUT_SLOT_BOT      96 // 72 tall, 4px gap above
#define LAYOUT_SLOT_HEIGHT   72
#define STAT_BATT_LEFT       96 // LEFT + WIDTH + NIB_WIDTH <= 143
#define STAT_BATT_TOP         4
#define STAT_BATT_WIDTH      44 // should be divisible by 10, after subtracting 4 (2 pixels/side for the 'border')
#define STAT_BATT_HEIGHT     15
#define STAT_BATT_NIB_WIDTH   3 // >= 3
#define STAT_BATT_NIB_HEIGHT  5 // >= 3
#define STAT_BT_ICON_LEFT     4 // 0
#define STAT_BT_ICON_TOP      2
#define STAT_CHRG_ICON_LEFT  76
#define STAT_CHRG_ICON_TOP    2

#define SLOT_ID_CLOCK_1  0
#define SLOT_ID_CALENDAR 1
#define SLOT_ID_WEATHER  2
#define SLOT_ID_CLOCK_2  3

// Create a struct to hold our persistent settings...
typedef struct persist {
  uint8_t version;                // version key
  uint8_t inverted;               // Invert display
  uint8_t day_invert;             // Invert colors on today's date
  uint8_t grid;                   // Show the grid
  uint8_t vibe_hour;              // vibrate at the top of the hour?
  uint8_t dayOfWeekOffset;        // first day of our week
  uint8_t date_format;            // date format
  uint8_t show_am_pm;             // Show AM/PM below time
  uint8_t show_day;               // Show day name below time
  uint8_t show_week;              // Show week number below time
  uint8_t week_format;            // week format (calculation, e.g. ISO 8601)
  uint8_t vibe_pat_disconnect;    // vibration pattern for disconnect
  uint8_t vibe_pat_connect;       // vibration pattern for connect
  uint8_t slot_one;               // item in slot 1 [T]
  uint8_t slot_two;               // item in slot 2 [B]
  uint8_t slot_three;             // item in slot 3 [T, doubletap]
  uint8_t slot_four;              // item in slot 4 [B, doubletap]
  uint8_t slot_five;              // item in slot 5 [T, tripletap]
  uint8_t slot_six;               // item in slot 6 [B, tripletap]
  uint8_t degree_style;           // формат градусов С-F
  uint8_t first_page;             // что на первой странице календарь или погода
  uint32_t weather_upd;            // интервал обновления погоды
  uint32_t battery_show;           // отображать индикатор батареии
  uint32_t phone_battery_show;     // отображать индикатор батареи телефона
  uint32_t auto_back;				// автовозврат на первую страницу
  uint8_t default_font;            // шрифт часов
  uint8_t get_position;            // выбор позиции погоды
  char type_position[50];         // набраный населенный пункт
  uint8_t data_over_time;			// 1- дата над временем
  uint8_t show_bt_connected;
  uint8_t show_location;
  uint8_t show_update_time;
  uint8_t showDayWeather;
  uint8_t weatherPeriod;
  uint8_t sleepbegin;
  uint8_t sleepend;
  uint8_t select_page;
  uint8_t bat_cifr;
  uint8_t show_phone_state;
  uint8_t show_gmail;
  uint8_t vibe_disconnect_delay;
  uint8_t sleepmode;
} __attribute__((__packed__)) persist;

typedef struct persist_datetime_lang { // 249 bytes
  char abbrDaysOfWeek[7][5];      //  21:  2 characters for each of  7 weekdays
  char monthsNames[12][20];       // 144: 11 characters for each of 12 months
  char DaysOfWeek[7][32];         //  84: 11 characters for each of  7 weekdays
//                                   249 bytes
} __attribute__((__packed__)) persist_datetime_lang;

typedef struct persist_general_lang { // 32 bytes
//  char statuses[2][18];           //  20:  9 characters for each of  2 statuses
  char abbrTime[2][10];            //  12:  5 characters for each of  2 abbreviations
  char getposition[45];
  char week[5];
} __attribute__((__packed__)) persist_general_lang;

persist settings = {
  .version    = 2,
  .inverted   = 0, // no, dark
  .day_invert = 1, // yes
  .grid       = 1, // yes
  .vibe_hour  = 0, // no
  .dayOfWeekOffset = 1, // 0 - 6, Sun - Sat
  .date_format = 0, // Month DD, YYYY
  .show_am_pm  = 1, // no AM/PM       [0:Hide, 1:AM/PM, 2:TZ,    3:Week]
  .show_day    = 1, // no day name    [0:Hide, 1:Day,   2:Month, 3:TZ, 4:Week, 5:AM/PM]
  .show_week   = 1, // no week number [0:Hide, 1:Week,  2:TZ,    3:AM/PM
  .week_format = 0, // ISO 8601
  .vibe_pat_disconnect = 2, // double vibe
  .vibe_pat_connect = 0, // no vibe
  .slot_one   = 0, // clock_1
  .slot_two   = 1, // calendar
  .slot_three = 2, // TODO: weather
  .slot_four  = 3, // TODO: clock_2 (2nd timezone)
  // options for other slots: extend weather to take 2, moon, tides, travel to/home, context, etc.
  .slot_five  = 1, // TODO: calendar (test)
  .slot_six   = 0, // TODO: weather  (test)
  .degree_style = 1, // градусы С
  .first_page = 0,   // календарь
  .weather_upd = 10*60,  // обновление погоды
  .battery_show =101,    // индикатор батареи всегда
  .phone_battery_show = 101,
  .auto_back = 0,
  .default_font = 0,     // Digital
  .get_position = 1,     // GPS
  .type_position = "\0",
  .data_over_time = 1,
  .show_bt_connected = 1,
  .show_location = 1,
  .show_update_time = 1,
  .showDayWeather = 4,
  .weatherPeriod = 6,
  .sleepbegin = 0,
  .sleepend = 0,
  .select_page = 1,
  .bat_cifr = 0,
  .show_phone_state = 1,
  .show_gmail = 1,
  .vibe_disconnect_delay = 15,
  .sleepmode = 1,
};

persist_datetime_lang lang_datetime = {
  .abbrDaysOfWeek = { "Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб" },
  .monthsNames = { "ЯНВАРЬ", "ФЕВРАЛЬ", "МАРТ", "АПРЕЛЬ", "МАЙ", "ИЮНЬ", "ИЮЛЬ", "АВГУСТ", "СЕНТЯБРЬ", "ОКТЯБРЬ", "НОЯБРЬ", "ДЕКАБРЬ" },
  .DaysOfWeek = { "Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота" },
};

persist_general_lang lang_gen = {
  .abbrTime = { "AM", "PM" },
  .getposition = "поиск местоположения",
  .week = "Нд",
};

struct tm *get_time()
{
    time_t tt = time(0);
    return localtime(&tt);
}

static void connection_handle(void *data){
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "connection_handele");	
	lostconnection=NULL;
	if (bluetooth_connected) {
		generate_vibe(settings.vibe_pat_connect);  // non-op, by default
	} else {
		generate_vibe(settings.vibe_pat_disconnect);  // because, this is bad...
	}	
}

void SetSleepMode(){
	switch (settings.sleepmode){
		case 0:
		case 1:
			rotate_first_page();
			break;
		case 2:
		case 3:
			if (DEBUGLOG){app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "sleepmode %d",settings.sleepmode);}
				layer_set_hidden(weather_layer, true);
				layer_set_hidden(calendar_layer, true);
			break;
	}
}

void SleepModeRun(){
	if (settings.sleepmode==0){
		issleepmode=false;
		set_current_page();
		return;
	}
	struct tm *curTime = get_time();
	
	int hour=curTime->tm_hour;
	bool setsleepmode=false;

	if(settings.sleepbegin!=settings.sleepend){
		if(settings.sleepbegin>settings.sleepend){
			if(hour>=settings.sleepbegin || hour<settings.sleepend){
				//sleepmode
				if(! issleepmode) setsleepmode=true;
				issleepmode=true;
			}else{
				//sleepmode off
				if(issleepmode) setsleepmode=true;
				issleepmode=false;
			}
		}else{
			if(hour>=settings.sleepbegin && hour<settings.sleepend){
				//sleepmode
				if(! issleepmode) setsleepmode=true;
				issleepmode=true;				
			}else{
				//sleepmode off
				if(issleepmode) setsleepmode=true;
				issleepmode=false;				
			}
		}
	}else{
		//sleepmode off
		if(issleepmode) setsleepmode=true;
		issleepmode=false;		
	}
	if (setsleepmode){
		SetSleepMode();
		if(issleepmode) {
//			if(settings.first_page==2 || settings.first_page==3){	
				if(accel_started && settings.sleepmode != 3){
					accel_tap_service_unsubscribe();
					accel_started=false;
				}
//			}
		}else{
			if(settings.first_page==0 || settings.first_page==1){
				if(! accel_started){
					if (settings.select_page==1){
						accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);		
						accel_tap_service_subscribe(accel_int);
						accel_started=true;
					}
				}
			}
		}
	}
	if(issleepmode){
		if (sleepmode3<1){
			SetSleepMode();
		}
	}

	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "sleepmode3 %d",sleepmode3);}
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "sleep begin %d end %d  setsleepmode %d accel_started %d sleepmode %d SleepMode %d", settings.sleepbegin,settings.sleepend,setsleepmode,accel_started,issleepmode, settings.sleepmode);	}
}

void setColors(GContext* ctx) {
    window_set_background_color(window, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorWhite);
}

void setInvColors(GContext* ctx) {
    window_set_background_color(window, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_text_color(ctx, GColorBlack);
}

void draw_bitmap(GContext* ctx, int x, int y, int rid){
	GBitmap *image;
	image = gbitmap_create_with_resource(rid);
	GRect bounds=image->bounds;
	if(image!=NULL){
		graphics_draw_bitmap_in_rect(ctx,image, (GRect){ .origin = { x, y }, .size = bounds.size });
		gbitmap_destroy(image);
	}
}

void draw_text_sf(GContext* ctx,char* text, char* fnt, int x, int y,int w,int h,  GTextAlignment align){
	graphics_draw_text(ctx,text,fonts_get_system_font(fnt), GRect(x,y,w,h),GTextOverflowModeWordWrap,align,NULL);	
}

void draw_text_pf(GContext* ctx,char* text,GFont fnt, int x, int y,int w,int h){
	graphics_draw_text(ctx,text,fnt, GRect(x,y,w,h),GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);	
}

void drawtitleiconfont(GContext* ctx,int left, char* text){
	draw_text_pf(ctx,text,s20, left, -5,20,20);
}



void drawaweatherblock(GContext* ctx, int left,int top,int w_temperature, int conditions, time_t time){
if(conditions>0){
	bool isday=(conditions>=1000);
	if ((settings.showDayWeather==5 && isday)||(settings.showDayWeather==6 && !isday)){
		setInvColors(ctx);
/*	    window_set_background_color(window, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_text_color(ctx, GColorBlack);*/
		graphics_fill_rect(ctx,GRect(left,top+9,50,30),3,GCornersAll );
	}

	char *time_format1;
	if (clock_is_24h_style()) {
		time_format1 = " %R\n";
	} else {
		time_format1 = " %I:%M\n";
	}
	draw_text_pf(ctx,get_weather_conditions(conditions),w25,left,top,25,25);
	char temperature_text[8];
	char timetext[9];
	char fulltimetext[14];
	int temperature = weather_convert_temperature(w_temperature, settings.degree_style);
	snprintf(temperature_text, 8, "%d\u00B0", temperature);
	draw_text_sf(ctx,temperature_text,FONT_KEY_GOTHIC_18_BOLD,left+19/*64*/,top+7/*0*/,30,18, GTextAlignmentCenter);
	strftime(timetext, sizeof(timetext), time_format1, localtime(&time) );
	snprintf(fulltimetext,5,"%s",lang_datetime.abbrDaysOfWeek[localtime(&time)->tm_wday]);
	strcat(fulltimetext,timetext);
	draw_text_sf(ctx, fulltimetext,FONT_KEY_GOTHIC_14,left-1/*44*/,top+23/*16*/,50,14,GTextAlignmentCenter);	
}else{
	draw_text_pf(ctx,weather_update_a,w25,left+8,top+5,25,25);
}
	setColors(ctx);
}

static void weather_update_handle(void *data){
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_update_handle");
	weather_update= NULL;
	if (strncmp(weather_update_a,"a",1)==0){
		strncpy(weather_update_a,"b",1);
	}else{
		if (strncmp(weather_update_a,"b",1)==0){
		strncpy(weather_update_a,"c",1);
		}else{
			if (strncmp(weather_update_a,"c",1)==0){
				strncpy(weather_update_a,"a",1);
			}
		}
	}
	layer_mark_dirty(weather_layer);
}

void weather_layer_update_callback(Layer *me, GContext* ctx) {
	if( weather_update != NULL){
		weather_update = NULL;
	}
	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather_layer_update_callback");}
	if(settings.showDayWeather==4 || settings.showDayWeather==5 || settings.showDayWeather==6){
		if(settings.showDayWeather==4){
			setColors(ctx);
			graphics_draw_line(ctx,GPoint(93,2),GPoint(93,60));
			graphics_draw_line(ctx,GPoint(44,31),GPoint(144,31));
			graphics_draw_line(ctx,GPoint(43,2),GPoint(43,60));
		}
//		SetHideWeatherILayer(weather->conditions0>=1000/*weather->time0*/,inverter_f41,settings.showDayWeather);
//		SetHideWeatherILayer(weather->conditions6>=1000/*weather->time6*/,inverter_f42,settings.showDayWeather);
//		SetHideWeatherILayer(weather->conditions12>=1000/*weather->time12*/,inverter_f43,settings.showDayWeather);
//		SetHideWeatherILayer(weather->conditions18>=1000/*weather->time18*/,inverter_f44,settings.showDayWeather);		
//		setColors(ctx);
		
		if(weather->conditions>0){
//			drawweathericonfontbig(ctx,3,-5, get_weather_conditions(weather->conditions));
			draw_text_pf(ctx,get_weather_conditions(weather->conditions),w40,3,-5,42,42);
			char temperature_text[8];
			int temperature = weather_convert_temperature(weather->temperature, settings.degree_style);//prefs->temp_format);
			snprintf(temperature_text, 8, "%d\u00B0", temperature);
			draw_text_sf(ctx, temperature_text,FONT_KEY_GOTHIC_24_BOLD,4,30,40,24,GTextAlignmentCenter);
		}else{
//			drawweathericonfontbig(ctx,1,2, weather_update_a);
			draw_text_pf(ctx,weather_update_a,w40,1,2,42,42);
			weather_update = app_timer_register(2000, &weather_update_handle, NULL);
		}
		
		drawaweatherblock(ctx, 45,-7,weather->temperature0,weather->conditions0, weather->time0);
		drawaweatherblock(ctx, 45,23,weather->temperature6,weather->conditions6, weather->time6);
		drawaweatherblock(ctx, 95,-7,weather->temperature12,weather->conditions12, weather->time12);
		drawaweatherblock(ctx, 95,23,weather->temperature18,weather->conditions18, weather->time18);
		setColors(ctx);
	}else{
/*		layer_set_hidden(inverter_layer_get_layer(inverter_f41), true);
		layer_set_hidden(inverter_layer_get_layer(inverter_f42), true);
		layer_set_hidden(inverter_layer_get_layer(inverter_f43), true);
		layer_set_hidden(inverter_layer_get_layer(inverter_f44), true);*/
		if(weather->conditions>0){
//			app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "weather condition draw %s",get_weather_conditions(weather->conditions));
//			drawweathericonfontsbig(ctx,14,-11, get_weather_conditions(weather->conditions));
			draw_text_pf(ctx,get_weather_conditions(weather->conditions),w60,14,-11,90,90);
			char temperature_text[8];
			int temperature = weather_convert_temperature(weather->temperature, settings.degree_style);//prefs->temp_format);
			snprintf(temperature_text, 8, "%d\u00B0", temperature);
//			if(settings.default_font==0){ // digitsl
//				draw_text_pf(ctx,temperature_text,f40, 70, 5, 72, 80);
//			}
//			if(settings.default_font==1){ // normal
				draw_text_sf(ctx,temperature_text,/*FONT_KEY_GOTHIC_28_BOLD*/FONT_KEY_BITHAM_30_BLACK,70, 12, 72, 80, GTextAlignmentCenter);
//			}
		}else{
//			drawweathericonfontsbig(ctx,15,-12, weather_update_a);
			draw_text_pf(ctx,weather_update_a,w60,14,-11,90,90);
			weather_update = app_timer_register(2000, &weather_update_handle, NULL);
		}
	}
	char weather_text[80]="\0";
	if (is_inet_connected){
		if(weather->last_update_time>0){
			if(settings.show_location==1){
				strcat(weather_text,weather->name);
				strcat(weather_text," \0");
				if(settings.get_position==1){
					strcat(weather_text," (GPS)");
				}
			}
			if(settings.show_update_time==1){
				char time_text[] = "00:00";
				char *time_format;
				if (clock_is_24h_style()) {
					time_format = "%R\n";
				} else {
					time_format = "%I:%M\n";
				}
				strftime(time_text, sizeof(time_text), time_format, localtime(&weather->last_update_time));
				strcat(weather_text,"  ");
				strcat(weather_text,time_text);
			}
		}else{
			strcat(weather_text,lang_gen.getposition);
		}
	}else{
		snprintf(weather_text, sizeof(weather_text), "Offline");
	}
	draw_text_sf(ctx,weather_text,FONT_KEY_GOTHIC_14,0, 57, 144, 14, GTextAlignmentCenter);
}

// How many days are/were in the month
int daysInMonth(int mon, int year)
{
    mon++; // dec = 0|12, lazily optimized

    // April, June, September and November have 30 Days
    if (mon == 4 || mon == 6 || mon == 9 || mon == 11) {
        return 30;
    } else if (mon == 2) {
        // Deal with Feburary & Leap years
        if (year % 400 == 0) {
            return 29;
        } else if (year % 100 == 0) {
            return 28;
        } else if (year % 4 == 0) {
            return 29;
        } else {
            return 28;
        }
    } else {
        // Most months have 31 days
        return 31;
    }
}

char* get_weather_conditions(uint32_t conditions){
	bool is_day = conditions >= 1000;
    switch((conditions - (conditions % 100)) % 1000) {
        case 0:
            return "C";
        case 200:
            return "Q";
        case 300:
            return "F";
        case 500:
            return "K";
        case 600:
            switch(conditions % 100) {
                case 611:
                    return "N";
                case 612:
                    return "L";
                case 615:
                case 616:
                case 620:
                case 621:
                case 622:
                    return "M";
            }
            return "O";
        case 700:
            switch(conditions % 100) {
                case 731:
                case 781:
                    return "R";
            }
            return "G";
        case 800:
            switch(conditions % 100) {
                case 0:
					if(is_day)
						return "A";
					return "B";
                case 1:
                case 2:
					if(is_day)
						return "I";
					return "J";
                case 3:
                case 4:
                    return "D";
            }
        case 900:
            switch(conditions % 100) {
                case 0:
                case 1:
                case 2:
                    return "R";
                case 3:
                    return "H";
                case 4:
                    return "E";
                case 5:
                    return "R";
                case 950:
                case 951:
                case 952:
                case 953:
					if(is_day)
						return "A";
					return "B";
                case 954:
                case 955:
                case 956:
                case 957:
                case 959:
                case 960:
                case 961:
                case 962:
                    return "R";
            }
    }
    return "C";

}



void calendar_layer_update_callback(Layer *me, GContext* ctx) {
    struct tm *curTime = get_time();
	if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "calendar_layer_update_callback"); };
    int mon = curTime->tm_mon;
    int year = curTime->tm_year + 1900;
    int daysThisMonth = daysInMonth(mon, year);
    int specialDay = curTime->tm_wday - settings.dayOfWeekOffset;
    if (specialDay < 0) { specialDay += 7; }
    /* We're going to build an array to hold the dates to be shown in the calendar.
     *
     * There are five 'parts' we'll calculate for this (though since we only display 3 weeks, we'll only ever see at most 4 of them)
     *
     *   daysVisPrevMonth = days from the previous month that are visible
     *   daysPriorToToday = days before today (including any days from previous month)
     *   ( today )
     *   daysAfterToday   = days after today (including any days from next month)
     *   daysVisNextMonth = days from the following month that are visible
     *
     *  daysPriorToToday + 1 + daysAfterToday = 21, since we display exactly 3 weeks.
     */
    int show_last = 1; // show last week?
    int show_next = 1; // show next week?
//	int show_day_abbr = 0; // выводить заголовки дней недели
    int calendar[21];
    int cellNum = 0;   // address for current day table cell: 0-20
    int daysVisPrevMonth = 0;
    int daysVisNextMonth = 0;
    int daysPriorToToday = 7 + curTime->tm_wday - settings.dayOfWeekOffset;
    int daysAfterToday   = 6 - curTime->tm_wday + settings.dayOfWeekOffset;

    // tm_wday is based on Sunday being the startOfWeek, but Sunday may not be our startOfWeek.
    if (curTime->tm_wday < settings.dayOfWeekOffset) { 
      if (show_last) {
        daysPriorToToday += 7; // we're <7, so in the 'first' week due to startOfWeek offset - 'add a week' before this one
      }
    } else {
      if (show_next) {
        daysAfterToday += 7;   // otherwise, we're already in the second week, so 'add a week' after
      }
    }

    if ( daysPriorToToday >= curTime->tm_mday ) {
      // We're showing more days before today than exist this month
      int daysInPrevMonth = daysInMonth(mon - 1,year); // year only matters for February, which will be the same 'from' March

      // Number of days we'll show from the previous month
      daysVisPrevMonth = daysPriorToToday - curTime->tm_mday + 1;

      for (int i = 0; i < daysVisPrevMonth; i++, cellNum++ ) {
        calendar[cellNum] = daysInPrevMonth + i - daysVisPrevMonth + 1;
      }
    }

    // optimization: instantiate i to a hot mess, since the first day we show this month may not be the 1st of the month
    int firstDayShownThisMonth = daysVisPrevMonth + curTime->tm_mday - daysPriorToToday;
    for (int i = firstDayShownThisMonth; i < curTime->tm_mday; i++, cellNum++ ) {
      calendar[cellNum] = i;
    }

    //int currentDay = cellNum; // the current day... we'll style this special
    calendar[cellNum] = curTime->tm_mday;
    cellNum++;

    if ( curTime->tm_mday + daysAfterToday > daysThisMonth ) {
      daysVisNextMonth = curTime->tm_mday + daysAfterToday - daysThisMonth;
    }

    // add the days after today until the end of the month/next week, to our array...
    int daysLeftThisMonth = daysAfterToday - daysVisNextMonth;
    for (int i = 0; i < daysLeftThisMonth; i++, cellNum++ ) {
      calendar[cellNum] = i + curTime->tm_mday + 1;
    }

    // add any days in the next month to our array...
    for (int i = 0; i < daysVisNextMonth; i++, cellNum++ ) {
      calendar[cellNum] = i + 1;
    }

// ---------------------------
// Now that we've calculated which days go where, we'll move on to the display logic.
// ---------------------------

    #define CAL_DAYS   7   // number of columns (days of the week)
    #define CAL_WIDTH  20  // width of columns
    #define CAL_GAP    1   // gap around calendar
    #define CAL_LEFT   2   // left side of calendar
    #define CAL_HEIGHT 18  // How tall rows should be depends on how many weeks there are
//    #define CAL_HEIGHT 12  // How tall rows should be depends on how many weeks there are

    int weeks  =  3;  // always display 3 weeks: previous, current, next
    if (!show_last) { weeks--; }
    if (!show_next) { weeks--; }
        
    GFont normal = fonts_get_system_font(FONT_KEY_GOTHIC_14); // fh = 16
    GFont bold   = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); // fh = 22
    GFont current = normal;
    int font_vert_offset = 0;

    // generate a light background for the calendar grid
    setInvColors(ctx);
	if(settings.grid==1){
		graphics_fill_rect(ctx, GRect (CAL_LEFT + CAL_GAP, CAL_HEIGHT - CAL_GAP, DEVICE_WIDTH - 2 * (CAL_LEFT + CAL_GAP), CAL_HEIGHT * weeks), 0, GCornerNone);
	}
    setColors(ctx);
    for (int col = 0; col < CAL_DAYS; col++) {

      // Adjust labels by specified offset
      int weekday = col + settings.dayOfWeekOffset;
      if (weekday > 6) { weekday -= 7; }

      if (col == specialDay) {
        current = bold;
        font_vert_offset = 0;
      }
      // draw the cell background
      graphics_fill_rect(ctx, GRect (CAL_WIDTH * col + CAL_LEFT + CAL_GAP, 0, CAL_WIDTH - CAL_GAP, CAL_HEIGHT - CAL_GAP), 0, GCornerNone);

      // draw the cell text
      graphics_draw_text(ctx, lang_datetime.abbrDaysOfWeek[weekday], current, GRect(CAL_WIDTH * col + CAL_LEFT + CAL_GAP, CAL_GAP + font_vert_offset, CAL_WIDTH, CAL_HEIGHT), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
      if (col == specialDay) {
        current = normal;
        font_vert_offset = 0;
      }
    }

    // draw the individual calendar rows/columns
    int week = 0;
    for (int row = 1; row <= 3; row++) {
      if (row == 1 && !show_last) { continue; }
      if (row == 3 && !show_next) { continue; }
      week++;
      for (int col = 0; col < CAL_DAYS; col++) {
        if ( row == 2 && col == specialDay) {
		  if(settings.day_invert ==1){	
			setInvColors(ctx);
		  }
          current = bold;
          font_vert_offset = 0;
        }

        // draw the cell background
        graphics_fill_rect(ctx, GRect (CAL_WIDTH * col + CAL_LEFT + CAL_GAP, CAL_HEIGHT * week, CAL_WIDTH - CAL_GAP, CAL_HEIGHT - CAL_GAP), 0, GCornerNone);

        // draw the cell text
        char date_text[3];
        snprintf(date_text, sizeof(date_text), "%d", calendar[col + 7 * (row - 1)]);
        graphics_draw_text(ctx, date_text, current, GRect(CAL_WIDTH * col + CAL_LEFT, CAL_HEIGHT * week - CAL_GAP + font_vert_offset, CAL_WIDTH, CAL_HEIGHT), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 

        if ( row == 2 && col == specialDay) {
		  if(settings.day_invert==1){	
            setColors(ctx);
		  }
          current = normal;
          font_vert_offset = 0;
        }
      }
    }
}

void datetime_layer_update_callback(Layer *me, GContext* ctx) {
if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "datetime_layer_update_callback"); };
//app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "datetime_layer_update_callback %d:%d",curTime->tm_hour,curTime->tm_min); 
    setColors(ctx);
// update time text	
  char time_text[] = "00:00";
  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  struct tm *curTime = get_time();

  strftime(time_text, sizeof(time_text), time_format, curTime);
  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }	
//--------------------------------
//update date text
    char date_text[20];
    char date_string[30];

    if (settings.date_format == 0) {
      // Month DD, YYYY (localized)
      strftime(date_text, sizeof(date_text), "%d, %Y", curTime); // DD, YYYY
      snprintf(date_string, sizeof(date_string), "%s %s", lang_datetime.monthsNames[curTime->tm_mon], date_text); // prefix Month (localized)
    } else if (settings.date_format == 1) {
      // DD.MM.YYYY
      strftime(date_text, sizeof(date_text), "%d.%m.%Y", curTime);  // DD.MM.YYYY
      snprintf(date_string, sizeof(date_string), "%s", date_text); // straight copy
    }
    // dd/mm/yyyy
    // yyyy.mm.dd
    // yyyy mm dd
    // yyyy/mm/dd
    // YYYY MM DD
    // YYYY-MM-DD
    // D MMMM YYYY
    // yyyy-mm-dd
    // d.m.yyyy

	if(settings.data_over_time==1){ // дата над временем
//		if(settings.default_font==1){ // фонт нормал
			if(!settings.show_day && !settings.show_week &&!settings.show_am_pm){ // если не выводится нижняя строка
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, -2,144,24, GTextAlignmentCenter);
				if(settings.default_font==1){
					draw_text_sf(ctx, time_text,FONT_KEY_ROBOTO_BOLD_SUBSET_49, 0, 15,144,50, GTextAlignmentCenter);
				}else{
					draw_text_pf(ctx,time_text,f60, 2, 6,144,124);
				}
			}else{ // если выводится нижняя строка
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, -6,144,24, GTextAlignmentCenter);
				if(settings.default_font==1){
					draw_text_sf(ctx, time_text,FONT_KEY_ROBOTO_BOLD_SUBSET_49, 0, 8,144,50, GTextAlignmentCenter);			
				}else{
					draw_text_pf(ctx,time_text,f60, 2, -2,144,124);
				}
			}
/*		}else{	// фонт дигитал
			if(!settings.show_day && !settings.show_week &&!settings.show_am_pm){ // если не выводится нижняя строка
//				draw_text_pf(ctx,date_string,f20, 2, -2,144,24);
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, -2,144,24, GTextAlignmentCenter);
				draw_text_pf(ctx,time_text,f60, 2, 6,144,124);
			}else{ // если выводится нижняя строка
//				draw_text_pf(ctx,date_string,f20, 2, -4,144,24);
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, -6,144,24, GTextAlignmentCenter);
				draw_text_pf(ctx,time_text,f60, 2, -2,144,124);
			}
		}*/
	}else{ // время над датой
//		if(settings.default_font==1){ // фонт нормал
			if(!settings.show_day && !settings.show_week &&!settings.show_am_pm){ // если не выводится нижняя строка
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, 39,144,24, GTextAlignmentCenter);
				if(settings.default_font==1){ // фонт нормал
					draw_text_sf(ctx, time_text,FONT_KEY_ROBOTO_BOLD_SUBSET_49, 0, -9,144,50, GTextAlignmentCenter);
				}else{
					draw_text_pf(ctx,time_text,f60, 2, -14,144,124);
				}
			}else{ // если выводится нижняя строка
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, 33,144,24, GTextAlignmentCenter);
				if(settings.default_font==1){ // фонт нормал
					draw_text_sf(ctx, time_text,FONT_KEY_ROBOTO_BOLD_SUBSET_49, 0, -12,144,50, GTextAlignmentCenter);				
				}else{
					draw_text_pf(ctx,time_text,f60, 2, -19,144,124);
				}
			}
/*		}else{	// фонт дигитал
			if(!settings.show_day && !settings.show_week &&!settings.show_am_pm){ // если не выводится нижняя строка
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, 39,144,24, GTextAlignmentCenter);
//				draw_text_pf(ctx,date_string,f20, 2, 47,144,24);
				draw_text_pf(ctx,time_text,f60, 2, -14,144,124);
			}else{ // если выводится нижняя строка
//				draw_text_pf(ctx,date_string,f20, 2, 38,144,24);
				draw_text_sf(ctx, date_string,FONT_KEY_GOTHIC_24, 0, 33,144,24, GTextAlignmentCenter);
				draw_text_pf(ctx,time_text,f60, 2, -19,144,124);
			}
		}*/
	}
		if(settings.show_day>0){
			if(settings.show_day==1){
				char dow[32];
				snprintf(dow, sizeof(lang_datetime.DaysOfWeek[curTime->tm_wday]), "%s",lang_datetime.DaysOfWeek[curTime->tm_wday]);
				draw_text_sf(ctx,dow,FONT_KEY_GOTHIC_14,36,57,72,16, GTextAlignmentCenter);
			}
			if(settings.show_day==2){
				if(alarm_set) { 
					draw_text_pf(ctx,"A",s20, 48, 52,25,25);
					draw_text_sf(ctx,alarm_time,FONT_KEY_GOTHIC_14,66,57,72,16, GTextAlignmentLeft);
				}
			}
		}
		if(settings.show_week){
			char week_text[7] = "\0";
			char week_num[3] = "\0";
			if (settings.week_format == 0) {
				// ISO 8601 week number (00-53)
				strftime(week_num, sizeof(week_num), "%V", curTime);
			} else if (settings.week_format == 1) {
				// Week number with the first Sunday as the first day of week one (00-53)
				strftime(week_num, sizeof(week_num), "%U", curTime);
			} else if (settings.week_format == 2) {
				// Week number with the first Monday as the first day of week one (00-53)
				strftime(week_num, sizeof(week_num), "%W", curTime);
			}
			snprintf(week_text, sizeof(lang_gen.week), "%s",lang_gen.week);
			strncat(week_text,week_num,2);		
			draw_text_sf(ctx,week_text,FONT_KEY_GOTHIC_14,0,56,36,16, GTextAlignmentCenter);
		}
		if(settings.show_am_pm){
			if (curTime->tm_hour < 12 ) {
				draw_text_sf(ctx,lang_gen.abbrTime[0],FONT_KEY_GOTHIC_14,114,56,36,16, GTextAlignmentCenter);
			} else {
				draw_text_sf(ctx,lang_gen.abbrTime[1],FONT_KEY_GOTHIC_14,114,56,36,16, GTextAlignmentCenter);
			} 		
		}
}

static void fn_battery_blink(void *data){
	battery_blink = !battery_blink;
	at_battery_blink = NULL;
	layer_mark_dirty(title_layer);
}

void set_title_layer_color(GContext* ctx, bool clr){
	if(settings.show_bt_connected==4 && !bluetooth_connected){
		if(clr){
			setInvColors(ctx);
		}else{
			setColors(ctx);
		}
	}else{
		if(clr){
			setColors(ctx);
		}else{
			setInvColors(ctx);
		}
	
	}
}

void title_layer_update_callback(Layer *me, GContext* ctx) {
	if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "title_layer_update_callback"); };
	set_title_layer_color(ctx,true);	
	if(settings.show_bt_connected==4 && !bluetooth_connected){
//		set_title_layer_color(ctx,true);
//		setInvColors(ctx);
		graphics_fill_rect(ctx,GRect(0,0,DEVICE_WIDTH,24),3,GCornersAll );
	}
	if (bluetooth_connected) {
		if(settings.show_bt_connected==1 || settings.show_bt_connected==2){
			drawtitleiconfont(ctx,0, "B");
		}
/*		if(settings.show_bt_connected==4){
			if(layer_get_hidden(inverter_layer_get_layer(inverter_title))==false){
				layer_set_hidden(inverter_layer_get_layer(inverter_title), true);
			}
		}*/
		if(settings.show_phone_state>0 ){
			if(phoneAMS>-1){
				if(phoneAMS==0) drawtitleiconfont(ctx,20,"X");//draw_bitmap(ctx, 20, 2, RESOURCE_ID_AMS0);
				if(phoneAMS==1) drawtitleiconfont(ctx,20,"Y");//draw_bitmap(ctx, 20, 2, RESOURCE_ID_AMS1);
				if(settings.show_phone_state==1){
					if(phoneAMS==2 /*&& alarm_set == 0*/ ) drawtitleiconfont(ctx,20,"Z");//draw_bitmap(ctx, 20, 2, RESOURCE_ID_AMS2);
				}
			}
		}
		if(settings.show_gmail==1){
			if(mail_count>0){
				drawtitleiconfont(ctx,40,"G");
			}else{
				if(sms_incom_count>0){
					drawtitleiconfont(ctx,40, "S");
				}
			}
		}else{
			if(sms_incom_count>0){
				drawtitleiconfont(ctx,40, "S");
			}
		}
		if(calls_incom_count>0){
			drawtitleiconfont(ctx,60, "C");
		}	
	} else {
		if(settings.show_bt_connected==1 || settings.show_bt_connected==3){
			drawtitleiconfont(ctx,0, "N");
		}
/*		if(settings.show_bt_connected==4){
			if(layer_get_hidden(inverter_layer_get_layer(inverter_title))){
				layer_set_hidden(inverter_layer_get_layer(inverter_title), false);
			}
		}*/
	}
	
	if(issleepmode){
		drawtitleiconfont(ctx,80, "W");
	}else{
		if (settings.vibe_hour) {
			drawtitleiconfont(ctx,80, "V");
//			draw_bitmap(ctx, 80, 2, RESOURCE_ID_IMAGE_HOURVIBE_ICON);
		}
	}
	
	static char prctext[4];
	if((phone_battery_percent<settings.phone_battery_show || phone_battery_charging || phone_battery_plugged) && phone_battery_percent>0 ){
			set_title_layer_color(ctx,true);
			int prc=phone_battery_percent;
//			int hgth = (int)prc/(100/13);
			int hgth = (int)prc/(100/12);
			
//			setColors(ctx);
			
			draw_bitmap(ctx, 100, 2+13, RESOURCE_ID_BAT_PHONE);
			if (phone_battery_charging && phone_battery_plugged){
				graphics_draw_bitmap_in_rect(ctx,batt_charging,GRect(100,14,11,11));
			}else{ 
				if (phone_battery_plugged){
					graphics_draw_bitmap_in_rect(ctx,batt_pluged,GRect(100,14,11,11));	
				}
			}	

			if( settings.bat_cifr==0){
				set_title_layer_color(ctx,true);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setInvColors(ctx);
				}else{
					setColors(ctx);
				}			*/
				graphics_draw_rect(ctx, GRect (100+2,2,16,12));	
				graphics_draw_line(ctx,GPoint(100+1,2+3),GPoint(100+1,2+3+5));
				set_title_layer_color(ctx,false);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setColors(ctx);
				}else{
					setInvColors(ctx);
				}*/
				graphics_fill_rect(ctx, GRect (100+2+2+12-hgth,2+2,hgth,8), 0, GCornerNone);
			}else{
				set_title_layer_color(ctx,true);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setInvColors(ctx);
				}else{
					setColors(ctx);
				}*/
			
//				setColors(ctx);
				snprintf(prctext,4,"%d",phone_battery_percent);
				draw_text_sf(ctx,prctext,FONT_KEY_GOTHIC_14,100,-1,20,14, GTextAlignmentCenter);
			
			}
/*		}
		if( settings.bat_cifr==1){
			setColors(ctx);
			draw_bitmap(ctx, 100, 2, RESOURCE_ID_BAT_PHONE);
			char text[4];
			sprintf(text,"%d",phone_battery_percent);
			draw_text(ctx,FONT_KEY_GOTHIC_14,100,2,20,14, text);
		}*/
	}

	if(battery_percent<settings.battery_show || battery_charging || battery_plugged){
			int prc=battery_percent;
			int hgth = (int)prc/(100/12);
			set_title_layer_color(ctx,false);
//			setColors(ctx);
			draw_bitmap(ctx, 122, 2+13, RESOURCE_ID_BAT_PEBBLE);
			if(battery_blink){
				if (battery_charging && battery_plugged){
					graphics_draw_bitmap_in_rect(ctx,batt_charging,GRect(122,14,11,11));
				}else{
					if (battery_plugged){
						graphics_draw_bitmap_in_rect(ctx,batt_pluged,GRect(122,14,11,11));	
					}
				}
			}
			if( settings.bat_cifr==0){
				set_title_layer_color(ctx,true);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setInvColors(ctx);
				}else{
					setColors(ctx);
				}			*/
				graphics_draw_rect(ctx, GRect (122+2,2,16,12));	
				graphics_draw_line(ctx,GPoint(122+1,2+3),GPoint(122+1,2+3+5));
				set_title_layer_color(ctx,false);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setColors(ctx);
				}else{
					setInvColors(ctx);
				}			*/

//				setInvColors(ctx);
				graphics_fill_rect(ctx, GRect (122+2+2+12-hgth,2+2,hgth,8), 0, GCornerNone);			
			}else{
				set_title_layer_color(ctx,true);
/*				if(settings.show_bt_connected==4 && !bluetooth_connected){
					setInvColors(ctx);
				}else{
					setColors(ctx);
				}*/
			
//				setColors(ctx);
				snprintf(prctext,4,"%d",battery_percent);
				draw_text_sf(ctx,prctext,FONT_KEY_GOTHIC_14,122,-1,20,14, GTextAlignmentCenter);
			}
			if (battery_charging || battery_plugged){	
				if(at_battery_blink == NULL){
					at_battery_blink = app_timer_register(1000, &fn_battery_blink, NULL);	
				}
			}else{
				at_battery_blink = NULL;	
			}
	}
}




static void battery_status_send(void *data) {
  if ( (battery_percent  == sent_battery_percent  )
     & (battery_charging == sent_battery_charging )
     & (battery_plugged  == sent_battery_plugged  ) 
	 & (pebble_on_phone_battery_percent == battery_percent)
	 & (pebble_on_phone_battery_charging == battery_charging)	 
	 ) {
//    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "repeat battery reading"); }
    battery_sending = NULL;
    return; // no need to resend the same value
  }
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (iter == NULL) {
 //   if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "iterator is null: %d", result); }
    return;
  }

  if (result != APP_MSG_OK) {
 //   if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed to open outbox: %d", (AppMessageResult) result); }
    return;
  }

/*  if (dict_write_int8(iter, SM_SCREEN_ENTER_KEY, STATUS_SCREEN_APP) != DICT_OK){
	return;
  }

  if (dict_write_int8(iter, SM_SEND_VERSION, curversion) != DICT_OK){
	return;
  }  */

  
/*  if (dict_write_uint8(iter, AK_MESSAGE_TYPE, AK_SEND_BATT_PERCENT) != DICT_OK) {
    return;
  }
  if (dict_write_uint8(iter, AK_SEND_BATT_PERCENT, battery_percent) != DICT_OK) {
    return;
  }
  if (dict_write_uint8(iter, AK_SEND_BATT_CHARGING, battery_charging ? 1: 0) != DICT_OK) {
    return;
  }
  if (dict_write_uint8(iter, AK_SEND_BATT_PLUGGED, battery_plugged ? 1: 0) != DICT_OK) {
    return;
  }*/
/*  if(dict_write_int(iter,AK_SEND_BATTERY_VALUE,&battery_percent,sizeof(uint8_t),true)!=DICT_OK){
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_VALUE"); };
  }
  int sv = 0;
  if (battery_charging){ sv=1;};
  if(dict_write_int(iter,AK_SEND_BATTERY_CHARGING,&sv,sizeof(int),true)!=DICT_OK){
//	if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); };
  }
  sv = 0;
  if (battery_plugged) {sv=1;};
  if(dict_write_int(iter,AK_SEND_BATTERY_PLUGGED,&sv,sizeof(int),true)!=DICT_OK){
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); };
  }
  
  if(dict_write_int(iter,AK_SEND_PHONE_BATTERY_REQUEST,&sv,sizeof(int),true)!=DICT_OK){
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); };
  }  
  sv= settings.phone_battery_show;	
  if (dict_write_int(iter, AK_MIN_PHONE_STATE, &sv,sizeof(int),true) != DICT_OK) {
    return;
  } */
  uint8_t sarr[4];
  sarr[0]=curversion;
  sarr[1]=battery_percent;
  sarr[2]=0;
  if(battery_charging){
	sarr[2]^=1;
  }
  if(battery_plugged){
	sarr[2]^=1<<1;
  }
  if(phone_battery_percent==0){  // asc phone battary percent
	sarr[2]^=1<<2;
  }
  sarr[3]=settings.phone_battery_show;  
  
  if (dict_write_data(iter, 333, sarr,4) != DICT_OK){
	return;
  }  
/*  Send_val sendv;
  sendv.status=STATUS_SCREEN_APP;
  sendv.version=curversion;
  sendv.batprc=battery_percent;
  sendv.batcharg=battery_charging;
  sendv.batplug=battery_plugged;
  sendv.batreqw=true;
  sendv.batshow=settings.phone_battery_show;
  if (dict_write_data(iter, 333, sendv.array,sizeof(sendv.array)) != DICT_OK){
	return;
  }*/

  
  app_message_outbox_send();
  sent_battery_percent  = battery_percent;
  sent_battery_charging = battery_charging;
  sent_battery_plugged  = battery_plugged;
  battery_sending = NULL;

}





static void show_battery_state(){
	if (battery_sending == NULL) {
		battery_sending = app_timer_register(5000, &battery_status_send, NULL);
//		if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery timer queued"); }
	}
	layer_mark_dirty(title_layer);
}



static void handle_battery(BatteryChargeState charge_state) {
  battery_percent = charge_state.charge_percent;
  battery_charging = charge_state.is_charging;
  battery_plugged = charge_state.is_plugged;
  if (battery_charging || battery_plugged){
	if(at_battery_blink == NULL){
		at_battery_blink = app_timer_register(1000, &fn_battery_blink, NULL);
	}
  }else{
	at_battery_blink = NULL;
  }
  show_battery_state(NULL);
}

static void accel_starting(void *data) {
	is_accel_stoped = false;
	accel_start = NULL;
}

void generate_vibe(uint32_t vibe_pattern_number) {
  if (vibe_suppression) { return; }
  if (issleepmode) {return;}
  is_accel_stoped = true;
  accel_start = NULL;
  accel_start = app_timer_register(2000, &accel_starting, NULL);
  vibes_cancel();
  switch ( vibe_pattern_number ) {
  case 0: // No Vibration
//	is_accel_stoped = false;
    return;
  case 1: // Single short
    vibes_short_pulse();
    break;
  case 2: // Double short
    vibes_double_pulse();
    break;
  case 3: // Triple
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {200, 100, 200, 100, 200},
      .num_segments = 5
    } );
  case 4: // Long
    vibes_long_pulse();
    break;
  case 5: // Subtle
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {50, 200, 50, 200, 50, 200, 50},
      .num_segments = 7
    } );
    break;
  case 6: // Less Subtle
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {100, 200, 100, 200, 100, 200, 100},
      .num_segments = 7
    } );
    break;
  case 7: // Not Subtle
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {500, 250, 500, 250, 500, 250, 500},
      .num_segments = 7
    } );
    break;
  case 8: // Not Subtle
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {100},
      .num_segments = 1
    } );
    break;	
  case 9: // Not Subtle
    vibes_enqueue_custom_pattern( (VibePattern) {
      .durations = (uint32_t []) {50},
      .num_segments = 1
    } );
    break;		
  default: // No Vibration
    return;
  }
}

void update_connection() {

  if (bluetooth_connected) {
//    generate_vibe(settings.vibe_pat_connect);  // non-op, by default
	is_inet_connected = true;
  } else {
//    generate_vibe(settings.vibe_pat_disconnect);  // because, this is bad...
	phone_battery_percent =0;
	is_inet_connected = false;
  }
  layer_mark_dirty(title_layer);
}

static void handle_bluetooth(bool connected) {
//  if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_bluetooth"); }
  bluetooth_connected = connected;
  if (isloading) {
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "handle_bluetooth isloading");
	return;
  }
  if(lostconnection != NULL){
	lostconnection = NULL;
  }else{
	if (settings.vibe_disconnect_delay>0){
		lostconnection = app_timer_register(settings.vibe_disconnect_delay*1000, &connection_handle, NULL);
	}else{
		connection_handle(NULL);
	}
  }
  update_connection();
}

static void window_load(Window *window) {

//	f20=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_20));
//	f40=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_40));
//	f48=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_48));
	f60=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_60));
	
	s20=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYMBOL_25));
	w25=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_32));
	w40=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_42));
	w60=fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_65));
 
  Layer *window_layer = window_get_root_layer(window);
//  layer_set_clips(window_layer,false);
  GRect bounds = layer_get_bounds(window_layer);
  
  title_layer = layer_create(GRect(0,0,DEVICE_WIDTH,26));
  layer_add_child(window_layer, title_layer);
//  layer_set_clips(title_layer,false);
  layer_set_update_proc(title_layer, title_layer_update_callback);
  
//  GRect stat_bounds = layer_get_bounds(statusbar);

 // slot_top = layer_create(GRect(0,LAYOUT_SLOT_TOP,DEVICE_WIDTH,LAYOUT_SLOT_BOT));
 // layer_set_update_proc(slot_top, slot_top_layer_update_callback);
 // layer_add_child(window_layer, slot_top);
//  GRect slot_top_bounds = layer_get_bounds(slot_top);

//  slot_bot = layer_create(GRect(0,LAYOUT_SLOT_BOT,DEVICE_WIDTH,DEVICE_HEIGHT));
//  layer_set_update_proc(slot_bot, slot_bot_layer_update_callback);
//  layer_add_child(window_layer, slot_bot);
//  GRect slot_bot_bounds = layer_get_bounds(slot_bot);
  
  
  weather_layer = layer_create(GRect(0,LAYOUT_SLOT_BOT,DEVICE_WIDTH,DEVICE_HEIGHT));//slot_bot_bounds);//GRect(0, 90, 144, 80));
  layer_add_child(window_layer/*slot_bot*/, weather_layer);  
  layer_set_update_proc(weather_layer, weather_layer_update_callback); 

/*  inverter_f41 = inverter_layer_create(GRect(44,2,50,30));
  layer_add_child(weather_layer, inverter_layer_get_layer(inverter_f41));
  layer_set_hidden(inverter_layer_get_layer(inverter_f41), true);
  inverter_f42 = inverter_layer_create(GRect(44,32,50,30));
  layer_add_child(weather_layer, inverter_layer_get_layer(inverter_f42));
  layer_set_hidden(inverter_layer_get_layer(inverter_f42), true);
  inverter_f43 = inverter_layer_create(GRect(93,2,50,30));
  layer_add_child(weather_layer, inverter_layer_get_layer(inverter_f43));
  layer_set_hidden(inverter_layer_get_layer(inverter_f43), true);
  inverter_f44 = inverter_layer_create(GRect(93,32,50,30));
  layer_add_child(weather_layer, inverter_layer_get_layer(inverter_f44));
  layer_set_hidden(inverter_layer_get_layer(inverter_f44), true);*/
  

  batt_charging = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
  batt_pluged = gbitmap_create_with_resource(RESOURCE_ID_PLUGED);
  

  datetime_layer = layer_create(GRect(0,LAYOUT_SLOT_TOP,DEVICE_WIDTH,LAYOUT_SLOT_BOT));//slot_top_bounds);
  layer_set_update_proc(datetime_layer, datetime_layer_update_callback);
  layer_add_child(window_layer/*slot_top*/, datetime_layer);

  calendar_layer = layer_create(GRect(0,LAYOUT_SLOT_BOT,DEVICE_WIDTH,DEVICE_HEIGHT));//slot_bot_bounds);
  layer_set_update_proc(calendar_layer, calendar_layer_update_callback);
  layer_add_child(window_layer/*slot_bot*/, calendar_layer);

//  date_layer = text_layer_create( GRect(REL_CLOCK_DATE_LEFT, REL_CLOCK_DATE_TOP, DEVICE_WIDTH, REL_CLOCK_DATE_HEIGHT) );
//  text_layer_set_text_color(date_layer, GColorWhite);
//  text_layer_set_background_color(date_layer, GColorClear);
//  text_layer_set_font(date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_20)));
//  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
//  layer_add_child(datetime_layer, text_layer_get_layer(date_layer));

 
//  time_layer = text_layer_create( GRect(REL_CLOCK_TIME_LEFT, REL_CLOCK_TIME_TOP, DEVICE_WIDTH, REL_CLOCK_TIME_HEIGHT) ); // see position_time_layer()
//  text_layer_set_text_color(time_layer, GColorWhite);
//  text_layer_set_background_color(time_layer, GColorClear);
//  text_layer_set_font(time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_48)));
//  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
//  position_time_layer(); // make use of our whitespace, if we have it...
//  layer_add_child(datetime_layer, text_layer_get_layer(time_layer));

//  week_layer = text_layer_create( GRect(4, REL_CLOCK_SUBTEXT_TOP, 140, 16) );
//  text_layer_set_text_color(week_layer, GColorWhite);
//  text_layer_set_background_color(week_layer, GColorClear);
//  text_layer_set_font(week_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
//  text_layer_set_text_alignment(week_layer, GTextAlignmentLeft);
//  layer_add_child(datetime_layer, text_layer_get_layer(week_layer));
//  if ( settings.show_week == 0 ) {
//    layer_set_hidden(text_layer_get_layer(week_layer), true);
//  }

//  day_layer = text_layer_create( GRect(28, REL_CLOCK_SUBTEXT_TOP, DEVICE_WIDTH - 56, 16) );
//  text_layer_set_text_color(day_layer, GColorWhite);
//  text_layer_set_background_color(day_layer, GColorClear);
//  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
//  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
//  layer_add_child(datetime_layer, text_layer_get_layer(day_layer));
//  if ( settings.show_day == 0 ) {
//    layer_set_hidden(text_layer_get_layer(day_layer), true);
//  }

//  ampm_layer = text_layer_create( GRect(0, REL_CLOCK_SUBTEXT_TOP, 140, 16) );
//  text_layer_set_text_color(ampm_layer, GColorWhite);
//  text_layer_set_background_color(ampm_layer, GColorClear);
//  text_layer_set_font(ampm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
//  text_layer_set_text_alignment(ampm_layer, GTextAlignmentRight);
//  layer_add_child(datetime_layer, text_layer_get_layer(ampm_layer));
//  if ( settings.show_am_pm == 0 ) {
//    layer_set_hidden(text_layer_get_layer(ampm_layer), true);
//  }

//  update_datetime_subtext();

 
  // topmost inverter layer, determines dark or light...
  inverter_layer = inverter_layer_create(bounds);
  if (settings.inverted==0) {
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
  }else{
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), false);  
  }
  layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
  
//  inverter_title = inverter_layer_create(GRect(0,0,DEVICE_WIDTH,24));
//  layer_add_child(window_layer, inverter_layer_get_layer(inverter_title));
//  layer_set_hidden(inverter_layer_get_layer(inverter_title), true);
  
   rotate_first_page();
//	layer_set_clips(window_layer,false);
}

static void window_unload(Window *window) {
//	if(f20!=NULL) fonts_unload_custom_font(f20);
//	if(f40!=NULL) fonts_unload_custom_font(f40);
	if(f60!=NULL) fonts_unload_custom_font(f60);
	if(s20!=NULL) fonts_unload_custom_font(s20);
	if(w25!=NULL) fonts_unload_custom_font(w25);
	if(w40!=NULL) fonts_unload_custom_font(w40);
	if(w60!=NULL) fonts_unload_custom_font(w60);
	
	battery_sending = NULL;
	auto_back_fp = NULL;
	at_battery_blink = NULL;
	accel_start = NULL;
	lostconnection = NULL;	
	
	
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Window unload BEGIN");};
    layer_destroy(weather_layer);  
  layer_destroy(inverter_layer_get_layer(inverter_layer));
//  layer_destroy(inverter_layer_get_layer(inverter_title));
  
/*  layer_destroy(inverter_layer_get_layer(inverter_f41));
  layer_destroy(inverter_layer_get_layer(inverter_f42));  
  layer_destroy(inverter_layer_get_layer(inverter_f43));
  layer_destroy(inverter_layer_get_layer(inverter_f44));  */
  
//  layer_destroy(text_layer_get_layer(ampm_layer));
//  layer_destroy(text_layer_get_layer(day_layer));
//  layer_destroy(text_layer_get_layer(week_layer));
//  layer_destroy(text_layer_get_layer(time_layer));
//  layer_destroy(text_layer_get_layer(date_layer));
  layer_destroy(calendar_layer);
  layer_destroy(datetime_layer);
  gbitmap_destroy(batt_charging);  
  
  gbitmap_destroy(batt_pluged);  
//  layer_destroy(slot_bot);
 // layer_destroy(slot_top);
  layer_destroy(title_layer);
//  if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Window unload END");};
  persist_write_int(PK_SELECT_PAGE, current_page);
}

static void deinit(void) {
  // deinit anything we init
	
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{

	if (DEBUGLOG) { 
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_minute_tick %d:%d",tick_time->tm_hour,tick_time->tm_min); 
	}
//	curTime = tick_time;
//		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_minute_tick %d:%d",curTime->tm_hour,curTime->tm_min); 
//  update_time_text();

//  if (units_changed & MONTH_UNIT) {
//    update_date_text();
//  }

  if (units_changed & HOUR_UNIT) {
//    request_timezone();
//    update_datetime_subtext();
    if (settings.vibe_hour) {
      generate_vibe(settings.vibe_hour);
    }
  }

    layer_mark_dirty(datetime_layer);
  if (units_changed & DAY_UNIT) {
//	request_timezone();
//    layer_mark_dirty(datetime_layer);
    layer_mark_dirty(calendar_layer);
  }

	if(wsample<10){
		if(weather_needs_update(weather, settings.weather_upd) && settings.first_page!=3 ){ // prefs->weather_update_freq))
//			if(weather_now_loading==false){
				if (is_inet_connected){
					wsample++;
					if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Weather request %d",wsample);}
					weather_request_update(settings.get_position,settings.showDayWeather,settings.type_position,settings.weatherPeriod);
				}
//			}
		}
	}else{
		if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Weather request error %d",wsample);}
		wsample=0;
		weather->last_update_time=time(NULL);
	}
  SleepModeRun();
  if (sleepmode3>0) sleepmode3=sleepmode3-1;
  // calendar gets redrawn every time because time_layer is changed and all layers are redrawn together.
}

void my_out_sent_handler(DictionaryIterator *sent, void *context) {
// outgoing message was delivered
}
void my_out_fail_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
// outgoing message failed
}

/*void in_timezone_handler(DictionaryIterator *received, void *context) {
    Tuple *tz_offset = dict_find(received, AK_TIMEZONE_OFFSET);
    if (tz_offset != NULL) {
      timezone_offset = tz_offset->value->int8;
    }
 // if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Timezone received: %d", timezone_offset); }
}*/

/*void set_pos_data_over_time(){
	if (settings.data_over_time==1){
		REL_CLOCK_DATE_LEFT		=0;
		REL_CLOCK_DATE_TOP		=-6;
		REL_CLOCK_DATE_HEIGHT	=30; // date/time overlap, due to the way text is 'positioned'
		REL_CLOCK_TIME_LEFT		=0;
		REL_CLOCK_TIME_TOP		=7;
		REL_CLOCK_TIME_HEIGHT	=60; // date/time overlap, due to the way text is 'positioned'
		REL_CLOCK_SUBTEXT_TOP	=56;	
	}else{
		REL_CLOCK_DATE_LEFT		=0;
		REL_CLOCK_DATE_TOP		=35;
		REL_CLOCK_DATE_HEIGHT	=30; // date/time overlap, due to the way text is 'positioned'
		REL_CLOCK_TIME_LEFT		=0;
		REL_CLOCK_TIME_TOP		=-10;
		REL_CLOCK_TIME_HEIGHT	=60; // date/time overlap, due to the way text is 'positioned'
		REL_CLOCK_SUBTEXT_TOP	=56;	
	}
//	layer_set_frame( text_layer_get_layer(time_layer), GRect(REL_CLOCK_TIME_LEFT, REL_CLOCK_TIME_TOP, DEVICE_WIDTH, REL_CLOCK_TIME_HEIGHT) );
//	layer_set_frame( text_layer_get_layer(date_layer), GRect(REL_CLOCK_DATE_LEFT, REL_CLOCK_DATE_TOP, DEVICE_WIDTH, REL_CLOCK_DATE_HEIGHT) );
}*/

void in_configuration_handler(DictionaryIterator *received, void *context) {
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function in_configuration_handler"); };
//	if (weather_in_cache()){
		weather=weather_load_cache();
//	}
    // style_inv == inverted
	Tuple *SleepMode = dict_find(received, AK_SLEEP_MODE);
	if(SleepMode !=NULL){
		settings.sleepmode = SleepMode->value->uint8;
	}
	
	Tuple *vibe_disconnect_delay = dict_find(received, AK_VIBE_DISCONNECT_DELAY);
	if(vibe_disconnect_delay !=NULL){
		settings.vibe_disconnect_delay = vibe_disconnect_delay->value->uint8;
	}
	
	Tuple *show_gmail = dict_find(received, AK_SHOW_GMAIL);
	if(show_gmail !=NULL){
		settings.show_gmail = show_gmail->value->uint8;
	}
	
	Tuple *show_phone_state = dict_find(received, AK_SHOW_PHONE_STATE);
	if( show_phone_state != NULL){
		settings.show_phone_state = show_phone_state->value->uint8;
	}
	
	
	Tuple *bat_prc_view = dict_find(received, AK_BAT_PRC_VIEW);
	if( bat_prc_view != NULL){
		settings.bat_cifr= bat_prc_view->value->uint8;
	}
	
	Tuple *select_page = dict_find(received, AK_SELECT_PAGE);
	if (select_page !=NULL){
		settings.select_page = select_page->value->uint8;
	}
	
	Tuple *sleepbegin = dict_find(received, AK_SLEEPBEGIN);
	if(sleepbegin != NULL){
		settings.sleepbegin=sleepbegin->value->uint8;
//		if (DEBUGLOG){app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "SleepBegin %d",settings.sleepbegin);}
	}
	Tuple *sleepend = dict_find(received, AK_SLEEPEND);
	if(sleepend != NULL){
		settings.sleepend=sleepend->value->uint8;
//		if (DEBUGLOG){app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "SleepEnd %d",settings.sleepend);}
	}
	
	
	Tuple *data_over_time = dict_find(received , AK_DATA_OVER_TIME);
	if(data_over_time != NULL)	{
		settings.data_over_time = data_over_time->value->uint8;
//		set_pos_data_over_time();
	}
	Tuple *trans_week = dict_find(received, AK_TRANS_WEEK);
	if(trans_week != NULL) {
		static char tr_week[5]="\0";
        snprintf(tr_week, sizeof(tr_week), "%s", trans_week->value->cstring);	
		strncpy(lang_gen.week, tr_week, sizeof(tr_week));
//		update_week_text(week_layer);
	}
	
	Tuple *show_bt_connected = dict_find(received , AK_SHOW_BT_CONNECTED);
	if(show_bt_connected != NULL)	{
		settings.show_bt_connected = show_bt_connected->value->uint8;
		update_connection();
	}
	
	Tuple *show_location = dict_find(received , AK_SHOW_LOCATION);
	if(show_location != NULL)	{
		settings.show_location = show_location->value->uint8;
//		if(settings.show_location==0){
//			layer_set_hidden(text_layer_get_layer(weather_name_layer), true);
//		}else{
//			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);		
//		}
	}
	
	Tuple *show_update_time = dict_find(received , AK_SHOW_UPDATE_TIME);
	if(show_update_time != NULL)	{
		settings.show_update_time = show_update_time->value->uint8;
		if(settings.show_update_time==1){
//			layer_set_hidden(text_layer_get_layer(weather_time_layer), true);
		}else{
//			layer_set_hidden(text_layer_get_layer(weather_time_layer), false);
		}
	}
	
	
    Tuple *style_inv = dict_find(received, AK_STYLE_INV);
    if (style_inv != NULL) {
      settings.inverted = style_inv->value->uint8;
      if (style_inv->value->uint8==0) {
        layer_set_hidden(inverter_layer_get_layer(inverter_layer), true); // hide inversion = dark
      } else {
        layer_set_hidden(inverter_layer_get_layer(inverter_layer), false); // show inversion = light
      }
    }
	
	// 
	Tuple *degree_style = dict_find(received, AK_DEGREE_STYLE);
	if (degree_style !=NULL) {
		settings.degree_style = degree_style->value->uint8;
//		is_weather_update=true;
//		layer_mark_dirty(weather_layer);
	}
	
	Tuple *first_page = dict_find(received, AK_FIRST_PAGE);
	if (first_page !=NULL) {
      settings.first_page = first_page->value->uint8;
//	  show_calendar = (settings.first_page==1)? true:false;

	  if(settings.first_page==0 || settings.first_page==1){
		if(settings.select_page==1){
			accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);	  
			accel_tap_service_subscribe(accel_int);
			accel_started=true;
			rotate_first_page();
		}else{
			if(accel_started){
				accel_tap_service_unsubscribe();
				accel_started=false;
			}
		}
	  }
	  if(settings.first_page==2 || settings.first_page==3){
		accel_tap_service_unsubscribe();
		accel_started=false;
		current_page=settings.first_page;
//		set_current_page(NULL);
	  }
//	  rotate_first_page();
//	  SleepModeRun();
	}
	
	Tuple *weather_upd = dict_find(received, AK_WEATHER_UPD);
	if (weather_upd !=NULL) {
      settings.weather_upd = weather_upd->value->uint32;
//	  if(!weather_needs_update(weather, settings.weather_upd))
//		is_weather_update=true;
//	    layer_mark_dirty(weather_layer);
//		update_weather_info(weather);	  
	  
	}
	
	Tuple *showDayWeather = dict_find(received, AK_SHOWDAYWEATHER);
	if (showDayWeather !=NULL) {
      settings.showDayWeather = showDayWeather->value->uint8;
/*	  if(!weather_needs_update(weather, settings.weather_upd))
		update_weather_info(weather);*/
	}
	
	Tuple *weatherPeriod = dict_find(received, AK_WEATHER_PERIOD);
	if(weatherPeriod !=NULL) {
		settings.weatherPeriod = weatherPeriod->value->uint8;
	}
	
	Tuple *battery_show = dict_find(received, AK_BATTERY_SHOW);
	if (battery_show !=NULL) {
      settings.battery_show = battery_show->value->uint32;
//	  show_battery_state();
//	  battery_percent=battery_percent+1;
//	  battery_status_send(NULL);
	  
	}
	
	Tuple *phone_battery_show = dict_find(received, AK_PHONE_BATTERY_SHOW);
	if (phone_battery_show !=NULL) {
      settings.phone_battery_show = phone_battery_show->value->uint32;
//	  show_battery_state();
	}
	
	Tuple *auto_back = dict_find(received, AK_AUTO_BACK);
	if (auto_back !=NULL) {
      settings.auto_back = auto_back->value->uint32;
//	  rotate_first_page();
	}

	
	Tuple *default_font = dict_find(received, AK_DEFAULT_FONT);
	if (default_font !=NULL) {
      settings.default_font = default_font->value->uint8;
//	  set_default_font();	  
	}
	
	Tuple *get_position = dict_find(received, AK_GET_POSITION);
	if (get_position !=NULL) {
      settings.get_position = get_position->value->uint8;
//	  set_default_font();	  
	}
	
	Tuple *type_position = dict_find(received, AK_TYPE_POSITION);
	if (type_position !=NULL) {
	  strncpy(settings.type_position,type_position->value->cstring,40);		  
		if (is_inet_connected){
			wsample=0;
			weather_request_update(settings.get_position,settings.showDayWeather,settings.type_position,settings.weatherPeriod);
		}
//		rotate_first_page();
	}

	
    // style_day_inv == day_invert
    Tuple *style_day_inv = dict_find(received, AK_STYLE_DAY_INV);
    if (style_day_inv != NULL) {
      settings.day_invert = style_day_inv->value->uint8;
    }

    // style_grid == grid
    Tuple *style_grid = dict_find(received, AK_STYLE_GRID);
    if (style_grid != NULL) {
      settings.grid = style_grid->value->uint8;
    }

    // AK_VIBE_HOUR == vibe_hour - vibration patterns for hourly vibration
    Tuple *vibe_hour = dict_find(received, AK_VIBE_HOUR);
    if (vibe_hour != NULL) {
      settings.vibe_hour = vibe_hour->value->uint8;
//	  layer_mark_dirty(title_layer);
//      if (settings.vibe_hour && (!battery_plugged || phone_battery_percent>0)) {
//        layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), false);
//        bitmap_layer_set_bitmap(bmp_charging_layer, image_hourvibe_icon);
//      } else if (!battery_charging || phone_battery_percent>0) {
//        layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
      //}
    }

    // INTL_DOWO == dayOfWeekOffset
    Tuple *INTL_DOWO = dict_find(received, AK_INTL_DOWO);
    if (INTL_DOWO != NULL) {
      settings.dayOfWeekOffset = INTL_DOWO->value->uint8;
    }

    // AK_INTL_FMT_DATE == date format (strftime + manual localization)
    Tuple *FMT_DATE = dict_find(received, AK_INTL_FMT_DATE);
    if (FMT_DATE != NULL) {
      settings.date_format = FMT_DATE->value->uint8;
//      update_date_text();
    }

    // AK_STYLE_WEEK
    Tuple *style_week = dict_find(received, AK_STYLE_WEEK);
    if (style_week != NULL) {
      settings.show_week = style_week->value->uint8;
/*      if ( settings.show_week ) {
        layer_set_hidden(text_layer_get_layer(week_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(week_layer), true);
      }*/
    }

    // AK_INTL_FMT_WEEK == week format (strftime)
    Tuple *FMT_WEEK = dict_find(received, AK_INTL_FMT_WEEK);
    if (FMT_WEEK != NULL) {
      settings.week_format = FMT_WEEK->value->uint8;
    }

    // AK_STYLE_DAY
    Tuple *style_day = dict_find(received, AK_STYLE_DAY);
    if (style_day != NULL) {
      settings.show_day = style_day->value->uint8;
 /*     if ( settings.show_day ) {
        layer_set_hidden(text_layer_get_layer(day_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(day_layer), true);
      }*/
    }

    // AK_STYLE_AM_PM
    Tuple *style_am_pm = dict_find(received, AK_STYLE_AM_PM);
    if (style_am_pm != NULL) {
      settings.show_am_pm = style_am_pm->value->uint8;
/*      if ( settings.show_am_pm ) {
        layer_set_hidden(text_layer_get_layer(ampm_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(ampm_layer), true);
      }*/
    }

    // now that we've received any changes, redraw the subtext (which processes week, day, and AM/PM)
//    update_datetime_subtext();

    // AK_VIBE_PAT_DISCONNECT / AK_VIBE_PAT_CONNECT == vibration patterns for connect and disconnect
    Tuple *VIBE_PAT_D = dict_find(received, AK_VIBE_PAT_DISCONNECT);
    if (VIBE_PAT_D != NULL) {
      settings.vibe_pat_disconnect = VIBE_PAT_D->value->uint8;
    }
    Tuple *VIBE_PAT_C = dict_find(received, AK_VIBE_PAT_CONNECT);
    if (VIBE_PAT_C != NULL) {
      settings.vibe_pat_connect = VIBE_PAT_C->value->uint8;
    }

    // begin translations...
    Tuple *translation;

    // AK_TRANS_ABBR_*DAY == abbrDaysOfWeek // localized Su Mo Tu We Th Fr Sa
    for (int i = AK_TRANS_ABBR_SUNDAY; i <= AK_TRANS_ABBR_SATURDAY; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
//		if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d", i); }		
        strncpy(lang_datetime.abbrDaysOfWeek[i - AK_TRANS_ABBR_SUNDAY], translation->value->cstring, sizeof(lang_datetime.abbrDaysOfWeek[i - AK_TRANS_ABBR_SUNDAY])-1);
      }
    }

    // AK_TRANS_*DAY == daysOfWeek // localized Sunday through Saturday, max ~11 characters
    for (int i = AK_TRANS_SUNDAY; i <= AK_TRANS_SATURDAY; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {

//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d",i); }		
        strncpy(lang_datetime.DaysOfWeek[i - AK_TRANS_SUNDAY], translation->value->cstring, sizeof(lang_datetime.DaysOfWeek[i - AK_TRANS_SUNDAY])-1);
      }
    }

    // AK_TEXT_MONTH == monthsOfYear // localized month names, max ~9 characters ('September' == practical display limit)
    for (int i = AK_TRANS_JANUARY; i <= AK_TRANS_DECEMBER; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d ", i); }		
        strncpy(lang_datetime.monthsNames[i - AK_TRANS_JANUARY], translation->value->cstring, sizeof(lang_datetime.monthsNames[i - AK_TRANS_JANUARY])-1);
      }
    }

	
    vibe_suppression = true;
    update_connection();
    vibe_suppression = false;

    // AK_TRANS_TIME_AM / AK_TRANS_TIME_PM == AM / PM text, e.g. "AM" "PM" :)
    for (int i = AK_TRANS_TIME_AM; i <= AK_TRANS_TIME_PM; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
        strncpy(lang_gen.abbrTime[i - AK_TRANS_TIME_AM], translation->value->cstring, sizeof(lang_gen.abbrTime[i - AK_TRANS_TIME_AM])-1);
      }
    }
    
	Tuple *getposition = dict_find(received, AK_GETPOSITION);
	if (getposition !=NULL){
		static char type_getposition[45]="\0";
        snprintf(type_getposition, sizeof(type_getposition), "%s\n", getposition->value->cstring);	
		strncpy(lang_gen.getposition, type_getposition, sizeof(type_getposition));
//		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "lang_gen getposition %s",lang_gen.getposition);};

	}
    // end translations...

//    int result = 0;
    /*result = */persist_write_data(PK_SETTINGS, &settings, sizeof(settings) );
//    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into settings", result); }
    /*result = */persist_write_data(PK_LANG_GEN, &lang_gen, sizeof(lang_gen) );
//    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_gen", result); }

    /*result = */persist_write_data(PK_LANG_DATETIME, &lang_datetime.abbrDaysOfWeek, sizeof(lang_datetime.abbrDaysOfWeek) );
//    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime %d", result,sizeof(lang_datetime.abbrDaysOfWeek)); }

    /*result = */persist_write_data(PK_LANG_DATETIME1, &lang_datetime.monthsNames, sizeof(lang_datetime.monthsNames) );
//    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime1 %d", result,sizeof(lang_datetime.monthsNames)); }

    /*result = */persist_write_data(PK_LANG_DATETIME2, &lang_datetime.DaysOfWeek, sizeof(lang_datetime.DaysOfWeek) );
//    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime2 %d", result,sizeof(lang_datetime.DaysOfWeek)); }
	
	
	
    // ==== Implemented SDK ====
    // Battery
    // Connected
    // Persistent Storage
    // Screenshot Operation
    // ==== Available in SDK ====
    // Accelerometer
    // App Focus ( does this apply to Timely? )
    // ==== Waiting on / SDK gaps ====
    // Magnetometer
    // PebbleKit JS - more accurate location data
    // ==== Interesting SDK possibilities ====
    // PebbleKit JS - more information from phone
    // ==== Future improvements ====
    // Positioning - top, bottom, etc.
//  if (1) { 
	show_battery_state();
	layer_mark_dirty(calendar_layer); 
//	}
//  if (1) { 
  layer_mark_dirty(datetime_layer); 
//  }
	layer_mark_dirty(weather_layer);
//	layer_mark_dirty(title_layer);
	  rotate_first_page();
	  SleepModeRun();
  //update_time_text(&currentTime);
}

void my_in_rcv_handler(DictionaryIterator *received, void *context) {
// incoming message received
//  Tuple *message_type = dict_find(received, AK_MESSAGE_TYPE);
  Tuple *set_weather = dict_find(received, SET_WEATHER_MSG_KEY);
	Tuple *set_preference = dict_find(received,SET_PREFERENCES_MSG_KEY); 
	Tuple *set_phone_bs = dict_find(received, 201);
	Tuple *set_phone_bs_chg = dict_find(received, 202);	
	Tuple *get_battery_state = dict_find(received,200);
	Tuple *status_pebble_battery_on_phone = dict_find(received,203);
	Tuple *status_pebble_cherging_on_phone = dict_find(received,204);
	Tuple *incom_sms_cnt = dict_find(received,205);
	Tuple *incom_calls_cnt = dict_find(received,206);	
	Tuple *set_phone_bs_plg = dict_find(received,207);
//	Tuple *smartstatus_battery = dict_find(received,64525);
	Tuple *tis_inet_connected = dict_find(received,208);
	Tuple *audioMS = dict_find(received, 209);
	Tuple *isalarmset = dict_find(received, 210);
	Tuple *alarmtime = dict_find(received, 211);
	Tuple *ismailcount = dict_find(received, 212);

	if(alarmtime !=NULL){
		strncpy(alarm_time, alarmtime->value->cstring,18);
//		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "alarmtime %s", alarm_time);
	}
	
	if (isalarmset !=NULL){
		alarm_set= isalarmset->value->uint8;
		layer_mark_dirty(title_layer);
	}
	if (ismailcount != NULL){
		mail_count= ismailcount->value->uint16;
		layer_mark_dirty(title_layer);
	}
	if (tis_inet_connected != NULL){
		if(tis_inet_connected->value->uint8 == 1){
			is_inet_connected = true;
		}else{
			is_inet_connected = false;
		}
		wsample = 0;
	}
	
/*	if (smartstatus_battery != NULL){
		phone_battery_percent = smartstatus_battery->value->uint8;
		show_battery_state();
	}*/
	
	if (audioMS != NULL){
//		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "audiomanagerstate %d", (int)audioMS->value->uint32);
		phoneAMS=audioMS->value->uint8;
		layer_mark_dirty(title_layer);
	}
	
	if (set_phone_bs !=NULL){
//		int phone_bs_value = set_phone_bs->value-> uint32;
		uint8_t phone_bs_value = set_phone_bs->value-> uint8;
		uint8_t phone_bs_chg = set_phone_bs_chg->value->uint8;
		
		phone_battery_percent=phone_bs_value;
		phone_battery_charging=false;
		if (phone_bs_chg==1){
			phone_battery_charging=true;
		}
		
		uint8_t po_phone_bs_value = status_pebble_battery_on_phone->value-> uint8;
		uint8_t po_phone_bs_chg = status_pebble_cherging_on_phone->value->uint8;
		
		pebble_on_phone_battery_percent=po_phone_bs_value;
		pebble_on_phone_battery_charging=false;
		if (po_phone_bs_chg==1){
			pebble_on_phone_battery_charging=true;
		}
		if (set_phone_bs_plg != NULL){
			if( set_phone_bs_plg->value->uint8==1){
				phone_battery_plugged = true;
			}else{
				phone_battery_plugged = false;
			}
		}
//		battery_status_send(NULL);
		show_battery_state();
	}
	
	if(incom_sms_cnt != NULL){
		sms_incom_count = incom_sms_cnt->value-> uint16;
		layer_mark_dirty(title_layer);
	}
	
	if(incom_calls_cnt != NULL){
		calls_incom_count = incom_calls_cnt->value-> uint16;
		layer_mark_dirty(title_layer);
	}
	
	
	if(get_battery_state !=NULL){
		battery_status_send(NULL);
	}
	
	if (set_preference !=NULL){
		in_configuration_handler(received, context);
	}
/*  if (message_type != NULL) {
    switch ( message_type->value->uint8 ) {
    case AK_TIMEZONE_OFFSET:
      in_timezone_handler(received, context);
      return;
    }
  } else {
  }*/
  	if(set_weather) {
		wsample=0; //???
		if(set_weather->value->uint8==1){
//			if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "my_in_recv_hand bf weather_set %s",weather->name);};
//			weather_now_loading=false;
			weather_set(weather, received);
//			is_weather_update=true;
			layer_mark_dirty(weather_layer);
			wsample=0;
		};
		if(set_weather->value->uint8==2){
			strcpy(weather->name, "Not Find Location\0");
		}
		if(set_weather->value->uint8==0){
			strcpy(weather->name, "No Connect\0");		
		}
		if(set_weather->value->uint8==3){
			strcpy(weather->name, "GPS error\0");
		}
		if(set_weather->value->uint8==5){
			strcpy(weather->name, "I-net Error\0");
		}
		if(set_weather->value->uint8==4){
//			weather_now_loading=false;
			weather_set(weather, received);
//			is_weather_update=true;
			layer_mark_dirty(weather_layer);
			wsample=0;
		}
	
	}
}

void my_in_drp_handler(AppMessageResult reason, void *context) {
// incoming message dropped
//  if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "AppMessage Dropped: %d", reason); }
}

static void app_message_init(void) {
  // Register message handlers
  app_message_register_inbox_received(my_in_rcv_handler);
  app_message_register_inbox_dropped(my_in_drp_handler);
  app_message_register_outbox_sent(my_out_sent_handler);
  app_message_register_outbox_failed(my_out_fail_handler);
  // Init buffers
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
//	prefs = preferences_load();
//	if(weather_in_cache()){
//		weather = weather_load_cache();
//	}

//	preferences_send(prefs);  
}

static void rotate_page_by_timer(void *data){
	rotate_first_page();
}
static void set_current_page() { 
//	setdatetimepos(LAYOUT_SLOT_TOP);
//	layer_set_frame(datetime_layer,GRect(0,LAYOUT_SLOT_TOP,DEVICE_WIDTH,LAYOUT_SLOT_BOT));
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "set current page %d",current_page);};
	if (current_page==settings.first_page){ auto_back_fp = NULL;}
	switch(current_page){
		case 0:
		case 3:
			{
				layer_set_hidden(calendar_layer, false);
				layer_set_hidden(weather_layer, true);	
				return;
			}
		case 1:
		case 2:
			{
				layer_set_hidden(weather_layer, false);
				layer_set_hidden(calendar_layer, true);
				return;
			}
	}	

}

void rotate_first_page(){
	if(settings.get_position==0 && strlen(settings.type_position)==0){
		settings.first_page=0;
	}
	current_page=settings.first_page;
	set_current_page(NULL);
}


void accel_int(AccelAxisType axis, int32_t direction){
	if (is_accel_stoped == false){
		if (issleepmode && settings.sleepmode==3 && sleepmode3<1){
			if (layer_get_hidden(calendar_layer)==true && layer_get_hidden(weather_layer)==true){
				rotate_first_page();
			}
			sleepmode3=1;
		}else{
		
//		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "start_timer is_accel_stop=false AccelAxisType = %d direction =%d",axis,(int)direction);};
			if(current_page==0){ current_page=1;}else{current_page=0;}
			if(settings.get_position==0 && strlen(settings.type_position)==0){
				current_page=0;
			}
			set_current_page(NULL);
			if (current_page!=settings.first_page && settings.auto_back!=0){
				auto_back_fp = app_timer_register(settings.auto_back*1000, &rotate_page_by_timer, NULL);
				if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "start_timer %d",(int)(settings.auto_back*1000));}
			}
			if (current_page == settings.first_page){
				auto_back_fp = NULL;
			}
		}
	} else {
		;
	}
}


static void init(void) {
//  curTime = get_time();
  app_message_init();



  curversion=__pbl_app_info.app_version.minor;
  int version=0;
  if (persist_exists(PK_VERSION)) {
	version=persist_read_int(PK_VERSION);
  }
  if (version==curversion){
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "load settings");
	if (persist_exists(PK_SETTINGS)) {
//		if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read settings");};
		persist_read_data(PK_SETTINGS, &settings, sizeof(settings) );
	}
	if (persist_exists(PK_LANG_GEN)) {
//		if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read lang");};
		persist_read_data(PK_LANG_GEN, &lang_gen, sizeof(lang_gen) );
	}
	if (persist_exists(PK_LANG_DATETIME)) {
//		if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.abbrDaysOfWeek));};
		persist_read_data(PK_LANG_DATETIME, &lang_datetime.abbrDaysOfWeek, sizeof(lang_datetime.abbrDaysOfWeek) );
	}
  
	if (persist_exists(PK_LANG_DATETIME1)) {
//		if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.monthsNames));};
		persist_read_data(PK_LANG_DATETIME1, &lang_datetime.monthsNames, sizeof(lang_datetime.monthsNames) );
	}
  
	if (persist_exists(PK_LANG_DATETIME2)) {
//		if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.DaysOfWeek));};
		persist_read_data(PK_LANG_DATETIME2, &lang_datetime.DaysOfWeek, sizeof(lang_datetime.DaysOfWeek) );
	}
  } else {
	if (DEBUGLOG){ app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "clear settings");}
	if (persist_exists(PK_SETTINGS)) persist_delete(PK_SETTINGS);	
	if (persist_exists(PK_LANG_GEN)) persist_delete(PK_LANG_GEN);
	if (persist_exists(PK_LANG_DATETIME)) persist_delete(PK_LANG_DATETIME);
	if (persist_exists(PK_LANG_DATETIME1)) persist_delete(PK_LANG_DATETIME1);
	if (persist_exists(PK_LANG_DATETIME2)) persist_delete(PK_LANG_DATETIME2);
	if (persist_exists(PK_WEATHER)) persist_delete(PK_WEATHER);
	persist_write_int(PK_VERSION, curversion);
  }
  

 
//  request_timezone();

  window = window_create();
//  window_set_click_config_provider(window,(ClickConfigProvider) click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  const bool animated = false;
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, animated);

  //update_time_text();

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  battery_state_service_subscribe(&handle_battery);
  handle_battery(battery_state_service_peek()); // initialize
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  handle_bluetooth(bluetooth_connection_service_peek()); // initialize
  

  if(settings.first_page==0 || settings.first_page==1){
	if(settings.select_page==1){
		accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
		accel_tap_service_subscribe(accel_int);
		accel_started=true;
	}
  }
  SleepModeRun();  
  current_page=settings.first_page;
  int pksp = 0;
  if (persist_exists(PK_SELECT_PAGE)) {
	if(settings.select_page == 2){
		if(settings.first_page==0 || settings.first_page==1){
			pksp=persist_read_int(PK_SELECT_PAGE);
			if(pksp==0){current_page=1;}
			if(pksp==1){current_page=0;}
//			app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "PK_SELECT_PAGE %d",pksp);		
			if (current_page!=settings.first_page && settings.auto_back!=0){
				auto_back_fp = app_timer_register(settings.auto_back*1000, &rotate_page_by_timer, NULL);
			}
		}
	}
  }
  set_current_page(NULL);
  
  vibe_suppression = false;
  
	weather = weather_load_cache();  
//	is_weather_update=true;
	layer_mark_dirty(weather_layer);

//	set_default_font();
	get_phone_battery_state = true;
//	set_pos_data_over_time();
}

int main(void) {
  init();
  isloading=false;
  app_event_loop();
  deinit();
}

