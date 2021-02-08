#include <tizen.h>
#include "conwaylife.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Eina_Array *uinv;
	int w;
	int h;
} appdata_s;

#define TEXT_BUF_SIZE 256

static int is_alive(Evas_Object *o) {
	int alpha;
	int r;
	int g;
	int b;

	evas_object_color_get(o, &r, &g, &b, &alpha);

	return r == 85;
}

static void evolve(Eina_Array *uinv, int w, int h)
{
	unsigned new[h][w];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int n = 0;
			for (int y1 = y - 1; y1 <= y + 1; y1++) {
				for (int x1 = x - 1; x1 <= x + 1; x1++) {
					int idx = (y1 + h) % h;
					Eina_Array *d = eina_array_data_get(uinv, (x1 + w) % w);
					Evas_Object *o = eina_array_data_get(d, idx);
					if (o && is_alive(o))
						n++;
				}
			}

			Eina_Array *d = eina_array_data_get(uinv, x);
			Evas_Object *o = eina_array_data_get(d, y);
			if (is_alive(o)) n--;
			new[x][y] = (n == 3 || (n == 2 && is_alive(o)));
		}
	}

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			Eina_Array *d = eina_array_data_get(uinv, x);
			Evas_Object *o = eina_array_data_get(d, y);
			if (new[x][y]) {
				evas_object_color_set(o, 85, 108, 252, 255);
			} else {
				evas_object_color_set(o, 0, 0, 0, 0);
			}
		}
	}
}

static const char* get_month(int month) {
	switch (month) {
		case 1:  return "Jan";
		case 2:  return "Feb";
		case 3:  return "Mar";
		case 4:  return "Apr";
		case 5:  return "May";
		case 6:  return "Jun";
		case 7:  return "Jul";
		case 8:  return "Aug";
		case 9:  return "Sep";
		case 10:  return "Oct";
		case 11: return "Nov";
		case 12: return "Dec";
	}

	return "";
}

static const char* get_day (int day) {
	switch (day) {
		case 1: return "Mon";
		case 2: return "Tue";
		case 3: return "Wed";
		case 4: return "Thu";
		case 5: return "Fri";
		case 6: return "Sat";
		case 7: return "Sun";
	}

	return "";
}

static void
update_watch(appdata_s *ad, watch_time_h watch_time, int ambient)
{
	char watch_text[TEXT_BUF_SIZE];
	int hour24, minute, second, month, dow, day;

	if (watch_time == NULL)
		return;

	watch_time_get_hour24(watch_time, &hour24);
	watch_time_get_minute(watch_time, &minute);
	watch_time_get_second(watch_time, &second);
	watch_time_get_month(watch_time, &month);
	watch_time_get_day_of_week(watch_time, &dow);
	watch_time_get_day(watch_time, &day);
	if (!ambient) {
		snprintf(watch_text, TEXT_BUF_SIZE,
				"<align=middle>"
				"<font=Tizen,Vera,Kochi>"
				"<font_size=80>%02d<color=#fc0a2e>:</color>%02d<color=#fc0a2e>:</color>%02d</font_size>"
				"<br/>"
				"<font_size=20>%s %02d %s</font_size>"
				"</font>"
				"</align>",
			hour24, minute, second, get_day(dow), day, get_month(month));
	} else {
		snprintf(watch_text, TEXT_BUF_SIZE,
						"<align=middle>"
						"<font=Tizen,Vera,Kochi>"
						"<font_size=80>%02d<color=#fc0a2e>:</color>%02d<color=#fc0a2e>:</color>%02d</font_size>"
						"<br/>"
						"<font_size=20>%s %02d %s</font_size>"
						"</font>"
						"</align>",
					hour24, minute, second, get_day(dow), day, get_month(month));
	}

	evolve(ad->uinv, ad->w, ad->h);

	elm_object_text_set(ad->label, watch_text);
}

static void
create_base_gui(appdata_s *ad, int width, int height)
{
	int ret;
	watch_time_h watch_time = NULL;

	/* Window */
	ret = watch_app_get_elm_win(&ad->win);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return;
	}

	evas_object_resize(ad->win, width, height);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	Evas *evas = evas_object_evas_get(ad->win);
	ad->uinv = eina_array_new(width / 5);

	ad->w = width / 5;
	ad->h = height / 5;

	for (int x = 0; x < width / 5; x++) {
		Eina_Array *d = eina_array_new(height / 5);

		for (int y = 0; y < height / 5; y++) {
			Evas_Object *o = evas_object_rectangle_add(evas);
			if (rand() < RAND_MAX / 10) {
				evas_object_color_set(o, 85, 108, 252, 255);
			} else {
				evas_object_color_set(o, 0, 0, 0, 0);
			}

			evas_object_resize(o, 3, 3);
			evas_object_move(o, x * 5, y * 5);
			evas_object_show(o);

			eina_array_push(d, o);
		}
		eina_array_push(ad->uinv, d);
	}

	/* Label*/
	ad->label = elm_label_add(ad->conform);
	evas_object_resize(ad->label, width, height);
	evas_object_move(ad->label, 0, height / 3);
	evas_object_show(ad->label);

	ret = watch_time_get_current_time(&watch_time);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get current time. err = %d", ret);

	update_watch(ad, watch_time, 0);
	watch_time_delete(watch_time);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(int width, int height, void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad, width, height);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
app_time_tick(watch_time_h watch_time, void *data)
{
	/* Called at each second while your app is visible. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 0);
}

static void
app_ambient_tick(watch_time_h watch_time, void *data)
{
	/* Called at each minute while the device is in ambient mode. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 1);
}

static void
app_ambient_changed(bool ambient_mode, void *data)
{
	/* Update your watch UI to conform to the ambient mode */
}

static void
watch_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
watch_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	watch_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.time_tick = app_time_tick;
	event_callback.ambient_tick = app_ambient_tick;
	event_callback.ambient_changed = app_ambient_changed;

	watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, watch_app_lang_changed, &ad);
	watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, watch_app_region_changed, &ad);

	ret = watch_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
	}

	return ret;
}
