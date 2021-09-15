#include "setting-docks.hpp"
#include <obs-module.h>
#include <QMainWindow>

#include "version.h"
#include "stream-dock.hpp"
#include "video-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Exeldro");
OBS_MODULE_USE_DEFAULT_LOCALE("setting-docks", "en-US")

bool obs_module_load()
{
	blog(LOG_INFO, "[Setting Docks] loaded version %s", PROJECT_VERSION);

	const auto main_window =
		static_cast<QMainWindow *>(obs_frontend_get_main_window());
	obs_frontend_push_ui_translation(obs_module_get_string);
	obs_frontend_add_dock(new StreamDock(main_window));
	obs_frontend_add_dock(new VideoDock(main_window));
	obs_frontend_pop_ui_translation();

	return true;
}

void obs_module_unload() {}

MODULE_EXPORT const char *obs_module_description(void)
{
	return obs_module_text("Description");
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return obs_module_text("SettingDocks");
}
