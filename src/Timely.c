#include <pebble.h>
#include "config.h"
//#include "preferences.h"
#include "weather.h"
#include "timely.h"
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

//typedef struct { char *name;
//} Test; 
 
static Window *window;

//static Layer *battery_layer;
static Layer *datetime_layer;
static TextLayer *date_layer;
static TextLayer *time_layer;
static TextLayer *week_layer;
static TextLayer *ampm_layer;
static TextLayer *day_layer;
static Layer *calendar_layer;
static Layer *statusbar;
static Layer *slot_top;
static Layer *slot_bot;


// weather
static Layer *weather_layer;
static TextLayer *weather_temperature_layer;
static BitmapLayer *weather_icon_layer;
static GBitmap *weather_icon_bitmap = NULL;
static TextLayer *weather_name_layer;
static TextLayer *weather_time_layer;
// ----

static BitmapLayer *bmp_connection_layer;
static GBitmap *image_connection_icon;
static GBitmap *image_noconnection_icon;
static BitmapLayer *bmp_charging_layer;
static GBitmap *image_charging_icon;
static GBitmap *image_hourvibe_icon;
//static TextLayer *text_connection_layer;
//static TextLayer *text_battery_layer;

static BitmapLayer *pp_battphone_status;
static BitmapLayer *pp_battpebble_status;
static GBitmap *battphone;
static GBitmap *battpebble;
static GBitmap *batt_charging;
static GBitmap *batt_pluged;

static BitmapLayer *sms_incom_layer;
static GBitmap *sms_incom;

static BitmapLayer *calls_incom_layer;
static GBitmap *calls_incom;


static InverterLayer *inverter_layer;
//static InverterLayer *battery_meter_layer;

// battery info, instantiate to 'worst scenario' to prevent false hopes
static uint8_t battery_percent = 10;
static bool battery_charging = false;

static uint8_t phone_battery_percent =0;
static bool phone_battery_charging = false;
static bool phone_battery_plugged = false;

static bool is_inet_connected = true;

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

static bool is_accel_stoped = false;

AppTimer *battery_sending = NULL;
AppTimer *auto_back_fp = NULL;
AppTimer *at_battery_blink = NULL;
AppTimer *accel_start = NULL;
// connected info
static bool bluetooth_connected = false;
// suppress vibration
static bool vibe_suppression = true;
static int8_t timezone_offset = 0;
static bool weather_now_loading = false;

// weather
Weather *weather;
//static Preferences *prefs;



GFont futura48;
GFont futura60;
GFont futura24;
GFont futura35;
GFont gothic24;
GFont gothic28;
GFont roboto49;

void rotate_first_page();
void accel_int(AccelAxisType axis, int32_t direction);
static void set_current_page();
// define the persistent storage key(s)
#define PERSIST_DATA_MAX_LENGTH 1024
#define   PK_SETTINGS      10
#define    PK_LANG_GEN      11
#define    PK_LANG_DATETIME 12	
#define    PK_LANG_DATETIME1 13	
#define    PK_LANG_DATETIME2 14	

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

#define AK_MESSAGE_TYPE          99
#define AK_SEND_BATT_PERCENT    100
#define AK_SEND_BATT_CHARGING   101
#define AK_SEND_BATT_PLUGGED    102
#define AK_TIMEZONE_OFFSET      103


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

// relative coordinates (relative to SLOTs)
static int REL_CLOCK_DATE_LEFT		=0;
static int REL_CLOCK_DATE_TOP		=-6;
static int REL_CLOCK_DATE_HEIGHT	=30; // date/time overlap, due to the way text is 'positioned'
static int REL_CLOCK_TIME_LEFT		=0;
static int REL_CLOCK_TIME_TOP		=7;
static int REL_CLOCK_TIME_HEIGHT	=60; // date/time overlap, due to the way text is 'positioned'
static int REL_CLOCK_SUBTEXT_TOP	=56; // time/ampm overlap, due to the way text is 'positioned'

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
};

persist_datetime_lang lang_datetime = {
  .abbrDaysOfWeek = { "Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб" },
  .monthsNames = { "ЯНВАРЬ", "ФЕВРАЛЬ", "МАРТ", "АПРЕЛЬ", "МАЙ", "ИЮНЬ", "ИЮЛЬ", "АВГУСТ", "СЕНТЯБРЬ", "ОКТЯБРЬ", "НОЯБРЬ", "ДЕКАБРЬ" },
  .DaysOfWeek = { "Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота" },
};

persist_general_lang lang_gen = {
//  .statuses = { "Соед", "НЕТ соед" },
  .abbrTime = { "AM", "PM" },
  .getposition = "поиск местоположения",
  .week = "Нд",
};

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

struct tm *get_time()
{
    time_t tt = time(0);
    return localtime(&tt);
}

// weather

/*void change_preferences(Preferences *old_prefs, Preferences *new_prefs) {
	if(old_prefs == NULL || old_prefs->temp_format != new_prefs->temp_format) {
		if(!weather_needs_update(weather, new_prefs->weather_update_freq))
			update_weather_info(weather);
	}
}*/
void set_default_font(){
	if(settings.default_font==0){ // digitsl
		text_layer_set_font(date_layer, futura24);
		text_layer_set_font(time_layer, futura48);
		text_layer_set_font(weather_temperature_layer, futura35);
	}
	if(settings.default_font==1){ // normal
		text_layer_set_font(date_layer, gothic24);		
		text_layer_set_font(time_layer, roboto49);
		text_layer_set_font(weather_temperature_layer, gothic28);
	}
	
}

void update_weather_info(Weather *weather) {
//	text_layer_set_text(weather_temperature_layer, "АВИ");
    if(weather->conditions % 1000) {
        static char temperature_text[8];
		int temperature = weather_convert_temperature(weather->temperature, settings.degree_style);//prefs->temp_format);
		
        snprintf(temperature_text, 8, "%d\u00B0", temperature);
        text_layer_set_text(weather_temperature_layer, temperature_text);
        
/*        if(10 <= temperature && temperature <= 99) {
            layer_set_frame(text_layer_get_layer(weather_temperature_layer), GRect(70, 19+3, 72, 80));
//            text_layer_set_font(weather_temperature_layer, futura_35);
        }
        else if((0 <= temperature && temperature <= 9) || (-9 <= temperature && temperature <= -1)) {
            layer_set_frame(text_layer_get_layer(weather_temperature_layer), GRect(70, 19, 72, 80));
//            text_layer_set_font(weather_temperature_layer, futura_40);
        }
        else if((100 <= temperature) || (-99 <= temperature && temperature <= -10)) {
            layer_set_frame(text_layer_get_layer(weather_temperature_layer), GRect(70, 19+3, 72, 80));
//            text_layer_set_font(weather_temperature_layer, futura_28);
        }
        else {
            layer_set_frame(text_layer_get_layer(weather_temperature_layer), GRect(70, 19+6, 72, 80));
//            text_layer_set_font(weather_temperature_layer, futura_25);
        }*/
        
        if(weather_icon_bitmap)
            gbitmap_destroy(weather_icon_bitmap);
        weather_icon_bitmap = gbitmap_create_with_resource(get_resource_for_weather_conditions(weather->conditions));
        bitmap_layer_set_bitmap(weather_icon_layer, weather_icon_bitmap);
		static char weather_name_text[40]="\0";
		if(settings.get_position==0){
        snprintf(weather_name_text, sizeof(weather_name_text), "%s (%s)", weather->name, weather->country);
		}
		if(settings.get_position==1){
        snprintf(weather_name_text, sizeof(weather_name_text), "%s (%s)(GPS)", weather->name, weather->country);
		}
		
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "location %s",weather_name_text);}
		text_layer_set_text(weather_name_layer,weather_name_text);//weather_name_text);

		static char time_text[] = "00:00";
		static char time_text1[20];
		char *time_format;
		if (clock_is_24h_style()) {
			time_format = "%R\n";
		} else {
			time_format = "%I:%M\n";
		}
//		int up_min;
		if (is_inet_connected){
//			up_min=settings.weather_upd / 60;
			strftime(time_text, sizeof(time_text), time_format, localtime(&weather->last_update_time));
//			snprintf(time_text1,20,"%s (%d)",time_text, up_min);
						snprintf(time_text1,20,"%s",time_text);
			if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "update %s",time_text);};
			text_layer_set_text(weather_time_layer,time_text1);
			if(settings.show_update_time==1){
				layer_set_hidden(text_layer_get_layer(weather_time_layer), false);
			}else{
				layer_set_hidden(text_layer_get_layer(weather_time_layer), true);
			}
		}else{
			text_layer_set_text(weather_time_layer,"Offline");
			layer_set_hidden(text_layer_get_layer(weather_time_layer), false);
		}

    }
}

uint32_t get_resource_for_weather_conditions(uint32_t conditions) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "get_resource_for_weather_conditions"); }
	bool is_day = conditions >= 1000;
    switch((conditions - (conditions % 100)) % 1000) {
        case 0:
            if (DEBUGLOG) {APP_LOG(APP_LOG_LEVEL_DEBUG, "Error getting data (conditions returned %d)", (int)conditions);};
            return RESOURCE_ID_ICON_CLOUD_ERROR;
        case 200:
            return RESOURCE_ID_WEATHER_THUNDER;
        case 300:
            return RESOURCE_ID_WEATHER_DRIZZLE;
        case 500:
            return RESOURCE_ID_WEATHER_RAIN;
        case 600:
            switch(conditions % 100) {
                case 611:
                    return RESOURCE_ID_WEATHER_SLEET;
                case 612:
                    return RESOURCE_ID_WEATHER_RAIN_SLEET;
                case 615:
                case 616:
                case 620:
                case 621:
                case 622:
                    return RESOURCE_ID_WEATHER_RAIN_SNOW;
            }
            return RESOURCE_ID_WEATHER_SNOW;
        case 700:
            switch(conditions % 100) {
                case 731:
                case 781:
                    return RESOURCE_ID_WEATHER_WIND;
            }
            return RESOURCE_ID_WEATHER_FOG;
        case 800:
            switch(conditions % 100) {
                case 0:
					if(is_day)
						return RESOURCE_ID_WEATHER_CLEAR_DAY;
					return RESOURCE_ID_WEATHER_CLEAR_NIGHT;
                case 1:
                case 2:
					if(is_day)
						return RESOURCE_ID_WEATHER_PARTLY_CLOUDY_DAY;
					return RESOURCE_ID_WEATHER_PARTLY_CLOUDY_NIGHT;
                case 3:
                case 4:
                    return RESOURCE_ID_WEATHER_CLOUDY;
            }
        case 900:
            switch(conditions % 100) {
                case 0:
                case 1:
                case 2:
                    return RESOURCE_ID_WEATHER_WIND;
                case 3:
                    return RESOURCE_ID_WEATHER_HOT;
                case 4:
                    return RESOURCE_ID_WEATHER_COLD;
                case 5:
                    return RESOURCE_ID_WEATHER_WIND;
                case 950:
                case 951:
                case 952:
                case 953:
					if(is_day)
						return RESOURCE_ID_WEATHER_CLEAR_DAY;
					return RESOURCE_ID_WEATHER_CLEAR_NIGHT;
                case 954:
                case 955:
                case 956:
                case 957:
                case 959:
                case 960:
                case 961:
                case 962:
                    return RESOURCE_ID_WEATHER_WIND;
            }
    }
    
    if (DEBUGLOG) {APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown wearther conditions: %d", (int)conditions);};
    return RESOURCE_ID_ICON_CLOUD_ERROR;
}

// -weather



void setColors(GContext* ctx) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function setColors"); }
    window_set_background_color(window, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorWhite);
}

void setInvColors(GContext* ctx) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function setInvColors"); }
    window_set_background_color(window, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_text_color(ctx, GColorBlack);
}

void calendar_layer_update_callback(Layer *me, GContext* ctx) {

    (void)me;
    struct tm *currentTime = get_time();

    int mon = currentTime->tm_mon;
    int year = currentTime->tm_year + 1900;
    int daysThisMonth = daysInMonth(mon, year);
    int specialDay = currentTime->tm_wday - settings.dayOfWeekOffset;
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
    int calendar[21];
    int cellNum = 0;   // address for current day table cell: 0-20
    int daysVisPrevMonth = 0;
    int daysVisNextMonth = 0;
    int daysPriorToToday = 7 + currentTime->tm_wday - settings.dayOfWeekOffset;
    int daysAfterToday   = 6 - currentTime->tm_wday + settings.dayOfWeekOffset;

    // tm_wday is based on Sunday being the startOfWeek, but Sunday may not be our startOfWeek.
    if (currentTime->tm_wday < settings.dayOfWeekOffset) { 
      if (show_last) {
        daysPriorToToday += 7; // we're <7, so in the 'first' week due to startOfWeek offset - 'add a week' before this one
      }
    } else {
      if (show_next) {
        daysAfterToday += 7;   // otherwise, we're already in the second week, so 'add a week' after
      }
    }

    if ( daysPriorToToday >= currentTime->tm_mday ) {
      // We're showing more days before today than exist this month
      int daysInPrevMonth = daysInMonth(mon - 1,year); // year only matters for February, which will be the same 'from' March

      // Number of days we'll show from the previous month
      daysVisPrevMonth = daysPriorToToday - currentTime->tm_mday + 1;

      for (int i = 0; i < daysVisPrevMonth; i++, cellNum++ ) {
        calendar[cellNum] = daysInPrevMonth + i - daysVisPrevMonth + 1;
      }
    }

    // optimization: instantiate i to a hot mess, since the first day we show this month may not be the 1st of the month
    int firstDayShownThisMonth = daysVisPrevMonth + currentTime->tm_mday - daysPriorToToday;
    for (int i = firstDayShownThisMonth; i < currentTime->tm_mday; i++, cellNum++ ) {
      calendar[cellNum] = i;
    }

    //int currentDay = cellNum; // the current day... we'll style this special
    calendar[cellNum] = currentTime->tm_mday;
    cellNum++;

    if ( currentTime->tm_mday + daysAfterToday > daysThisMonth ) {
      daysVisNextMonth = currentTime->tm_mday + daysAfterToday - daysThisMonth;
    }

    // add the days after today until the end of the month/next week, to our array...
    int daysLeftThisMonth = daysAfterToday - daysVisNextMonth;
    for (int i = 0; i < daysLeftThisMonth; i++, cellNum++ ) {
      calendar[cellNum] = i + currentTime->tm_mday + 1;
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

void update_date_text() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_date_text"); }
    struct tm *currentTime = get_time();

    // TODO - 18 @ this font is approaching the max width, localization may require smaller fonts, or no year...
    //September 11, 2013 => 18 chars
    //123456789012345678

    static char date_text[20];
    static char date_string[30];

    // http://www.cplusplus.com/reference/ctime/strftime/
    //strftime(date_text, sizeof(date_text), "%B %d, %Y", currentTime); // Month DD, YYYY {not localized}

    if (settings.date_format == 0) {
      // Month DD, YYYY (localized)
      strftime(date_text, sizeof(date_text), "%d, %Y", currentTime); // DD, YYYY
      snprintf(date_string, sizeof(date_string), "%s %s", lang_datetime.monthsNames[currentTime->tm_mon], date_text); // prefix Month (localized)
    } else if (settings.date_format == 1) {
      // DD.MM.YYYY
      strftime(date_text, sizeof(date_text), "%d.%m.%Y", currentTime);  // DD.MM.YYYY
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

    text_layer_set_text(date_layer, date_string);
}

void update_time_text() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_time_text"); }
  // Need to be static because used by the system later.
  static char time_text[] = "00:00";

  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  struct tm *currentTime = get_time();

  strftime(time_text, sizeof(time_text), time_format, currentTime);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  // I would love to just use clock_copy_time_string, but it refuses to center properly in 12-hour time (see Kludge above).
  //clock_copy_time_string(time_text, sizeof(time_text));
  text_layer_set_text(time_layer, time_text);

}

void update_day_text(TextLayer *which_layer) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_day_tex"); }
  struct tm *currentTime = get_time();
  text_layer_set_text(which_layer, lang_datetime.DaysOfWeek[currentTime->tm_wday]);
}

void update_month_text(TextLayer *which_layer) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_month_text"); }
  struct tm *currentTime = get_time();
  text_layer_set_text(which_layer, lang_datetime.monthsNames[currentTime->tm_mon]);
}

void update_week_text(TextLayer *which_layer) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_week_text"); }
  struct tm *currentTime = get_time();
  static char week_text[7] = "\0";
  static char week_num[3] = "\0";
  if (settings.week_format == 0) {
    // ISO 8601 week number (00-53)
	strftime(week_num, sizeof(week_num), "%V", currentTime);
  } else if (settings.week_format == 1) {
    // Week number with the first Sunday as the first day of week one (00-53)
	strftime(week_num, sizeof(week_num), "%U", currentTime);
  } else if (settings.week_format == 2) {
    // Week number with the first Monday as the first day of week one (00-53)
	strftime(week_num, sizeof(week_num), "%W", currentTime);
  }
  snprintf(week_text, sizeof(lang_gen.week), "%s",lang_gen.week);
//  strcat(week_text,lang_gen.week);
  strncat(week_text,week_num,2);
  if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "week_text %s", week_text);  }
  text_layer_set_text(which_layer, week_text);
}

void update_ampm_text(TextLayer *which_layer) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_ampm_text"); }
  struct tm *currentTime = get_time();

  if (currentTime->tm_hour < 12 ) {
    text_layer_set_text(which_layer, lang_gen.abbrTime[0]); //  0-11 AM
  } else {
    text_layer_set_text(which_layer, lang_gen.abbrTime[1]); // 12-23 PM
  }
}


void update_timezone_text(TextLayer *which_layer) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "update_timezone_text"); }
  static char timezone_text[7];
  if (timezone_offset > 0) {
    snprintf(timezone_text, sizeof(timezone_text), "GMT-%d", timezone_offset);
  } else {
    snprintf(timezone_text, sizeof(timezone_text), "GMT+%d", abs(timezone_offset));
  }
  text_layer_set_text(which_layer, timezone_text);
}

void process_show_week() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function process_shoe_week"); }
  switch ( settings.show_week ) {
  case 0: // Hide
    //layer_set_hidden(text_layer_get_layer(week_layer), true);
    return;
  case 1: // Show Week
    update_week_text(week_layer);
    break;
  case 2: // Show Timezone
    update_timezone_text(week_layer);
    break;
  case 3: // Show AM/PM
    update_ampm_text(week_layer);
    break;
  }
}

void process_show_day() {
//if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function process_show_days"); }
  switch ( settings.show_day ) {
  case 0: // Hide
    //layer_set_hidden(text_layer_get_layer(day_layer), true);
    return;
  case 1: // Show Day
    update_day_text(day_layer);
    break;
  case 2: // Show Month
    update_month_text(day_layer);
    break;
  case 3: // Show Timezone
    update_timezone_text(day_layer);
    break;
  case 4: // Show Week
    update_week_text(day_layer);
    break;
  case 5: // Show AM/PM
    update_ampm_text(day_layer);
    break;
  }
}

void process_show_ampm() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function process_show_ampm"); }
  switch ( settings.show_am_pm ) {
  case 0: // Hide
    //layer_set_hidden(text_layer_get_layer(ampm_layer), true);
    return;
  case 1: // Show AM/PM
    update_ampm_text(ampm_layer);
    break;
  case 2: // Show Timezone
    update_timezone_text(ampm_layer);
    break;
  case 3: // Show Week
    update_week_text(ampm_layer);
    break;
  }
}

void position_time_layer() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function position_time_layer"); }
  // potentially adjust the clock position, if we've added/removed the week, day, or AM/PM layers

	int time_offset = 0;
	int date_offset = 0;
	
	if(settings.default_font==0){
		if (!settings.show_day && !settings.show_week){
			text_layer_set_font(time_layer, futura60);  
		}else{
			text_layer_set_font(time_layer, futura48);  
		}
	}
	if(settings.default_font==1){
		text_layer_set_font(time_layer, roboto49);  
	}
	
	if (!settings.show_day && !settings.show_week) {
		if(settings.default_font==0){
			if (settings.data_over_time==0){
				time_offset=-8;
				date_offset=4;
			}
			if (settings.data_over_time==1){
				time_offset = -4;
				date_offset = 4;
			}
		}
		if(settings.default_font==1){
			if (settings.data_over_time==0){
				// +
			}
			if (settings.data_over_time==1){
				//+
			}
		}
	} else {
		if(settings.default_font==0){
			if (settings.data_over_time==0){
				//+
			}
			if (settings.data_over_time==1){
				//+
			}
		}
		if(settings.default_font==1){
			if (settings.data_over_time==0){
				date_offset =-2;
			}
			if (settings.data_over_time==1){
			}
		}
	}
	
	layer_set_frame( text_layer_get_layer(time_layer), GRect(REL_CLOCK_TIME_LEFT, REL_CLOCK_TIME_TOP + time_offset, DEVICE_WIDTH, REL_CLOCK_TIME_HEIGHT) );
	layer_set_frame( text_layer_get_layer(date_layer), GRect(REL_CLOCK_DATE_LEFT, REL_CLOCK_DATE_TOP + date_offset, DEVICE_WIDTH, REL_CLOCK_DATE_HEIGHT) );	
}

void update_datetime_subtext() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_datetime_subtext"); }
    process_show_week();
    process_show_day();
    process_show_ampm();
    position_time_layer();
}

void datetime_layer_update_callback(Layer *me, GContext* ctx) {

    (void)me;

    setColors(ctx);
    update_date_text();
    update_time_text();
    update_datetime_subtext();
}

void statusbar_layer_update_callback(Layer *me, GContext* ctx) {
// XXX positioning tests... only valid if we leave statusbar's frame/bounds set to the whole watch...
/*
    setColors(ctx);
    graphics_draw_rect(ctx, GRect(0,  0, 144, 24)); // statusbar
    graphics_draw_rect(ctx, GRect(0, 24, 144, 72)); // top half
    graphics_draw_rect(ctx, GRect(0, 96, 144, 72)); // bottom half
*
    graphics_draw_rect(ctx, GRect(0, 50, 20, 20)); // linked
    graphics_draw_rect(ctx, GRect(0, 72, 20, 20)); // icon 2
    graphics_draw_rect(ctx, GRect(0, 50, 10, 42)); // battery l
    graphics_draw_rect(ctx, GRect(144-10, 50, 10, 42)); // battery r
    graphics_draw_rect(ctx, GRect(144-20, 50, 20, 20)); // icon 3
    graphics_draw_rect(ctx, GRect(144-20, 72, 20, 20)); // icon 4
    graphics_draw_rect(ctx, GRect(0, 46, 144, 50)); // targeting time
*/
}

void slot_top_layer_update_callback(Layer *me, GContext* ctx) {
// TODO: configurable: draw appropriate slot
}

void slot_bot_layer_update_callback(Layer *me, GContext* ctx) {
// TODO: configurable: draw appropriate slot
}

/*void battery_layer_update_callback(Layer *me, GContext* ctx) {
// simply draw the battery outline here - the text is a different layer, and we then 'fill' it with an inverterLayer
  setColors(ctx);
// battery outline
  graphics_draw_rect(ctx, GRect(STAT_BATT_LEFT, STAT_BATT_TOP, STAT_BATT_WIDTH, STAT_BATT_HEIGHT));
// battery 'nib' terminal
  graphics_draw_rect(ctx, GRect(STAT_BATT_LEFT + STAT_BATT_WIDTH - 1,
                                STAT_BATT_TOP + (STAT_BATT_HEIGHT - STAT_BATT_NIB_HEIGHT)/2,
                                STAT_BATT_NIB_WIDTH,
                                STAT_BATT_NIB_HEIGHT));
}*/

static void request_timezone() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function request_timezone"); }
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if (iter == NULL) {
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "iterator is null: %d", result); }
    return;
  }
  if (dict_write_uint8(iter, AK_MESSAGE_TYPE, AK_TIMEZONE_OFFSET) != DICT_OK) {
    return;
  }
  app_message_outbox_send();
}

/*static void getPhoneBatteryState(){
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  long svbs=1;
  Tuplet value1 = TupletInteger(AK_SEND_PHONE_BATTERY_REQUEST,svbs);
  dict_write_tuplet(iter, &value1); 
  app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery sending request to phone");
  app_message_outbox_send();  
}*/


/*static void sendBatteryState(){
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "iterator is null: %d",result); 
    return;
  }
  
  if (result != APP_MSG_OK) {
    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed to open outbox: %d", (AppMessageResult) result); 
    return;
  }  
  long svbs=battery_percent;
  if(dict_write_int(iter,AK_SEND_BATTERY_VALUE,&svbs,sizeof(int),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_VALUE"); 
  }
  int sv = 0;
  if (battery_charging){ sv=1;};
  if(dict_write_int(iter,AK_SEND_BATTERY_CHARGING,&sv,sizeof(int),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); 
  }
  app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery sending %d %d ",battery_percent, sv);
  app_message_outbox_send();  
}*/

static void battery_status_send(void *data) {

//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery status send incom"); }
  if ( (battery_percent  == sent_battery_percent  )
     & (battery_charging == sent_battery_charging )
     & (battery_plugged  == sent_battery_plugged  ) 
	 & (pebble_on_phone_battery_percent == battery_percent)
	 & (pebble_on_phone_battery_charging == battery_charging)	 
	 ) {
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "repeat battery reading"); }
    battery_sending = NULL;
    return; // no need to resend the same value
  }
//  sendBatteryState();
  DictionaryIterator *iter;

  AppMessageResult result = app_message_outbox_begin(&iter);

  if (iter == NULL) {
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "iterator is null: %d", result); }
    return;
  }

  if (result != APP_MSG_OK) {
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed to open outbox: %d", (AppMessageResult) result); }
    return;
  }

  if (dict_write_int8(iter, SM_SCREEN_ENTER_KEY, STATUS_SCREEN_APP) != DICT_OK){
	return;
  }
  
  if (dict_write_uint8(iter, AK_MESSAGE_TYPE, AK_SEND_BATT_PERCENT) != DICT_OK) {
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
  }
  if(dict_write_int(iter,AK_SEND_BATTERY_VALUE,&battery_percent,sizeof(uint8_t),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_VALUE"); 
  }
  int sv = 0;
  if (battery_charging){ sv=1;};
  if(dict_write_int(iter,AK_SEND_BATTERY_CHARGING,&sv,sizeof(int),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); 
  }
  sv = 0;
  if (battery_plugged) {sv=1;};
  if(dict_write_int(iter,AK_SEND_BATTERY_PLUGGED,&sv,sizeof(int),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); 
  }
  
  if(dict_write_int(iter,AK_SEND_PHONE_BATTERY_REQUEST,&sv,sizeof(int),true)!=DICT_OK){
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Dict write failed AK_SEND_BATTERY_CHARGING"); 
  }  
  
  app_message_outbox_send();
  sent_battery_percent  = battery_percent;
  sent_battery_charging = battery_charging;
  sent_battery_plugged  = battery_plugged;
  battery_sending = NULL;
//  sendBatteryState();
//  getPhoneBatteryState();

}
static void fn_battery_blink(void *data){
	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery blink timer"); }
	battery_blink = !battery_blink;
	at_battery_blink = NULL;
	layer_mark_dirty(bitmap_layer_get_layer(pp_battpebble_status));
//	at_battery_blink = app_timer_register(1000, &fn_battery_blink, NULL);
}

void pp_battpebble_status_callback(Layer *me, GContext* ctx) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function pp_battpeble_status_calback"); }
	int prc=battery_percent;
	int hgth = (int)prc/(100/13);
	setColors(ctx);
	graphics_draw_bitmap_in_rect(ctx,battpebble,GRect(0,0,22,20));
	if(battery_blink){
		if (battery_charging && battery_plugged){
			graphics_draw_bitmap_in_rect(ctx,batt_charging,GRect(0,8,11,11));
		}else{
			if (battery_plugged){
				graphics_draw_bitmap_in_rect(ctx,batt_pluged,GRect(0,8,11,11));	
			}
		}
	}
	setInvColors(ctx);
	graphics_fill_rect(ctx, GRect (13,16-hgth,5,hgth), 0, GCornerNone);
	if (battery_charging || battery_plugged){	
		if(at_battery_blink == NULL){
			at_battery_blink = app_timer_register(1000, &fn_battery_blink, NULL);	
		}
	}else{
		at_battery_blink = NULL;	
	}
}

void pp_battphone_status_callback(Layer *me, GContext* ctx) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function ppbattphone_status_callback"); }
	int prc=phone_battery_percent;
	int hgth = (int)prc/(100/13);
	setColors(ctx);
	graphics_draw_bitmap_in_rect(ctx,battphone,GRect(0,0,22,20));
	if (phone_battery_charging && phone_battery_plugged){
		graphics_draw_bitmap_in_rect(ctx,batt_charging,GRect(0,8,11,11));
	}else{ 
		if (phone_battery_plugged){
			graphics_draw_bitmap_in_rect(ctx,batt_pluged,GRect(0,8,11,11));	
		}
	}	
	setInvColors(ctx);
	graphics_fill_rect(ctx, GRect (13,16-hgth,5,hgth), 0, GCornerNone);	
}

void incom_sms_show(){
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function incom_sms_show"); }
	if(sms_incom_count>0){
		layer_set_hidden(bitmap_layer_get_layer(sms_incom_layer),false);
	}else{
		layer_set_hidden(bitmap_layer_get_layer(sms_incom_layer),true);
	}
}

void incom_calls_show(){
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function incom_calls_show"); }
	if(calls_incom_count>0){
		layer_set_hidden(bitmap_layer_get_layer(calls_incom_layer),false);
	}else{
		layer_set_hidden(bitmap_layer_get_layer(calls_incom_layer),true);
	}
}


static void show_battery_state(){
//if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function show_battery_state"); }
//  static char battery_text[] = "100/100";
//  uint8_t battery_meter = battery_percent/10*(STAT_BATT_WIDTH-4)/10;
  // fill it in with current power
//  layer_set_bounds(inverter_layer_get_layer(battery_meter_layer), GRect(STAT_BATT_LEFT+2, STAT_BATT_TOP+2, battery_meter, STAT_BATT_HEIGHT-4));
//  layer_set_hidden(inverter_layer_get_layer(battery_meter_layer), false);

  if (battery_sending == NULL) {
    // multiple battery events can fire in rapid succession, we'll let it settle down before logging it
    battery_sending = app_timer_register(5000, &battery_status_send, NULL);
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "battery timer queued"); }
  }
  
 /* if(phone_battery_percent==0){
	if (battery_charging || phone_battery_charging) { // charging
		layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), false);
		bitmap_layer_set_bitmap(bmp_charging_layer, image_charging_icon);
	} else { // not charging
		if (battery_plugged) { // plugged but not charging = charging complete...
		layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
		} else { // normal wear
		if (settings.vibe_hour) {
			layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), false);
			bitmap_layer_set_bitmap(bmp_charging_layer, image_hourvibe_icon);
		} else {
			layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
		}
		}
	}
	layer_set_hidden(bitmap_layer_get_layer(pp_battphone_status), true);
	layer_set_hidden(bitmap_layer_get_layer(pp_battpebble_status), true);
  	  if(battery_percent<settings.battery_show){
		layer_set_hidden(text_layer_get_layer(text_battery_layer), false);
		layer_set_hidden(battery_layer, false);		
		layer_set_hidden(inverter_layer_get_layer(battery_meter_layer),false);
	  }else{
		layer_set_hidden(text_layer_get_layer(text_battery_layer), true);	  
		layer_set_hidden(battery_layer, true);				
		layer_set_hidden(inverter_layer_get_layer(battery_meter_layer),true);		
	  }
  } else{*/
		if (settings.vibe_hour) {
			layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), false);
			bitmap_layer_set_bitmap(bmp_charging_layer, image_hourvibe_icon);
		} else {
			layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
		}
  
	if(battery_percent<settings.battery_show || battery_charging || battery_plugged){
		layer_set_hidden(bitmap_layer_get_layer(pp_battpebble_status), false);
		layer_mark_dirty(bitmap_layer_get_layer(pp_battpebble_status));
	}else{
		layer_set_hidden(bitmap_layer_get_layer(pp_battpebble_status), true);	
	}
	if((phone_battery_percent<settings.phone_battery_show || phone_battery_charging || phone_battery_plugged) && phone_battery_percent>0 ){
		layer_set_hidden(bitmap_layer_get_layer(pp_battphone_status), false);
		layer_mark_dirty(bitmap_layer_get_layer(pp_battphone_status)); 
	}else{
		layer_set_hidden(bitmap_layer_get_layer(pp_battphone_status), true);	
	}
  
/*		layer_set_hidden(text_layer_get_layer(text_battery_layer), true);	  
		layer_set_hidden(battery_layer, true);				
		layer_set_hidden(inverter_layer_get_layer(battery_meter_layer),true);		
	
  }*/
	

//	snprintf(battery_text, sizeof(battery_text), "%d", battery_percent);  

//  text_layer_set_text(text_battery_layer, battery_text);
  
   
}



static void handle_battery(BatteryChargeState charge_state) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_battery"); }
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
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "accel_starting");
	is_accel_stoped = false;
	accel_start = NULL;
}

void generate_vibe(uint32_t vibe_pattern_number) {
  if (vibe_suppression) { return; }
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
  default: // No Vibration
//	is_accel_stoped = false;
    return;
  }
//  is_accel_stoped = false;
}

void update_connection() {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function update_connection"); }
//  text_layer_set_text(text_connection_layer, bluetooth_connected ? lang_gen.statuses[0] : lang_gen.statuses[1]) ;
  if (bluetooth_connected) {
    generate_vibe(settings.vibe_pat_connect);  // non-op, by default
    bitmap_layer_set_bitmap(bmp_connection_layer, image_connection_icon);
	is_inet_connected = true;
	if(settings.show_bt_connected==1){
		layer_set_hidden(bitmap_layer_get_layer(bmp_connection_layer), false);
	}else{
		layer_set_hidden(bitmap_layer_get_layer(bmp_connection_layer), true);	
	}
  } else {
  	layer_set_hidden(bitmap_layer_get_layer(bmp_connection_layer), false);
    generate_vibe(settings.vibe_pat_disconnect);  // because, this is bad...
    bitmap_layer_set_bitmap(bmp_connection_layer, image_noconnection_icon);
	layer_set_hidden(bitmap_layer_get_layer(sms_incom_layer), true);
	layer_set_hidden(bitmap_layer_get_layer(calls_incom_layer), true);	
	phone_battery_percent =0;
	is_inet_connected = false;
	show_battery_state();
  }
}

static void handle_bluetooth(bool connected) {
	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_bluetooth"); }
  bluetooth_connected = connected;
  update_connection();
}

static void window_load(Window *window) {

  futura48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_48));
  futura60 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_60));  
  futura24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_SEVEN_20));
  futura35 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_40));


   gothic24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);		
   roboto49 = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
   gothic28	= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_30));  

  

  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  
  // weather
  
    weather_layer = layer_create(GRect(0, 90, 144, 80));
    
    weather_icon_layer = bitmap_layer_create(GRect(9, 8, 60, 60));
    layer_add_child(weather_layer, bitmap_layer_get_layer(weather_icon_layer));
    
    weather_temperature_layer = text_layer_create(GRect(70, 12, 72, 80));
    text_layer_set_text_color(weather_temperature_layer, GColorWhite);
    text_layer_set_background_color(weather_temperature_layer, GColorClear);
    text_layer_set_font(weather_temperature_layer, futura35);
    text_layer_set_text_alignment(weather_temperature_layer, GTextAlignmentCenter);
    layer_add_child(weather_layer, text_layer_get_layer(weather_temperature_layer));
	text_layer_set_text(weather_temperature_layer, "");
	
	weather_name_layer = text_layer_create(GRect(0,60,144,15));
	text_layer_set_text_color(weather_name_layer, GColorWhite);
    text_layer_set_background_color(weather_name_layer, GColorClear);
//	text_layer_set_font(weather_name_layer, futura24);
	text_layer_set_font(weather_name_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(weather_name_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(weather_name_layer, GTextOverflowModeFill);
	layer_add_child(weather_layer, text_layer_get_layer(weather_name_layer));
	text_layer_set_text(weather_name_layer,lang_gen.getposition);
	
	if(settings.show_location==0){
		layer_set_hidden(text_layer_get_layer(weather_name_layer), true);
	}else{
		layer_set_hidden(text_layer_get_layer(weather_name_layer), false);	
	}
	
	
	weather_time_layer = text_layer_create(GRect(65,50,77,15));
	text_layer_set_text_color(weather_time_layer, GColorWhite);
    text_layer_set_background_color(weather_time_layer, GColorClear);
	text_layer_set_font(weather_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(weather_time_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(weather_time_layer, GTextOverflowModeFill);
	layer_add_child(weather_layer, text_layer_get_layer(weather_time_layer));
//	text_layer_set_text(weather_time_layer,"Обновл..");
	if(settings.show_update_time==1){
		layer_set_hidden(text_layer_get_layer(weather_time_layer), false);
	}else{
		layer_set_hidden(text_layer_get_layer(weather_time_layer), true);
	}
	
    layer_add_child(window_layer, weather_layer);  
  

  //statusbar = layer_create(GRect(0,LAYOUT_STAT,DEVICE_WIDTH,LAYOUT_SLOT_TOP));
  statusbar = layer_create(GRect(0,0,DEVICE_WIDTH,DEVICE_HEIGHT));
  layer_set_update_proc(statusbar, statusbar_layer_update_callback);
  layer_add_child(window_layer, statusbar);
//  GRect stat_bounds = layer_get_bounds(statusbar);

  slot_top = layer_create(GRect(0,LAYOUT_SLOT_TOP,DEVICE_WIDTH,LAYOUT_SLOT_BOT));
  layer_set_update_proc(slot_top, slot_top_layer_update_callback);
  layer_add_child(window_layer, slot_top);
  GRect slot_top_bounds = layer_get_bounds(slot_top);

  slot_bot = layer_create(GRect(0,LAYOUT_SLOT_BOT,DEVICE_WIDTH,DEVICE_HEIGHT));
  layer_set_update_proc(slot_bot, slot_bot_layer_update_callback);
  layer_add_child(window_layer, slot_bot);
  GRect slot_bot_bounds = layer_get_bounds(slot_bot);

  bmp_connection_layer = bitmap_layer_create( GRect(STAT_BT_ICON_LEFT, STAT_BT_ICON_TOP, 20, 20) );
  layer_add_child(statusbar, bitmap_layer_get_layer(bmp_connection_layer));
  image_connection_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_LINKED_ICON);
  image_noconnection_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_NOLINK_ICON);

  bmp_charging_layer = bitmap_layer_create( GRect(STAT_CHRG_ICON_LEFT, STAT_CHRG_ICON_TOP, 20, 20) );
  layer_add_child(statusbar, bitmap_layer_get_layer(bmp_charging_layer));
  image_charging_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_ICON);
  image_hourvibe_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOURVIBE_ICON);
  if (settings.vibe_hour) {
    bitmap_layer_set_bitmap(bmp_charging_layer, image_hourvibe_icon);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
  }

//  battery_layer = layer_create(stat_bounds);
//  layer_set_update_proc(battery_layer, battery_layer_update_callback);
//  layer_add_child(statusbar, battery_layer);

  sms_incom_layer = bitmap_layer_create(GRect(STAT_BT_ICON_LEFT+25, STAT_BT_ICON_TOP, 20, 20));
  layer_add_child(statusbar, bitmap_layer_get_layer(sms_incom_layer));
  sms_incom = gbitmap_create_with_resource(RESOURCE_ID_SMS_INCOM);
  bitmap_layer_set_bitmap(sms_incom_layer, sms_incom);
  layer_set_hidden(bitmap_layer_get_layer(sms_incom_layer), true);
  
  calls_incom_layer = bitmap_layer_create(GRect(STAT_BT_ICON_LEFT+45, STAT_BT_ICON_TOP, 20, 20));
  layer_add_child(statusbar, bitmap_layer_get_layer(calls_incom_layer));
  calls_incom = gbitmap_create_with_resource(RESOURCE_ID_CALLS_INCOM);
  bitmap_layer_set_bitmap(calls_incom_layer, calls_incom);
  layer_set_hidden(bitmap_layer_get_layer(calls_incom_layer), true);  
  
  pp_battphone_status = bitmap_layer_create( GRect(STAT_BATT_LEFT, STAT_BATT_TOP, 22, 20) );
  layer_set_update_proc(bitmap_layer_get_layer(pp_battphone_status), pp_battphone_status_callback);
  pp_battpebble_status = bitmap_layer_create( GRect(STAT_BATT_LEFT+22, STAT_BATT_TOP, 22, 20) );  
  layer_set_update_proc(bitmap_layer_get_layer(pp_battpebble_status), pp_battpebble_status_callback);
  
  
  
  layer_add_child(statusbar, bitmap_layer_get_layer(pp_battphone_status));
  layer_add_child(statusbar, bitmap_layer_get_layer(pp_battpebble_status));  
  layer_set_hidden(bitmap_layer_get_layer(pp_battphone_status), true);
  layer_set_hidden(bitmap_layer_get_layer(pp_battpebble_status), true);
  battphone = gbitmap_create_with_resource(RESOURCE_ID_BAT_PHONE);
  battpebble = gbitmap_create_with_resource(RESOURCE_ID_BAT_PEBBLE);
  batt_charging = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
  batt_pluged = gbitmap_create_with_resource(RESOURCE_ID_PLUGED);
//  bitmap_layer_set_bitmap(pp_battphone_status, battphone);
//  bitmap_layer_set_bitmap(pp_battpebble_status, battpebble);
  

  datetime_layer = layer_create(slot_top_bounds);
  layer_set_update_proc(datetime_layer, datetime_layer_update_callback);
  layer_add_child(slot_top, datetime_layer);

  calendar_layer = layer_create(slot_bot_bounds);
  layer_set_update_proc(calendar_layer, calendar_layer_update_callback);
  layer_add_child(slot_bot, calendar_layer);

  date_layer = text_layer_create( GRect(REL_CLOCK_DATE_LEFT, REL_CLOCK_DATE_TOP, DEVICE_WIDTH, REL_CLOCK_DATE_HEIGHT) );
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, futura24);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(datetime_layer, text_layer_get_layer(date_layer));


  
  time_layer = text_layer_create( GRect(REL_CLOCK_TIME_LEFT, REL_CLOCK_TIME_TOP, DEVICE_WIDTH, REL_CLOCK_TIME_HEIGHT) ); // see position_time_layer()
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, futura48);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  position_time_layer(); // make use of our whitespace, if we have it...
  layer_add_child(datetime_layer, text_layer_get_layer(time_layer));

  week_layer = text_layer_create( GRect(4, REL_CLOCK_SUBTEXT_TOP, 140, 16) );
  text_layer_set_text_color(week_layer, GColorWhite);
  text_layer_set_background_color(week_layer, GColorClear);
  text_layer_set_font(week_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(week_layer, GTextAlignmentLeft);
  layer_add_child(datetime_layer, text_layer_get_layer(week_layer));
  if ( settings.show_week == 0 ) {
    layer_set_hidden(text_layer_get_layer(week_layer), true);
  }

  day_layer = text_layer_create( GRect(28, REL_CLOCK_SUBTEXT_TOP, DEVICE_WIDTH - 56, 16) );
  text_layer_set_text_color(day_layer, GColorWhite);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  layer_add_child(datetime_layer, text_layer_get_layer(day_layer));
  if ( settings.show_day == 0 ) {
    layer_set_hidden(text_layer_get_layer(day_layer), true);
  }

  ampm_layer = text_layer_create( GRect(0, REL_CLOCK_SUBTEXT_TOP, 140, 16) );
  text_layer_set_text_color(ampm_layer, GColorWhite);
  text_layer_set_background_color(ampm_layer, GColorClear);
  text_layer_set_font(ampm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(ampm_layer, GTextAlignmentRight);
  layer_add_child(datetime_layer, text_layer_get_layer(ampm_layer));
  if ( settings.show_am_pm == 0 ) {
    layer_set_hidden(text_layer_get_layer(ampm_layer), true);
  }

  update_datetime_subtext();

  //text_connection_layer = text_layer_create( GRect(20+STAT_BT_ICON_LEFT, 0, 72, 22) );
  //text_layer_set_text_color(text_connection_layer, GColorWhite);
  //text_layer_set_background_color(text_connection_layer, GColorClear);
  //text_layer_set_font(text_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  //text_layer_set_text_alignment(text_connection_layer, GTextAlignmentLeft);
  //text_layer_set_text(text_connection_layer, "NO LINK");
  //layer_add_child(statusbar, text_layer_get_layer(text_connection_layer));

 // text_battery_layer = text_layer_create( GRect(STAT_BATT_LEFT, STAT_BATT_TOP-2, STAT_BATT_WIDTH, STAT_BATT_HEIGHT) );
//  text_layer_set_text_color(text_battery_layer, GColorWhite);
//  text_layer_set_background_color(text_battery_layer, GColorClear);
//  text_layer_set_font(text_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
//  text_layer_set_text_alignment(text_battery_layer, GTextAlignmentCenter);
//  text_layer_set_text(text_battery_layer, "?");

//  layer_add_child(statusbar, text_layer_get_layer(text_battery_layer));

  // NOTE: No more adding layers below here - the inverter layers NEED to be the last to be on top!

  // hide battery meter, until we can fix the size/position later when subscribing
//  battery_meter_layer = inverter_layer_create(stat_bounds);
//  layer_set_hidden(inverter_layer_get_layer(battery_meter_layer), true);
//  layer_add_child(statusbar, inverter_layer_get_layer(battery_meter_layer));

  // topmost inverter layer, determines dark or light...
  inverter_layer = inverter_layer_create(bounds);
  if (settings.inverted==0) {
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), true);
  }else{
    layer_set_hidden(inverter_layer_get_layer(inverter_layer), false);  
  }
  layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
  
  layer_set_hidden(calendar_layer, false);
  layer_set_hidden(weather_layer, true);	
//	change_preferences(NULL, prefs);
   rotate_first_page();
}

static void window_unload(Window *window) {
  // unload anything we loaded, destroy anything we created, remove anything we added

  if(weather_icon_bitmap)
        gbitmap_destroy(weather_icon_bitmap);
    text_layer_destroy(weather_temperature_layer);
	text_layer_destroy(weather_name_layer);
	text_layer_destroy(weather_time_layer);
    bitmap_layer_destroy(weather_icon_layer);
    layer_destroy(weather_layer);  
  
  fonts_unload_custom_font(futura48);	
  fonts_unload_custom_font(futura60);	
  fonts_unload_custom_font(futura24);  
  fonts_unload_custom_font(futura35);  
//  fonts_unload_custom_font(gothic24);
  fonts_unload_custom_font(gothic28);
//  fonts_unload_custom_font(roboto49);

  
  layer_destroy(inverter_layer_get_layer(inverter_layer));
//  layer_destroy(inverter_layer_get_layer(battery_meter_layer));
//  layer_destroy(text_layer_get_layer(text_battery_layer));
//  layer_destroy(text_layer_get_layer(text_connection_layer));
  layer_destroy(text_layer_get_layer(ampm_layer));
  layer_destroy(text_layer_get_layer(day_layer));
  layer_destroy(text_layer_get_layer(week_layer));
  layer_destroy(text_layer_get_layer(time_layer));
  layer_destroy(text_layer_get_layer(date_layer));
  layer_destroy(calendar_layer);
  layer_destroy(datetime_layer);
//  layer_destroy(battery_layer);
  layer_remove_from_parent(bitmap_layer_get_layer(bmp_charging_layer));
  layer_remove_from_parent(bitmap_layer_get_layer(bmp_connection_layer));
  bitmap_layer_destroy(bmp_charging_layer);
  bitmap_layer_destroy(bmp_connection_layer);
  bitmap_layer_destroy(sms_incom_layer);
  bitmap_layer_destroy(calls_incom_layer);
  gbitmap_destroy(image_connection_icon);
  gbitmap_destroy(image_noconnection_icon);
  gbitmap_destroy(image_charging_icon);
  gbitmap_destroy(image_hourvibe_icon);
  gbitmap_destroy(batt_charging);  
  gbitmap_destroy(sms_incom);  
  gbitmap_destroy(calls_incom);    
  
  gbitmap_destroy(batt_pluged);  
  bitmap_layer_destroy(pp_battphone_status);
  bitmap_layer_destroy(pp_battpebble_status);
  gbitmap_destroy(battphone);
  gbitmap_destroy(battpebble);
  layer_destroy(slot_bot);
  layer_destroy(slot_top);
  layer_destroy(statusbar);
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
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function handle_minute_tick"); }
  update_time_text();

  if (units_changed & MONTH_UNIT) {
    update_date_text();
  }

  if (units_changed & HOUR_UNIT) {
    request_timezone();
    update_datetime_subtext();
    if (settings.vibe_hour) {
      generate_vibe(settings.vibe_hour);
    }
  }

  if (units_changed & DAY_UNIT) {
    layer_mark_dirty(datetime_layer);
    layer_mark_dirty(calendar_layer);
  }

  // weather
  time_t now = time(NULL);

  if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "HMT: %d %d", (int)settings.weather_upd,(int)(now - weather->last_update_time));};
  if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "settings.type_position %s", settings.type_position);  };
    if(weather_needs_update(weather, settings.weather_upd)){ // prefs->weather_update_freq))
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "WAN HMT: %d", (int)settings.weather_upd);};
		if(weather_now_loading==false){
//			weather_now_loading=true;
			if (is_inet_connected){
				weather_request_update(settings.get_position,settings.type_position);
			}
		}
  }
//  if (get_phone_battery_state){
//	get_phone_battery_state=false;
//	sendBatteryState();
//	getPhoneBatteryState();
//  }
  // calendar gets redrawn every time because time_layer is changed and all layers are redrawn together.
}

void my_out_sent_handler(DictionaryIterator *sent, void *context) {
// outgoing message was delivered
}
void my_out_fail_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
// outgoing message failed
  /*if (DEBUGLOG)*/ { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "AppMessage Failed to Send: %d", reason); }
}

void in_timezone_handler(DictionaryIterator *received, void *context) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function in_timezone_handle"); }
    Tuple *tz_offset = dict_find(received, AK_TIMEZONE_OFFSET);
    if (tz_offset != NULL) {
      timezone_offset = tz_offset->value->int8;
    }
  if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Timezone received: %d", timezone_offset); }
}

void set_pos_data_over_time(){
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function set_pos_data_over_time"); }
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
	layer_set_frame( text_layer_get_layer(time_layer), GRect(REL_CLOCK_TIME_LEFT, REL_CLOCK_TIME_TOP, DEVICE_WIDTH, REL_CLOCK_TIME_HEIGHT) );
	layer_set_frame( text_layer_get_layer(date_layer), GRect(REL_CLOCK_DATE_LEFT, REL_CLOCK_DATE_TOP, DEVICE_WIDTH, REL_CLOCK_DATE_HEIGHT) );
}

void in_configuration_handler(DictionaryIterator *received, void *context) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function in_configuration_handler"); }
	if (weather_in_cache()){
		weather=weather_load_cache();
	}
    // style_inv == inverted
	
	Tuple *data_over_time = dict_find(received , AK_DATA_OVER_TIME);
	if(data_over_time != NULL)	{
		settings.data_over_time = data_over_time->value->uint8;
		set_pos_data_over_time();
	}
	Tuple *trans_week = dict_find(received, AK_TRANS_WEEK);
	if(trans_week != NULL) {
		static char tr_week[5]="\0";
        snprintf(tr_week, sizeof(tr_week), "%s", trans_week->value->cstring);	
		strncpy(lang_gen.week, tr_week, sizeof(tr_week));
		update_week_text(week_layer);
	}
	
	Tuple *show_bt_connected = dict_find(received , AK_SHOW_BT_CONNECTED);
	if(show_bt_connected != NULL)	{
		settings.show_bt_connected = show_bt_connected->value->uint8;
		update_connection();
	}
	
	Tuple *show_location = dict_find(received , AK_SHOW_LOCATION);
	if(show_location != NULL)	{
		settings.show_location = show_location->value->uint8;
		if(settings.show_location==0){
			layer_set_hidden(text_layer_get_layer(weather_name_layer), true);
		}else{
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);		
		}
	}
	
	Tuple *show_update_time = dict_find(received , AK_SHOW_UPDATE_TIME);
	if(show_update_time != NULL)	{
		settings.show_update_time = show_update_time->value->uint8;
		if(settings.show_update_time==1){
			layer_set_hidden(text_layer_get_layer(weather_time_layer), true);
		}else{
			layer_set_hidden(text_layer_get_layer(weather_time_layer), false);
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
      if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "degree_style %d",settings.degree_style);	};
	  if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "location %s",weather->name);};
      update_weather_info(weather);	  
	}
	
	Tuple *first_page = dict_find(received, AK_FIRST_PAGE);
	if (first_page !=NULL) {
      settings.first_page = first_page->value->uint8;
//	  show_calendar = (settings.first_page==1)? true:false;
	  if(settings.first_page==0 || settings.first_page==1){
		accel_tap_service_subscribe(accel_int);
		rotate_first_page();
	  }
	  if(settings.first_page==2 || settings.first_page==3){
		accel_tap_service_unsubscribe();
		current_page=settings.first_page;
		set_current_page(NULL);
	  }
	}
	
	Tuple *weather_upd = dict_find(received, AK_WEATHER_UPD);
	if (weather_upd !=NULL) {
      settings.weather_upd = weather_upd->value->uint32;
	  if(!weather_needs_update(weather, settings.weather_upd))
		update_weather_info(weather);	  
	  
	}
	
	Tuple *battery_show = dict_find(received, AK_BATTERY_SHOW);
	if (battery_show !=NULL) {
      settings.battery_show = battery_show->value->uint32;
	  show_battery_state();
//	  battery_percent=battery_percent+1;
//	  battery_status_send(NULL);
	  
	}
	
	Tuple *phone_battery_show = dict_find(received, AK_PHONE_BATTERY_SHOW);
	if (phone_battery_show !=NULL) {
      settings.phone_battery_show = phone_battery_show->value->uint32;
	  show_battery_state();
	}
	
	Tuple *auto_back = dict_find(received, AK_AUTO_BACK);
	if (auto_back !=NULL) {
      settings.auto_back = auto_back->value->uint32;
	  rotate_first_page();
	}

	
	Tuple *default_font = dict_find(received, AK_DEFAULT_FONT);
	if (default_font !=NULL) {
      settings.default_font = default_font->value->uint8;
	  set_default_font();	  
	}
	
	Tuple *get_position = dict_find(received, AK_GET_POSITION);
	if (get_position !=NULL) {
      settings.get_position = get_position->value->uint8;
//	  set_default_font();	  
	}
	
	Tuple *type_position = dict_find(received, AK_TYPE_POSITION);
	if (type_position !=NULL) {
	  strncpy(settings.type_position,type_position->value->cstring,40);		  
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "type_position %s",settings.type_position);};
		if (is_inet_connected){
			weather_request_update(settings.get_position,settings.type_position);
		}
		rotate_first_page();
//	  set_default_font();	  
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
      if (settings.vibe_hour && (!battery_plugged || phone_battery_percent>0)) {
        layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), false);
        bitmap_layer_set_bitmap(bmp_charging_layer, image_hourvibe_icon);
      } else if (!battery_charging || phone_battery_percent>0) {
        layer_set_hidden(bitmap_layer_get_layer(bmp_charging_layer), true);
      }
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
      update_date_text();
    }

    // AK_STYLE_WEEK
    Tuple *style_week = dict_find(received, AK_STYLE_WEEK);
    if (style_week != NULL) {
      settings.show_week = style_week->value->uint8;
      if ( settings.show_week ) {
        layer_set_hidden(text_layer_get_layer(week_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(week_layer), true);
      }
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
      if ( settings.show_day ) {
        layer_set_hidden(text_layer_get_layer(day_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(day_layer), true);
      }
    }

    // AK_STYLE_AM_PM
    Tuple *style_am_pm = dict_find(received, AK_STYLE_AM_PM);
    if (style_am_pm != NULL) {
      settings.show_am_pm = style_am_pm->value->uint8;
      if ( settings.show_am_pm ) {
        layer_set_hidden(text_layer_get_layer(ampm_layer), false);
      }  else {
        layer_set_hidden(text_layer_get_layer(ampm_layer), true);
      }
    }

    // now that we've received any changes, redraw the subtext (which processes week, day, and AM/PM)
    update_datetime_subtext();

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
//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
		if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d", i); }		
        strncpy(lang_datetime.abbrDaysOfWeek[i - AK_TRANS_ABBR_SUNDAY], translation->value->cstring, sizeof(lang_datetime.abbrDaysOfWeek[i - AK_TRANS_ABBR_SUNDAY])-1);
      }
    }

    // AK_TRANS_*DAY == daysOfWeek // localized Sunday through Saturday, max ~11 characters
    for (int i = AK_TRANS_SUNDAY; i <= AK_TRANS_SATURDAY; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d",i); }		
        strncpy(lang_datetime.DaysOfWeek[i - AK_TRANS_SUNDAY], translation->value->cstring, sizeof(lang_datetime.DaysOfWeek[i - AK_TRANS_SUNDAY])-1);
      }
    }

    // AK_TEXT_MONTH == monthsOfYear // localized month names, max ~9 characters ('September' == practical display limit)
    for (int i = AK_TRANS_JANUARY; i <= AK_TRANS_DECEMBER; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
//        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d ", i); }		
        strncpy(lang_datetime.monthsNames[i - AK_TRANS_JANUARY], translation->value->cstring, sizeof(lang_datetime.monthsNames[i - AK_TRANS_JANUARY])-1);
      }
    }

    // AK_TRANS_CONNECTED / AK_TRANS_DISCONNECTED == status text, e.g. "Linked" "NOLINK"
/*    for (int i = AK_TRANS_CONNECTED; i <= AK_TRANS_DISCONNECTED; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
        strncpy(lang_gen.statuses[i - AK_TRANS_CONNECTED], translation->value->cstring, sizeof(lang_gen.statuses[i - AK_TRANS_CONNECTED])-1);
      }
    }*/
	
	
    vibe_suppression = true;
    update_connection();
    vibe_suppression = false;

    // AK_TRANS_TIME_AM / AK_TRANS_TIME_PM == AM / PM text, e.g. "AM" "PM" :)
    for (int i = AK_TRANS_TIME_AM; i <= AK_TRANS_TIME_PM; i++ ) {
      translation = dict_find(received, i);
      if (translation != NULL) {
        if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "translation for key %d is %s", i, translation->value->cstring); }
        strncpy(lang_gen.abbrTime[i - AK_TRANS_TIME_AM], translation->value->cstring, sizeof(lang_gen.abbrTime[i - AK_TRANS_TIME_AM])-1);
      }
    }
    
	Tuple *getposition = dict_find(received, AK_GETPOSITION);
	if (getposition !=NULL){
		static char type_getposition[45]="\0";
        snprintf(type_getposition, sizeof(type_getposition), "%s\n", getposition->value->cstring);	
		strncpy(lang_gen.getposition, type_getposition, sizeof(type_getposition));
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "lang_gen getposition %s",lang_gen.getposition);};

	}
    // end translations...

    int result = 0;
    result = persist_write_data(PK_SETTINGS, &settings, sizeof(settings) );
    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into settings", result); }
    result = persist_write_data(PK_LANG_GEN, &lang_gen, sizeof(lang_gen) );
    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_gen", result); }

    result = persist_write_data(PK_LANG_DATETIME, &lang_datetime.abbrDaysOfWeek, sizeof(lang_datetime.abbrDaysOfWeek) );
    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime %d", result,sizeof(lang_datetime.abbrDaysOfWeek)); }

    result = persist_write_data(PK_LANG_DATETIME1, &lang_datetime.monthsNames, sizeof(lang_datetime.monthsNames) );
    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime1 %d", result,sizeof(lang_datetime.monthsNames)); }

    result = persist_write_data(PK_LANG_DATETIME2, &lang_datetime.DaysOfWeek, sizeof(lang_datetime.DaysOfWeek) );
    if (TRANSLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Wrote %d bytes into lang_datetime2 %d", result,sizeof(lang_datetime.DaysOfWeek)); }
	
	
	
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
  if (1) { layer_mark_dirty(calendar_layer); }
  if (1) { layer_mark_dirty(datetime_layer); }

  //update_time_text(&currentTime);
}

void my_in_rcv_handler(DictionaryIterator *received, void *context) {
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function my_in_rcv_handler"); }
// incoming message received
//  Tuple *ready_status = dict_find(received, AK_JS_LOADED);
  Tuple *message_type = dict_find(received, AK_MESSAGE_TYPE);
  // weather
  Tuple *set_weather = dict_find(received, SET_WEATHER_MSG_KEY);
//  Tuple *set_preferences = dict_find(received, SET_PREFERENCES_MSG_KEY);
  // - weather
	Tuple *set_preference = dict_find(received,SET_PREFERENCES_MSG_KEY); 
	Tuple *set_phone_bs = dict_find(received, 201);
	Tuple *set_phone_bs_chg = dict_find(received, 202);	
	Tuple *get_battery_state = dict_find(received,200);
	Tuple *status_pebble_battery_on_phone = dict_find(received,203);
	Tuple *status_pebble_cherging_on_phone = dict_find(received,204);
	Tuple *incom_sms_cnt = dict_find(received,205);
	Tuple *incom_calls_cnt = dict_find(received,206);	
	Tuple *set_phone_bs_plg = dict_find(received,207);
	Tuple *smartstatus_battery = dict_find(received,64525);
	Tuple *tis_inet_connected = dict_find(received,208);
	
	if (tis_inet_connected != NULL){
		if(tis_inet_connected->value->uint32 == 1){
			is_inet_connected = true;
		}else{
			is_inet_connected = false;
		}
		update_weather_info(weather);
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "is inet conected %d", (int)tis_inet_connected->value->uint32);
	}
	
	if (smartstatus_battery != NULL){
		phone_battery_percent = smartstatus_battery->value->uint8;
		show_battery_state();
	}
	
	
	if (set_phone_bs !=NULL){
		int phone_bs_value = set_phone_bs->value-> uint32;
		int phone_bs_chg = set_phone_bs_chg->value->uint32;
		
		phone_battery_percent=phone_bs_value;
		phone_battery_charging=false;
		if (phone_bs_chg==1){
			phone_battery_charging=true;
		}
		
		int po_phone_bs_value = status_pebble_battery_on_phone->value-> uint32;
		int po_phone_bs_chg = status_pebble_cherging_on_phone->value->uint32;
		
		pebble_on_phone_battery_percent=po_phone_bs_value;
		pebble_on_phone_battery_charging=false;
		if (po_phone_bs_chg==1){
			pebble_on_phone_battery_charging=true;
		}
		if (set_phone_bs_plg != NULL){
			if( set_phone_bs_plg->value->uint32==1){
				phone_battery_plugged = true;
			}else{
				phone_battery_plugged = false;
			}
		}
		if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Message type Phone Battery %d %d", phone_bs_value, phone_bs_chg);}
		battery_status_send(NULL);
		show_battery_state();
	}
	
	if(incom_sms_cnt != NULL){
		sms_incom_count = incom_sms_cnt->value-> uint32;
		incom_sms_show();
	}
	
	if(incom_calls_cnt != NULL){
		calls_incom_count = incom_calls_cnt->value-> uint32;
		incom_calls_show();
	}
	
	
	if(get_battery_state !=NULL){
		battery_status_send(NULL);
	}
	
	if (set_preference !=NULL){
		in_configuration_handler(received, context);
	}
  if (message_type != NULL) {
    if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Message type %d received", message_type->value->uint8); }
//	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "my_in_recv_hand message_type->value->uint8 %d",message_type->value->uint8);
    switch ( message_type->value->uint8 ) {
    case AK_TIMEZONE_OFFSET:
      in_timezone_handler(received, context);
      return;
//	case SET_PREFERENCES_MSG_KEY:
//		in_configuration_handler(received, context);
//	   return;
    }
  } else {
    // default to configuration, which may not send the message type...
//    in_configuration_handler(received, context);
//	weather_request_update();
  }
  // weather
  	if(set_weather) {
		if(set_weather->value->uint8==1){
//			text_layer_set_text(weather_temperature_layer, "РВИ");
			if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "my_in_recv_hand bf weather_set %s",weather->name);};
			weather_now_loading=false;
			weather_set(weather, received);
//			app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "my_in_recv_hand weather_set %s",weather->name);
			update_weather_info(weather);
//			app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "my_in_recv_hand upd_weather_info %s",weather->name);
		};
		if(settings.show_location==0){
			layer_set_hidden(text_layer_get_layer(weather_name_layer), true);
		}else{
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);
		}
		if(set_weather->value->uint8==2){
			text_layer_set_text(weather_name_layer,"Not Find Location");
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);
		}
		if(set_weather->value->uint8==0){
			text_layer_set_text(weather_name_layer,"No Connect");
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);
		}
		if(set_weather->value->uint8==3){
			text_layer_set_text(weather_name_layer,"GPS error");
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);
		}
		if(set_weather->value->uint8==3){
			text_layer_set_text(weather_name_layer,"I-net Error");
			layer_set_hidden(text_layer_get_layer(weather_name_layer), false);
		}		
	
	}
/*  if (ready_status){
	if (ready_status->value->int32 == 1){
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "receive status 'ready'");
		weather_request_update();
	}
  }	*/
/*	if(set_preferences) {
		Preferences old_prefs = *prefs;
		
		preferences_set(prefs, received);
		change_preferences(&old_prefs, prefs);
		
		preferences_save(prefs);
	}*/
	// -weather
}

void my_in_drp_handler(AppMessageResult reason, void *context) {
// incoming message dropped
  /*if (DEBUGLOG)*/ { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "AppMessage Dropped: %d", reason); }
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
	app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "set current page %d",current_page);
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
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function rotate_first_page"); }
	if(settings.get_position==0 && strlen(settings.type_position)==0){
		settings.first_page=0;
	}
	current_page=settings.first_page;
	set_current_page(NULL);
}


void accel_int(AccelAxisType axis, int32_t direction){
//	if (DEBUGLOG) { app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Function accel_int"); }
	if (is_accel_stoped == false){
		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "start_timer is_accel_stop=false AccelAxisType = %d direction =%d",axis,(int)direction);
		if(current_page==0){ current_page=1;}else{current_page=0;}
		if(settings.get_position==0 && strlen(settings.type_position)==0){
			current_page=0;
		}
		set_current_page(NULL);
		if (current_page!=settings.first_page && settings.auto_back!=0){
			auto_back_fp = app_timer_register(settings.auto_back*1000, &rotate_page_by_timer, NULL);
			app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "start_timer %d",(int)(settings.auto_back*1000));
		}
		if (current_page == settings.first_page){
			auto_back_fp = NULL;
		}
	} else {
//		app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "start_timer is_accel_stop=true");
		;
	}
}

//static void back_button_handler(ClickRecognizerRef recognizer, void *context) {
//	if( settings.first_page==0){ settings.first_page=1;} else {settings.first_page=0;}
//	rotate_first_page();
//}

//static void click_config_provider(void *context) {
//  window_single_click_subscribe(BUTTON_ID_BACK, back_button_handler);
//  app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "click config provider");
//}

static void init(void) {

  app_message_init();
//	if (DEBUGLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "init");};
  if (persist_exists(PK_SETTINGS)) {
	if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read settings");};
    persist_read_data(PK_SETTINGS, &settings, sizeof(settings) );
  }
  if (persist_exists(PK_LANG_GEN)) {
	if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read lang");};
    persist_read_data(PK_LANG_GEN, &lang_gen, sizeof(lang_gen) );
  }
  if (persist_exists(PK_LANG_DATETIME)) {
	if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.abbrDaysOfWeek));};
    persist_read_data(PK_LANG_DATETIME, &lang_datetime.abbrDaysOfWeek, sizeof(lang_datetime.abbrDaysOfWeek) );
  }
  
  if (persist_exists(PK_LANG_DATETIME1)) {
	if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.monthsNames));};
    persist_read_data(PK_LANG_DATETIME1, &lang_datetime.monthsNames, sizeof(lang_datetime.monthsNames) );
  }
  
  if (persist_exists(PK_LANG_DATETIME2)) {
	if (TRANSLOG) {app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "read datta %d",sizeof(lang_datetime.DaysOfWeek));};
    persist_read_data(PK_LANG_DATETIME2, &lang_datetime.DaysOfWeek, sizeof(lang_datetime.DaysOfWeek) );
  }
  
  request_timezone();

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
	accel_tap_service_subscribe(accel_int);
  }
  current_page=settings.first_page;
  set_current_page(NULL);
  
  vibe_suppression = false;
  
//	if(weather_in_cache()){
	weather = weather_load_cache();  
	update_weather_info(weather);
	
	set_default_font();
	get_phone_battery_state = true;
	set_pos_data_over_time();
//	sendBatteryState();
//	weather->last_update_time = 0;
//	}
//  weather_request_update();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

//{"message":"accurate","cod":"200","count":1,"list":[{"id":692194,"name":"Sumy","coord":{"lon":34.800289,"lat":50.9216},"main":{"temp":256.562,"temp_min":256.562,"temp_max":256.562,"pressure":1024.13,"sea_level":1046.02,"grnd_level":1024.13,"humidity":78},"dt":1390980780,"wind":{"speed":9.61,"deg":91.0002},"sys":{"country":"UA"},"clouds":{"all":24},"weather":[{"id":801,"main":"Clouds","description":"few clouds","icon":"02d"}]}]}