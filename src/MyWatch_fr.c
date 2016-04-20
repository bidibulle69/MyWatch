#include <pebble.h>

Window *window;

//-- MyWatch Declaration
TextLayer *text_time_layer;
TextLayer *text_wday_layer;
TextLayer *text_day_month_layer;
TextLayer *text_weeknumber_layer;
//TextLayer *text_dualtime_layer;

BitmapLayer *img_bluetooth_layer; 	
BitmapLayer *img_battery_layer; 	
BitmapLayer *img_moon_layer; 	

GBitmap *bitmap_moon;
GBitmap *bitmap_battery;
GBitmap *bitmap_bluetooth;


const int BATT_IMG[]={
	RESOURCE_ID_IMG_BATT0,
	RESOURCE_ID_IMG_BATT10,
	RESOURCE_ID_IMG_BATT20,
	RESOURCE_ID_IMG_BATT30,
	RESOURCE_ID_IMG_BATT40,
	RESOURCE_ID_IMG_BATT50,
	RESOURCE_ID_IMG_BATT60,
	RESOURCE_ID_IMG_BATT70,
	RESOURCE_ID_IMG_BATT80,
	RESOURCE_ID_IMG_BATT90,
	RESOURCE_ID_IMG_BATT100
};

const int MOON_IMG[]={
	RESOURCE_ID_IMG_MOON01, //New moon
	RESOURCE_ID_IMG_MOON02,
	RESOURCE_ID_IMG_MOON02,
	RESOURCE_ID_IMG_MOON03,
	RESOURCE_ID_IMG_MOON03,
	RESOURCE_ID_IMG_MOON04, //1st quater
	RESOURCE_ID_IMG_MOON04,
	RESOURCE_ID_IMG_MOON05,
	RESOURCE_ID_IMG_MOON05,
	RESOURCE_ID_IMG_MOON06,
	RESOURCE_ID_IMG_MOON06,
	RESOURCE_ID_IMG_MOON06,
	RESOURCE_ID_IMG_MOON07, //13: Full moon
	RESOURCE_ID_IMG_MOON08,
	RESOURCE_ID_IMG_MOON08,
	RESOURCE_ID_IMG_MOON08,
	RESOURCE_ID_IMG_MOON09,
	RESOURCE_ID_IMG_MOON09,
	RESOURCE_ID_IMG_MOON09,
	RESOURCE_ID_IMG_MOON10,
	RESOURCE_ID_IMG_MOON10, //21: Last quater
	RESOURCE_ID_IMG_MOON10,
	RESOURCE_ID_IMG_MOON11,
	RESOURCE_ID_IMG_MOON11,
	RESOURCE_ID_IMG_MOON11,
	RESOURCE_ID_IMG_MOON12,
	RESOURCE_ID_IMG_MOON12
};


const char* const WEEKDAYS[] = {
    "Dimanche",
    "Lundi",
    "Mardi",
    "Mercredi",
    "Jeudi",
    "Vendredi",
    "Samedi"
};

const char* const MONTHS[] = {
    "Janvier",
    "Février",
    "Mars",
    "Avril",
    "Mai",
    "Juin",
    "Juillet",
    "Août",
    "Septembre",
    "Octobre",
    "Novembre",
    "Décembre"
};

const char WEEK[] = "Semaine %d ";
const int DAYMONTH_FORMAT = 0; //0=Day and month 1=Month and day


static int moon_phase(int jdn)
{
    double jd;
    jd = jdn-2451550.1;
    jd /= 29.530588853;
    jd -= (int)jd;
    return (int)(jd*27 + 0.5); /* scale fraction from 0-27 and round by adding 0.5 */
}   


//-- Return the julian day
static int julianday(int day, int month, int year) {
    if (month < 3) {
        month = month + 12;
        year = year - 1;
    }

    return day + (int)((153 * month - 457) /  5) + 365 * year + (int)((year / 4)) - (int)(year / 100) + (int)(year / 400) + 1721119;
}


//-- Return the week number
static int weeknumber(int day, int month, int year) {
	int J = julianday(day, month, year);

	int d4 = (J+31741-(J % 7)) % 146097 % 36524 % 1461;
	int L = (int)(d4 / 1460);
	int d1 = ((d4-L) % 365)+L;

	return (int)(d1 / 7)+1;
}



//-- Called each minute
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";

  char *time_format;
  static char wday_str[] = " Xxxxxxxx";
  static char daymonth_str[] = "00 Xxxxxxxxxx";
  static char weeknumber_str[] = "Semaine 00";
  //static char battery_str[] = "000%";	

  //-- Refresh week day
  snprintf(wday_str,sizeof(wday_str),"%s",WEEKDAYS[tick_time->tm_wday]);
  text_layer_set_text(text_wday_layer,wday_str);

	//-- Refresh day and month
	if (DAYMONTH_FORMAT==0)
		snprintf(daymonth_str,sizeof(daymonth_str),"%d  %s",(int)tick_time->tm_mday,MONTHS[tick_time->tm_mon]);
	else
		snprintf(daymonth_str,sizeof(daymonth_str),"%s  %d",MONTHS[tick_time->tm_mon],(int)tick_time->tm_mday);
  	text_layer_set_text(text_day_month_layer, daymonth_str);

	//-- Refresh week number
  snprintf(weeknumber_str,sizeof(weeknumber_str),WEEK,weeknumber(tick_time->tm_mday, tick_time->tm_mon+1, tick_time->tm_year+1900));
  text_layer_set_text(text_weeknumber_layer, weeknumber_str);
  
	//-- Refresh battery level
  BatteryChargeState charge_state = battery_state_service_peek();
	gbitmap_destroy(bitmap_battery);
	bitmap_battery=gbitmap_create_with_resource(BATT_IMG[charge_state.charge_percent/10]);
	bitmap_layer_set_bitmap(img_battery_layer,bitmap_battery);
	
	//-- Refresh Time
	if (clock_is_24h_style()) {
		time_format = "%R";
	} else {
		time_format = "%I:%M";
	}
	
	strftime(time_text,sizeof(time_text), time_format, tick_time);

	// Kludge to handle lack of non-padded hour format string
	// for twelve hour clock.
	if (!clock_is_24h_style() && (time_text[0] == '0')) {
		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}

  	text_layer_set_text(text_time_layer, time_text);
	
	//-- Bluetooth connection indicator
	/*
	gbitmap_destroy(bitmap_bluetooth);
	
	if (bluetooth_connection_service_peek())
		bitmap_bluetooth=gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUETOOTH);
	else
		bitmap_bluetooth=gbitmap_create_with_resource(RESOURCE_ID_IMG_EMPTY);
	bitmap_layer_set_bitmap(img_bluetooth_layer,bitmap_bluetooth);
	*/
	layer_set_hidden(bitmap_layer_get_layer(img_bluetooth_layer),(!bluetooth_connection_service_peek()));
	
	//-- Moon indicator
	gbitmap_destroy(bitmap_moon);
	
	int mph=moon_phase( julianday(tick_time->tm_mday, tick_time->tm_mon+1, tick_time->tm_year+1900) );
	
	if (mph==0) mph=1; //In some case there is this problem
	
	bitmap_moon=gbitmap_create_with_resource(MOON_IMG[mph-1]);
	bitmap_layer_set_bitmap(img_moon_layer,bitmap_moon);

  //-- Vibrate function - 1 vibrate each 30min / 2 vibrates each hour (from 9h to 21h)
  int h;
  h= (tick_time->tm_hour)*100 + tick_time->tm_min;
  //if ((htick_time->tm_hour>=9) && (tick_time->tm_hour<=21)) {
  if ((h>=900) && (h<=2100)) {
    if ( (tick_time->tm_min==30) && (tick_time->tm_sec==0) )
      vibes_short_pulse();
    if ( (tick_time->tm_min==00) && (tick_time->tm_sec==0) )
      vibes_double_pulse();
  }
}



void handle_init(void) {
	
	window=window_create();
	
  	window_stack_push(window, true /* Animated */);
  	window_set_background_color(window, GColorBlack);
	
	Layer *window_layer = window_get_root_layer(window);

	//-- Dual time
    /*
	text_dualtime_layer=text_layer_create(GRect(0,-5,40, 18));
	text_layer_set_text_color(text_dualtime_layer, GColorWhite);
	text_layer_set_background_color(text_dualtime_layer, GColorClear);
	text_layer_set_text_alignment(text_dualtime_layer,GTextAlignmentLeft);
	text_layer_set_font(text_dualtime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text(text_dualtime_layer, "00:00");
	layer_add_child(window_layer, text_layer_get_layer(text_dualtime_layer));
	*/
    
	//-- Battery indicator
	img_battery_layer=bitmap_layer_create (GRect(144-25, 2, 25, 11)); 	
	bitmap_battery=gbitmap_create_with_resource(RESOURCE_ID_IMG_BATT100);
	bitmap_layer_set_bitmap(img_battery_layer,bitmap_battery);
    layer_add_child(window_layer, bitmap_layer_get_layer(img_battery_layer));

	//-- Bluetooth indicator
	img_bluetooth_layer=bitmap_layer_create (GRect(144-25-8-4, 0, 8, 15));
	bitmap_bluetooth=gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUETOOTH);
	bitmap_layer_set_bitmap(img_bluetooth_layer,bitmap_bluetooth);
	layer_set_hidden(bitmap_layer_get_layer(img_bluetooth_layer),true);
    layer_add_child(window_layer, bitmap_layer_get_layer(img_bluetooth_layer));
	
	//-- Moon indicator
	//img_moon_layer=bitmap_layer_create (GRect(10, 0, 14, 14));
	img_moon_layer=bitmap_layer_create (GRect(2, 150, 14, 14));
	bitmap_moon=gbitmap_create_with_resource(MOON_IMG[0]);
	bitmap_layer_set_bitmap(img_moon_layer,bitmap_moon);
    layer_add_child(window_layer, bitmap_layer_get_layer(img_moon_layer));
	
	//-- Week day
	text_wday_layer=text_layer_create(GRect(4, 28, 144, 28));
	text_layer_set_text_color(text_wday_layer, GColorWhite);
	text_layer_set_background_color(text_wday_layer, GColorClear);
	text_layer_set_text_alignment(text_wday_layer,GTextAlignmentLeft);
	text_layer_set_font(text_wday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	layer_add_child(window_layer, text_layer_get_layer(text_wday_layer));
						
						
	//-- Time
	text_time_layer=text_layer_create(GRect(0, 48, 144, 56));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_background_color(text_time_layer, GColorClear);
	text_layer_set_text_alignment(text_time_layer,GTextAlignmentCenter);
	text_layer_set_font(text_time_layer,fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));						

						
	//-- Day and month
	text_day_month_layer=text_layer_create(GRect(0, 93, 144, 32));
	text_layer_set_text_color(text_day_month_layer, GColorWhite);
	text_layer_set_background_color(text_day_month_layer, GColorClear);
	text_layer_set_text_alignment(text_day_month_layer,GTextAlignmentRight);
	text_layer_set_font(text_day_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	layer_add_child(window_layer, text_layer_get_layer(text_day_month_layer));						

						
	//-- Week number
	text_weeknumber_layer=text_layer_create(GRect(16, 138, 126, 28));
	text_layer_set_text_color(text_weeknumber_layer, GColorWhite);
	text_layer_set_background_color(text_weeknumber_layer, GColorBlack);
	text_layer_set_text_alignment(text_weeknumber_layer,GTextAlignmentRight);
	text_layer_set_font(text_weeknumber_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	layer_add_child(window_layer, text_layer_get_layer(text_weeknumber_layer));

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


//-- Called before living the application - Need to release memory
void handle_deinit(void) {
	tick_timer_service_unsubscribe();
 	
	bitmap_layer_destroy(img_bluetooth_layer);
	bitmap_layer_destroy(img_battery_layer);
	bitmap_layer_destroy(img_moon_layer);

	gbitmap_destroy(bitmap_battery);
	gbitmap_destroy(bitmap_bluetooth);
	gbitmap_destroy(bitmap_moon);
	
	text_layer_destroy(text_time_layer);
	//text_layer_destroy(text_dualtime_layer);
	text_layer_destroy(text_wday_layer);
	text_layer_destroy(text_day_month_layer);
	text_layer_destroy(text_weeknumber_layer);

  	window_destroy(window);
}


//-- Everything start here !!!
int main(void) {
	handle_init();
	
	app_event_loop();
  	handle_deinit();
}
