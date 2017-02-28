#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "config.h"
#include "interface/keyboard_shortcut.h"
#include "interface/themes.h"
#include "interface/viewport.h"
#include "localisation/language.h"
#include "localisation/localisation.h"
#include "network/network.h"
#include "OpenRCT2.h"
#include "util/util.h"

enum {
	CONFIG_VALUE_TYPE_BOOLEAN,
	CONFIG_VALUE_TYPE_UINT8,
	CONFIG_VALUE_TYPE_UINT16,
	CONFIG_VALUE_TYPE_UINT32,
	CONFIG_VALUE_TYPE_SINT8,
	CONFIG_VALUE_TYPE_SINT16,
	CONFIG_VALUE_TYPE_SINT32,
	CONFIG_VALUE_TYPE_FLOAT,
	CONFIG_VALUE_TYPE_DOUBLE,
	CONFIG_VALUE_TYPE_STRING
};

size_t _configValueTypeSize[] = {
	sizeof(bool),
	sizeof(uint8),
	sizeof(uint16),
	sizeof(uint32),
	sizeof(sint8),
	sizeof(sint16),
	sizeof(sint32),
	sizeof(float),
	sizeof(double),
	sizeof(utf8string)
};

typedef union {
	sint32 value_sint32;

	bool value_boolean;
	sint8 value_sint8;
	sint16 value_sint16;
	uint8 value_uint8;
	uint16 value_uint16;
	uint32 value_uint32;
	float value_float;
	double value_double;
	utf8string value_string;
} value_union;

typedef struct config_enum_definition {
	const_utf8string key;
	value_union value;
} config_enum_definition;

#define END_OF_ENUM { NULL, 0 }

typedef struct config_property_definition {
	size_t offset;
	const_utf8string property_name;
	uint8 type;
	value_union default_value;
	config_enum_definition *enum_definitions;
} config_property_definition;

typedef struct config_section_definition {
	void *base_address;
	const_utf8string section_name;
	config_property_definition *property_definitions;
	sint32 property_definitions_count;
} config_section_definition;

#pragma region Enum definitions

config_enum_definition _drawingEngineFormatEnum[] = {
	{ "SOFTWARE", DRAWING_ENGINE_SOFTWARE },
	{ "SOFTWARE_HWD", DRAWING_ENGINE_SOFTWARE_WITH_HARDWARE_DISPLAY },
	{ "OPENGL", DRAWING_ENGINE_OPENGL },
	END_OF_ENUM
};

config_enum_definition _measurementFormatEnum[] = {
	{ "IMPERIAL", MEASUREMENT_FORMAT_IMPERIAL },
	{ "METRIC", MEASUREMENT_FORMAT_METRIC },
	{ "SI", MEASUREMENT_FORMAT_SI },
	END_OF_ENUM
};

config_enum_definition _temperatureFormatEnum[] = {
	{ "CELSIUS", TEMPERATURE_FORMAT_C },
	{ "FAHRENHEIT", TEMPERATURE_FORMAT_F },
	END_OF_ENUM
};

config_enum_definition _currencyEnum[] = {
	{ "GBP", CURRENCY_POUNDS },
	{ "USD", CURRENCY_DOLLARS },
	{ "FRF", CURRENCY_FRANC },
	{ "DEM", CURRENCY_DEUTSCHMARK },
	{ "JPY", CURRENCY_YEN },
	{ "ESP", CURRENCY_PESETA },
	{ "ITL", CURRENCY_LIRA },
	{ "NLG", CURRENCY_GUILDERS },
	{ "SEK", CURRENCY_KRONA },
	{ "EUR", CURRENCY_EUROS },
	{ "KRW", CURRENCY_WON },
	{ "RUB", CURRENCY_ROUBLE },
	{ "CZK", CURRENCY_CZECH_KORUNA },
	{ "HKD", CURRENCY_HKD },
	{ "TWD", CURRENCY_TWD },
	{ "CNY", CURRENCY_YUAN },
	END_OF_ENUM
};

config_enum_definition _currencySymbolAffixEnum[] = {
	{ "PREFIX", CURRENCY_PREFIX },
	{ "SUFFIX", CURRENCY_SUFFIX },
	END_OF_ENUM
};

config_enum_definition _languageEnum[] = {
	{ "en-GB", 	LANGUAGE_ENGLISH_UK },
	{ "en-US", 	LANGUAGE_ENGLISH_US },
	{ "de-DE", 	LANGUAGE_GERMAN },
	{ "nl-NL", 	LANGUAGE_DUTCH },
	{ "fr-FR", 	LANGUAGE_FRENCH },
	{ "hu-HU", 	LANGUAGE_HUNGARIAN },
	{ "pl-PL", 	LANGUAGE_POLISH },
	{ "es-ES", 	LANGUAGE_SPANISH },
	{ "sv-SE", 	LANGUAGE_SWEDISH },
	{ "it-IT", 	LANGUAGE_ITALIAN },
	{ "pt-BR", 	LANGUAGE_PORTUGUESE_BR },
	{ "zh-TW",	LANGUAGE_CHINESE_TRADITIONAL },
	{ "zh-CN",	LANGUAGE_CHINESE_SIMPLIFIED },
	{ "fi-FI", 	LANGUAGE_FINNISH },
	{ "ko-KR", 	LANGUAGE_KOREAN },
	{ "ru-RU", 	LANGUAGE_RUSSIAN },
	{ "cs-CZ", 	LANGUAGE_CZECH },
	{ "ja-JP", 	LANGUAGE_JAPANESE },
	{ "nb-NO",	LANGUAGE_NORWEGIAN },
	END_OF_ENUM
};

config_enum_definition _dateFormatEnum[] = {
	{ "DD/MM/YY", DATE_FORMAT_DMY },
	{ "MM/DD/YY", DATE_FORMAT_MDY },
	{ "YY/MM/DD", DATE_FORMAT_YMD },
	{ "YY/DD/MM", DATE_FORMAT_YDM },
	END_OF_ENUM
};

#pragma endregion

#pragma region Section / property definitions

config_property_definition _generalDefinitions[] = {
	{ offsetof(general_configuration, always_show_gridlines),			"always_show_gridlines",		CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, autosave_frequency),				"autosave",						CONFIG_VALUE_TYPE_UINT8,		AUTOSAVE_EVERY_5MINUTES,		NULL					},
	{ offsetof(general_configuration, confirmation_prompt),				"confirmation_prompt",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, construction_marker_colour),		"construction_marker_colour",	CONFIG_VALUE_TYPE_UINT8,		false,							NULL					},
	{ offsetof(general_configuration, currency_format),					"currency_format",				CONFIG_VALUE_TYPE_UINT8,		CURRENCY_POUNDS,				_currencyEnum			},
	{ offsetof(general_configuration, custom_currency_rate),			"custom_currency_rate",			CONFIG_VALUE_TYPE_SINT32,		10,								NULL					},
	{ offsetof(general_configuration, custom_currency_affix),			"custom_currency_affix",		CONFIG_VALUE_TYPE_SINT8,		CURRENCY_SUFFIX,				_currencySymbolAffixEnum},
	{ offsetof(general_configuration, custom_currency_symbol),			"custom_currency_symbol",		CONFIG_VALUE_TYPE_STRING,		{ .value_string = "Ctm" },		NULL					},
	{ offsetof(general_configuration, edge_scrolling),					"edge_scrolling",				CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, fullscreen_mode),					"fullscreen_mode",				CONFIG_VALUE_TYPE_UINT8,		0,								NULL					},
	{ offsetof(general_configuration, fullscreen_height),				"fullscreen_height",			CONFIG_VALUE_TYPE_SINT32,		-1,								NULL					},
	{ offsetof(general_configuration, fullscreen_width),				"fullscreen_width",				CONFIG_VALUE_TYPE_SINT32,		-1,								NULL					},
	{ offsetof(general_configuration, rct1_path),						"rct1_path",					CONFIG_VALUE_TYPE_STRING,		{ .value_string = NULL },		NULL					},
	{ offsetof(general_configuration, rct2_path),						"game_path",					CONFIG_VALUE_TYPE_STRING,		{ .value_string = NULL },		NULL					},
	{ offsetof(general_configuration, landscape_smoothing),				"landscape_smoothing",			CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, language),						"language",						CONFIG_VALUE_TYPE_UINT16,		LANGUAGE_ENGLISH_UK,			_languageEnum			},
	{ offsetof(general_configuration, measurement_format),				"measurement_format",			CONFIG_VALUE_TYPE_UINT8,		MEASUREMENT_FORMAT_METRIC,	_measurementFormatEnum	},
	{ offsetof(general_configuration, play_intro),						"play_intro",					CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, save_plugin_data),				"save_plugin_data",				CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, debugging_tools),					"debugging_tools",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, show_height_as_units),			"show_height_as_units",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, temperature_format),				"temperature_format",			CONFIG_VALUE_TYPE_UINT8,		TEMPERATURE_FORMAT_C,			_temperatureFormatEnum	},
	{ offsetof(general_configuration, window_height),					"window_height",				CONFIG_VALUE_TYPE_SINT32,		-1,								NULL					},
	{ offsetof(general_configuration, window_snap_proximity),			"window_snap_proximity",		CONFIG_VALUE_TYPE_UINT8,		5,								NULL					},
	{ offsetof(general_configuration, window_width),					"window_width",					CONFIG_VALUE_TYPE_SINT32,		-1,								NULL					},
	{ offsetof(general_configuration, drawing_engine),					"drawing_engine",				CONFIG_VALUE_TYPE_UINT8,		DRAWING_ENGINE_SOFTWARE,		_drawingEngineFormatEnum},
	{ offsetof(general_configuration, uncap_fps),						"uncap_fps",					CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, test_unfinished_tracks),			"test_unfinished_tracks",		CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					}, //Default config setting is false until ghost trains are implemented #4540
	{ offsetof(general_configuration, no_test_crashes),					"no_test_crashes",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, date_format),						"date_format",					CONFIG_VALUE_TYPE_UINT8,		DATE_FORMAT_DMY,				_dateFormatEnum			},
	{ offsetof(general_configuration, auto_staff_placement),			"auto_staff",					CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, handymen_mow_default),			"handymen_mow_default",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, default_inspection_interval),		"default_inspection_interval",	CONFIG_VALUE_TYPE_UINT8,		2,								NULL					},
	{ offsetof(general_configuration, last_run_version),				"last_run_version",				CONFIG_VALUE_TYPE_STRING,		{ .value_string = NULL },		NULL					},
	{ offsetof(general_configuration, invert_viewport_drag),			"invert_viewport_drag",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, load_save_sort),					"load_save_sort",				CONFIG_VALUE_TYPE_UINT8,		SORT_NAME_ASCENDING,			NULL					},
	{ offsetof(general_configuration, minimize_fullscreen_focus_loss),	"minimize_fullscreen_focus_loss",CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, day_night_cycle),					"day_night_cycle",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					}, //Default config setting is false until the games canvas can be seperated from the effect
	{ offsetof(general_configuration, enable_light_fx),					"enable_light_fx",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, upper_case_banners),				"upper_case_banners",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, disable_lightning_effect),		"disable_lightning_effect",		CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, allow_loading_with_incorrect_checksum),"allow_loading_with_incorrect_checksum",	CONFIG_VALUE_TYPE_BOOLEAN,		true,			NULL					},
	{ offsetof(general_configuration, steam_overlay_pause),				"steam_overlay_pause",			CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, window_scale),					"window_scale",					CONFIG_VALUE_TYPE_FLOAT,		{ .value_float = 1.0f },		NULL					},
	{ offsetof(general_configuration, scale_quality),					"scale_quality",				CONFIG_VALUE_TYPE_UINT8,		1,								NULL					},
	{ offsetof(general_configuration, use_nn_at_integer_scales),		"use_nn_at_integer_scales",		CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, show_fps),						"show_fps",						CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, trap_cursor),						"trap_cursor",					CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, auto_open_shops),					"auto_open_shops",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(general_configuration, scenario_select_mode),			"scenario_select_mode",			CONFIG_VALUE_TYPE_UINT8,		SCENARIO_SELECT_MODE_ORIGIN,	NULL					},
	{ offsetof(general_configuration, scenario_unlocking_enabled),		"scenario_unlocking_enabled",	CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, scenario_hide_mega_park),			"scenario_hide_mega_park",		CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, last_save_game_directory),        "last_game_directory",          CONFIG_VALUE_TYPE_STRING,       { .value_string = NULL },       NULL                    },
	{ offsetof(general_configuration, last_save_landscape_directory),   "last_landscape_directory",     CONFIG_VALUE_TYPE_STRING,       { .value_string = NULL },       NULL                    },
	{ offsetof(general_configuration, last_save_scenario_directory),    "last_scenario_directory",      CONFIG_VALUE_TYPE_STRING,       { .value_string = NULL },       NULL                    },
	{ offsetof(general_configuration, last_save_track_directory),       "last_track_directory",         CONFIG_VALUE_TYPE_STRING,       { .value_string = NULL },       NULL                    },
	{ offsetof(general_configuration, window_limit),					"window_limit",					CONFIG_VALUE_TYPE_UINT8,		WINDOW_LIMIT_MAX,				NULL					},
	{ offsetof(general_configuration, zoom_to_cursor),					"zoom_to_cursor",				CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, render_weather_effects),			"render_weather_effects",		CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(general_configuration, render_weather_gloom),			"render_weather_gloom",			CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
};

config_property_definition _interfaceDefinitions[] = {
	{ offsetof(interface_configuration, toolbar_show_finances),			"toolbar_show_finances",		CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(interface_configuration, toolbar_show_research),			"toolbar_show_research",		CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(interface_configuration, toolbar_show_cheats),			"toolbar_show_cheats",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(interface_configuration, toolbar_show_news),				"toolbar_show_news",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(interface_configuration, select_by_track_type),			"select_by_track_type",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(interface_configuration, console_small_font),			"console_small_font",			CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(interface_configuration, current_theme_preset),			"current_theme",				CONFIG_VALUE_TYPE_STRING,		{ .value_string = "*RCT2" },	NULL					},
	{ offsetof(interface_configuration, current_title_sequence_preset),	"current_title_sequence",		CONFIG_VALUE_TYPE_STRING,		{ .value_string = "*OPENRCT2" },NULL					},
	{ offsetof(interface_configuration, object_selection_filter_flags),	"object_selection_filter_flags",CONFIG_VALUE_TYPE_UINT32,		0x7EF,							NULL					},
};

config_property_definition _soundDefinitions[] = {
	{ offsetof(sound_configuration, master_volume),						"master_volume",				CONFIG_VALUE_TYPE_UINT8,		100,							NULL					},
	{ offsetof(sound_configuration, title_music),						"title_music",					CONFIG_VALUE_TYPE_UINT8,		2,								NULL					},
	{ offsetof(sound_configuration, sound_enabled),						"sound",						CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(sound_configuration, sound_volume),						"sound_volume",					CONFIG_VALUE_TYPE_UINT8,		100,							NULL					},
	{ offsetof(sound_configuration, ride_music_enabled),				"ride_music",					CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(sound_configuration, ride_music_volume),					"ride_music_volume",			CONFIG_VALUE_TYPE_UINT8,		100,							NULL					},
	{ offsetof(sound_configuration, audio_focus),						"audio_focus",					CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(sound_configuration, device),							"audio_device",					CONFIG_VALUE_TYPE_STRING,		{ .value_string = NULL },		NULL					},
};

config_property_definition _twitchDefinitions[] = {
	{ offsetof(twitch_configuration, channel),							"channel",						CONFIG_VALUE_TYPE_STRING,		{ .value_string = NULL },		NULL					},
	{ offsetof(twitch_configuration, enable_follower_peep_names),		"follower_peep_names",			CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(twitch_configuration, enable_follower_peep_tracking),	"follower_peep_tracking",		CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(twitch_configuration, enable_chat_peep_names),			"chat_peep_names",				CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(twitch_configuration, enable_chat_peep_tracking),		"chat_peep_tracking",			CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(twitch_configuration, enable_news),						"news",							CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					}
};

config_property_definition _networkDefinitions[] = {
	{ offsetof(network_configuration, player_name),						"player_name",					CONFIG_VALUE_TYPE_STRING,		{.value_string = "Player" },	NULL					},
	{ offsetof(network_configuration, default_port),					"default_port",					CONFIG_VALUE_TYPE_UINT32,		NETWORK_DEFAULT_PORT,			NULL					},
	{ offsetof(network_configuration, default_password),				"default_password",				CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL		},	NULL					},
	{ offsetof(network_configuration, stay_connected),					"stay_connected",				CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(network_configuration, advertise),						"advertise",					CONFIG_VALUE_TYPE_BOOLEAN,		true,							NULL					},
	{ offsetof(network_configuration, maxplayers),						"maxplayers",					CONFIG_VALUE_TYPE_UINT8,		16,								NULL					},
	{ offsetof(network_configuration, server_name),						"server_name",					CONFIG_VALUE_TYPE_STRING,		{.value_string = "Server" },	NULL					},
	{ offsetof(network_configuration, server_description),				"server_description",			CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, server_greeting),					"server_greeting",				CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, master_server_url),				"master_server_url",			CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, provider_name),					"provider_name",				CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, provider_email),					"provider_email",				CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, provider_website),				"provider_website",				CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL					},
	{ offsetof(network_configuration, known_keys_only),					"known_keys_only",				CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
	{ offsetof(network_configuration, log_chat),						"log_chat",						CONFIG_VALUE_TYPE_BOOLEAN,		false,							NULL					},
};

config_property_definition _notificationsDefinitions[] = {
	{ offsetof(notification_configuration, park_award),							"park_award",							CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, park_marketing_campaign_finished),	"park_marketing_campaign_finished",		CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, park_warnings),						"park_warnings",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, park_rating_warnings),				"park_rating_warnings",					CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, ride_broken_down),					"ride_broken_down",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, ride_crashed),						"ride_crashed",							CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, ride_warnings),						"ride_warnings",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, ride_researched),					"ride_researched",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_warnings),						"guest_warnings",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_lost),							"guest_lost",							CONFIG_VALUE_TYPE_BOOLEAN,	false,	NULL	},
	{ offsetof(notification_configuration, guest_left_park),					"guest_entered_left_park",				CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_queuing_for_ride),				"guest_queuing_for_ride",				CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_on_ride),						"guest_on_ride",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_left_ride),					"guest_left_ride",						CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_bought_item),					"guest_bought_item",					CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_used_facility),				"guest_used_facility",					CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
	{ offsetof(notification_configuration, guest_died),							"guest_died",							CONFIG_VALUE_TYPE_BOOLEAN,	true,	NULL	},
};

config_property_definition _fontsDefinitions[] = {
	{ offsetof(font_configuration, file_name),				"file_name",			CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL			},
	{ offsetof(font_configuration, font_name),				"font_name",			CONFIG_VALUE_TYPE_STRING,		{.value_string = NULL },		NULL			},
	{ offsetof(font_configuration, x_offset),				"x_offset",				CONFIG_VALUE_TYPE_SINT8,		0,								NULL			},
	{ offsetof(font_configuration, y_offset),				"y_offset",				CONFIG_VALUE_TYPE_SINT8,		-1,								NULL			},
	{ offsetof(font_configuration, size_tiny),				"size_tiny",			CONFIG_VALUE_TYPE_UINT8,		8,								NULL			},
	{ offsetof(font_configuration, size_small),				"size_small",			CONFIG_VALUE_TYPE_UINT8,		10,								NULL			},
	{ offsetof(font_configuration, size_medium),			"size_medium",			CONFIG_VALUE_TYPE_UINT8,		11,								NULL			},
	{ offsetof(font_configuration, size_big),				"size_big",				CONFIG_VALUE_TYPE_UINT8,		12,								NULL			},
	{ offsetof(font_configuration, height_tiny),			"height_tiny",			CONFIG_VALUE_TYPE_UINT8,		6,								NULL			},
	{ offsetof(font_configuration, height_small),			"height_small",			CONFIG_VALUE_TYPE_UINT8,		12,								NULL			},
	{ offsetof(font_configuration, height_medium),			"height_medium",		CONFIG_VALUE_TYPE_UINT8,		12,								NULL			},
	{ offsetof(font_configuration, height_big),				"height_big",			CONFIG_VALUE_TYPE_UINT8,		20,								NULL			}
};

config_section_definition _sectionDefinitions[] = {
	{ &gConfigGeneral, "general", _generalDefinitions, countof(_generalDefinitions) },
	{ &gConfigInterface, "interface", _interfaceDefinitions, countof(_interfaceDefinitions) },
	{ &gConfigSound, "sound", _soundDefinitions, countof(_soundDefinitions) },
	{ &gConfigTwitch, "twitch", _twitchDefinitions, countof(_twitchDefinitions) },
	{ &gConfigNetwork, "network", _networkDefinitions, countof(_networkDefinitions) },
	{ &gConfigNotifications, "notifications", _notificationsDefinitions, countof(_notificationsDefinitions) },
	{ &gConfigFonts, "fonts", _fontsDefinitions, countof(_fontsDefinitions) }
};

#pragma endregion

general_configuration gConfigGeneral;
interface_configuration gConfigInterface;
sound_configuration gConfigSound;
twitch_configuration gConfigTwitch;
network_configuration gConfigNetwork;
notification_configuration gConfigNotifications;
font_configuration gConfigFonts;

static bool config_open(const utf8string path);
static bool config_save(const utf8string path);
static void config_read_properties(config_section_definition **currentSection, const_utf8string line);
static void config_save_property_value(SDL_RWops *file, uint8 type, value_union *value);
static bool config_read_enum(void *dest, sint32 destSize, const utf8 *key, sint32 keySize, config_enum_definition *enumDefinitions);
static void config_write_enum(SDL_RWops *file, uint8 type, value_union *value, config_enum_definition *enumDefinitions);

static void utf8_skip_whitespace(utf8 **outch);
static void utf8_skip_non_whitespace(utf8 **outch);

static sint32 rwopsreadc(SDL_RWops *file)
{
	sint32 c = 0;
	if (SDL_RWread(file, &c, 1, 1) != 1)
		c = EOF;
	return c;
}

static void rwopswritec(SDL_RWops *file, char c)
{
	SDL_RWwrite(file, &c, 1, 1);
}

static void rwopswritestr(SDL_RWops *file, const char *str)
{
	SDL_RWwrite(file, str, strlen(str), 1);
}

static void rwopsprintf(SDL_RWops *file, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[64];
	vsnprintf(buffer, 64, format, args);

	SDL_RWwrite(file, buffer, strlen(buffer), 1);

	va_end(args);
}

static void rwopswritenewline(SDL_RWops *file)
{
	rwopswritestr(file, PLATFORM_NEWLINE);
}

static void rwopswritestresc(SDL_RWops *file, const char *str) {
	sint32 length = 0;
	for (const char *c = str; *c != '\0'; ++c) {
		if (*c == '\\') length += 2;
		else ++length;
	}

	char *escaped = malloc(length + 1);
	sint32 j=0;
	for (const char *c = str; *c != '\0'; ++c) {
		if (*c == '\\') escaped[j++] = '\\';
		escaped[j++] = *c;
	}
	escaped[length] = '\0';

	rwopswritestr(file, escaped);
	SafeFree(escaped);
}

void config_set_defaults()
{
	sint32 i, j;

	for (i = 0; i < countof(_sectionDefinitions); i++) {
		config_section_definition *section = &_sectionDefinitions[i];
		for (j = 0; j < section->property_definitions_count; j++) {
			config_property_definition *property = &section->property_definitions[j];
			value_union *destValue = (value_union*)((size_t)section->base_address + (size_t)property->offset);

			// Special dynamic defaults
			if (strcmp(property->property_name, "language") == 0){
				destValue->value_uint16 = platform_get_locale_language();
				if (destValue->value_uint16 == LANGUAGE_UNDEFINED)
					memcpy(destValue, &property->default_value, _configValueTypeSize[property->type]);
			}
			else if (strcmp(property->property_name, "currency_format") == 0){
				destValue->value_uint8 = platform_get_locale_currency();
			}
			else if (strcmp(property->property_name, "measurement_format") == 0){
				destValue->value_uint8 = platform_get_locale_measurement_format();
			}
			else if (strcmp(property->property_name, "temperature_format") == 0){
				destValue->value_uint8 = platform_get_locale_temperature_format();
			}
			else if (strcmp(property->property_name, "player_name") == 0) {
				utf8* username = platform_get_username();

				if (username) {
					destValue->value_string = _strdup(username);
				} else {
					destValue->value_string = _strdup(language_get_string(STR_MULTIPLAYER_DEFAULT_NAME));
				}
			}
			else {
				// Use static default
				if (property->type == CONFIG_VALUE_TYPE_STRING) {
					// Copy the string to new memory
					const utf8 *src = property->default_value.value_string;
					const utf8 **dst = (const utf8**)&(destValue->value_string);
					if (src != NULL) {
						*dst = _strdup(property->default_value.value_string);
					}
				} else {
					memcpy(destValue, &property->default_value, _configValueTypeSize[property->type]);
				}
			}
		}
	}
}

void config_release()
{
	for (sint32 i = 0; i < countof(_sectionDefinitions); i++) {
		config_section_definition *section = &_sectionDefinitions[i];
		for (sint32 j = 0; j < section->property_definitions_count; j++) {
			config_property_definition *property = &section->property_definitions[j];
			value_union *destValue = (value_union*)((size_t)section->base_address + (size_t)property->offset);
			if (property->type == CONFIG_VALUE_TYPE_STRING) {
				utf8 **dst = (utf8**)&(destValue->value_string);
				SafeFree(*dst);
			}
		}
	}
}

void config_get_default_path(utf8 *outPath, size_t size)
{
	platform_get_user_directory(outPath, NULL, size);
	safe_strcat_path(outPath, "config.ini", size);
}

bool config_open_default()
{
	utf8 path[MAX_PATH];

	config_get_default_path(path, sizeof(path));
	if (config_open(path)) {
		currency_load_custom_currency_config();
		return true;
	}

	return false;
}

bool config_save_default()
{
	utf8 path[MAX_PATH];

	config_get_default_path(path, sizeof(path));
	if (config_save(path)) {
		return true;
	}

	return false;
}

bool config_open(const utf8string path)
{
	SDL_RWops *file;
	utf8string lineBuffer;
	size_t lineBufferCapacity;
	size_t lineLength;
	sint32 c;
	config_section_definition *currentSection;

	file = SDL_RWFromFile(path, "rb");
	if (file == NULL)
		return false;

	currentSection = NULL;
	lineBufferCapacity = 64;
	lineBuffer = malloc(lineBufferCapacity);
	lineLength = 0;

	// Skim UTF-8 byte order mark
	SDL_RWread(file, lineBuffer, 3, 1);
	if (!utf8_is_bom(lineBuffer))
		SDL_RWseek(file, 0, RW_SEEK_SET);

	while ((c = rwopsreadc(file)) != EOF) {
		if (c == '\n' || c == '\r') {
			lineBuffer[lineLength] = 0;
			config_read_properties(&currentSection, (const_utf8string)lineBuffer);
			lineLength = 0;
		} else {
			lineBuffer[lineLength++] = c;
		}

		if (lineLength >= lineBufferCapacity) {
			lineBufferCapacity *= 2;
			lineBuffer = realloc(lineBuffer, lineBufferCapacity);
		}
	}

	if (lineLength > 0) {
		lineBuffer[lineLength++] = 0;
		config_read_properties(&currentSection, lineBuffer);
	}

	free(lineBuffer);
	SDL_RWclose(file);

	// Window limit must be kept within valid range.
	if (gConfigGeneral.window_limit < WINDOW_LIMIT_MIN) {
		gConfigGeneral.window_limit = WINDOW_LIMIT_MIN;
	}
	else if (gConfigGeneral.window_limit > WINDOW_LIMIT_MAX) {
		gConfigGeneral.window_limit = WINDOW_LIMIT_MAX;
	}

	return true;
}

bool config_save(const utf8string path)
{
	SDL_RWops *file;
	sint32 i, j;
	value_union *value;

	file = SDL_RWFromFile(path, "wb");
	if (file == NULL) {
		log_error("Unable to write to config file.");
		return false;
	}

	for (i = 0; i < countof(_sectionDefinitions); i++) {
		config_section_definition *section = &_sectionDefinitions[i];

		rwopswritec(file, '[');
		rwopswritestr(file, section->section_name);
		rwopswritec(file, ']');
		rwopswritenewline(file);

		for (j = 0; j < section->property_definitions_count; j++) {
			config_property_definition *property = &section->property_definitions[j];

			rwopswritestr(file, property->property_name);
			rwopswritestr(file, " = ");

			value = (value_union*)((size_t)section->base_address + (size_t)property->offset);
			if (property->enum_definitions != NULL)
				config_write_enum(file, property->type, value, property->enum_definitions);
			else
				config_save_property_value(file, property->type, value);
			rwopswritenewline(file);
		}
		rwopswritenewline(file);
	}

	SDL_RWclose(file);
	return true;
}

static void config_save_property_value(SDL_RWops *file, uint8 type, value_union *value)
{
	switch (type) {
	case CONFIG_VALUE_TYPE_BOOLEAN:
		if (value->value_boolean) rwopswritestr(file, "true");
		else rwopswritestr(file, "false");
		break;
	case CONFIG_VALUE_TYPE_UINT8:
		rwopsprintf(file, "%u", value->value_uint8);
		break;
	case CONFIG_VALUE_TYPE_UINT16:
		rwopsprintf(file, "%u", value->value_uint16);
		break;
	case CONFIG_VALUE_TYPE_UINT32:
		rwopsprintf(file, "%lu", value->value_uint32);
		break;
	case CONFIG_VALUE_TYPE_SINT8:
		rwopsprintf(file, "%d", value->value_sint8);
		break;
	case CONFIG_VALUE_TYPE_SINT16:
		rwopsprintf(file, "%d", value->value_sint16);
		break;
	case CONFIG_VALUE_TYPE_SINT32:
		rwopsprintf(file, "%ld", value->value_sint32);
		break;
	case CONFIG_VALUE_TYPE_FLOAT:
		rwopsprintf(file, "%.3f", value->value_float);
		break;
	case CONFIG_VALUE_TYPE_DOUBLE:
		rwopsprintf(file, "%.6f", value->value_double);
		break;
	case CONFIG_VALUE_TYPE_STRING:
		rwopswritec(file, '"');
		if (value->value_string != NULL) {
			rwopswritestresc(file, value->value_string);
		}
		rwopswritec(file, '"');
		break;
	}
}

static bool config_get_section(const utf8string line, const utf8 **sectionName, sint32 *sectionNameSize)
{
	utf8 *ch;
	sint32 c;

	ch = line;
	utf8_skip_whitespace(&ch);
	if (*ch != '[') return false;
	*sectionName = ++ch;

	while ((c = utf8_get_next(ch, (const utf8**)&ch)) != 0) {
		if (c == '#') return false;
		if (c == '[') return false;
		if (c == ' ') break;
		if (c == ']') break;
	}

	*sectionNameSize = (sint32)(ch - *sectionName - 1);
	return true;
}

static bool config_get_property_name_value(const utf8string line, utf8 **propertyName, sint32 *propertyNameSize, utf8 **value, sint32 *valueSize)
{
	utf8 *ch, *clast;
	sint32 c;
	bool quotes;

	ch = line;
	utf8_skip_whitespace(&ch);

	if (*ch == 0) return false;
	*propertyName = ch;

	bool equals = false;
	while ((c = utf8_get_next(ch, (const utf8**)&ch)) != 0) {
		if (isspace(c) || c == '=') {
			if (c == '=') equals = true;
			*propertyNameSize = (sint32)(ch - *propertyName - 1);
			break;
		} else if (c == '#') {
			return false;
		}
	}

	if (*ch == 0) return false;
	utf8_skip_whitespace(&ch);
	if (!equals) {
		if (*ch != '=') return false;
		ch++;
		utf8_skip_whitespace(&ch);
	}
	if (*ch == 0) return false;

	if (*ch == '"') {
		ch++;
		quotes = true;
	} else {
		quotes = false;
	}
	*value = ch;

	clast = ch;
	while ((c = utf8_get_next(ch, (const utf8**)&ch)) != 0) {
		if (!quotes) {
			if (c == '#') break;
			if (c != ' ') clast = ch;
		}
	}
	if (!quotes) *valueSize = (sint32)(clast - *value);
	else *valueSize = (sint32)(ch - *value - 1);
	if (quotes) (*valueSize)--;
	return true;
}

static config_section_definition *config_get_section_def(const utf8 *name, sint32 size)
{
	sint32 i;

	for (i = 0; i < countof(_sectionDefinitions); i++) {
		const_utf8string sectionName = _sectionDefinitions[i].section_name;
		sint32 sectionNameSize = (sint32)strnlen(sectionName, size);
		if (sectionNameSize == size && sectionName[size] == 0 && _strnicmp(sectionName, name, size) == 0)
			return &_sectionDefinitions[i];
	}

	return NULL;
}

static config_property_definition *config_get_property_def(config_section_definition *section, const utf8 *name, sint32 size)
{
	sint32 i;

	for (i = 0; i < section->property_definitions_count; i++) {
		const_utf8string propertyName = section->property_definitions[i].property_name;
		sint32 propertyNameSize = (sint32)strnlen(propertyName, size);
		if (propertyNameSize == size && propertyName[size] == 0 && _strnicmp(propertyName, name, size) == 0)
		{
			return &section->property_definitions[i];
		}
	}

	return NULL;
}

static utf8string escape_string(const utf8 *value, sint32 valueSize) {
	sint32 length = 0;
	bool backslash = false;
	for (sint32 i=0; i < valueSize; ++i) {
		if (value[i] == '\\') {
			if (backslash) backslash = false;
			else ++length, backslash = true;
		} else ++length, backslash = false;
	}
	utf8string escaped = malloc(length + 1);

	sint32 j=0;
	backslash = false;
	for (sint32 i=0; i < valueSize; ++i) {
		if (value[i] == '\\') {
			if (backslash) backslash = false;
			else escaped[j++] = value[i], backslash = true;
		} else escaped[j++] = value[i], backslash = false;
	}
	escaped[length] = '\0';

	return escaped;
}

static void config_set_property(const config_section_definition *section, const config_property_definition *property, const utf8 *value, sint32 valueSize)
{
	value_union *destValue = (value_union*)((size_t)section->base_address + (size_t)property->offset);

	if (property->enum_definitions != NULL)
		if (config_read_enum(destValue, (sint32)_configValueTypeSize[property->type], value, valueSize, property->enum_definitions))
			return;

	switch (property->type) {
	case CONFIG_VALUE_TYPE_BOOLEAN:
		if (_strnicmp(value, "false", valueSize) == 0) destValue->value_boolean = false;
		else if (_strnicmp(value, "true", valueSize) == 0) destValue->value_boolean = true;
		else destValue->value_boolean = strtol(value, NULL, 0) != 0;
		break;
	case CONFIG_VALUE_TYPE_UINT8:
		destValue->value_uint8 = (uint8)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_UINT16:
		destValue->value_uint16 = (uint16)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_UINT32:
		destValue->value_uint32 = (uint32)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_SINT8:
		destValue->value_sint8 = (sint8)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_SINT16:
		destValue->value_sint16 = (sint16)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_SINT32:
		destValue->value_sint32 = (sint32)strtol(value, NULL, 0);
		break;
	case CONFIG_VALUE_TYPE_FLOAT:
		destValue->value_float = strtof(value, NULL);
		break;
	case CONFIG_VALUE_TYPE_DOUBLE:
		destValue->value_double = strtod(value, NULL);
		break;
	case CONFIG_VALUE_TYPE_STRING:
		SafeFree(destValue->value_string);
		destValue->value_string = escape_string(value, valueSize);
		break;
	}
}

static void config_read_properties(config_section_definition **currentSection, const_utf8string line)
{
	utf8 *ch = (utf8*)line;
	utf8_skip_whitespace(&ch);

	if (*ch == '[') {
		const utf8 *sectionName;
		sint32 sectionNameSize;
		if (config_get_section(ch, &sectionName, &sectionNameSize))
			*currentSection = config_get_section_def(sectionName, sectionNameSize);
	} else {
		if (*currentSection != NULL) {
			utf8 *propertyName, *value;
			sint32 propertyNameSize = 0, valueSize;
			if (config_get_property_name_value(ch, &propertyName, &propertyNameSize, &value, &valueSize)) {
				config_property_definition *property;
				property = config_get_property_def(*currentSection, propertyName, propertyNameSize);
				if (property != NULL)
					config_set_property(*currentSection, property, value, valueSize);
			}
		}
	}
}

static bool config_read_enum(void *dest, sint32 destSize, const utf8 *key, sint32 keySize, config_enum_definition *enumDefinitions)
{
	while (enumDefinitions->key != NULL) {
		if (strlen(enumDefinitions->key) == (size_t)keySize && _strnicmp(enumDefinitions->key, key, keySize) == 0) {
			memcpy(dest, &enumDefinitions->value.value_uint32, destSize);
			return true;
		}
		enumDefinitions++;
	}
	return false;
}

static void config_write_enum(SDL_RWops *file, uint8 type, value_union *value, config_enum_definition *enumDefinitions)
{
	uint32 enumValue = (value->value_uint32) & ((1 << (_configValueTypeSize[type] * 8)) - 1);
	while (enumDefinitions->key != NULL) {
		if (enumDefinitions->value.value_uint32 == enumValue) {
			rwopswritestr(file, enumDefinitions->key);
			return;
		}
		enumDefinitions++;
	}
	config_save_property_value(file, type, value);
}

static void utf8_skip_whitespace(utf8 **outch)
{
	utf8 *ch;
	while (**outch != 0) {
		ch = *outch;
		if (!isspace(utf8_get_next(*outch, (const utf8**)outch))) {
			*outch = ch;
			break;
		}
	}
}

static void utf8_skip_non_whitespace(utf8 **outch)
{
	while (**outch != 0) {
		if (isspace(utf8_get_next(*outch, (const utf8**)outch)))
			break;
	}
}

/*

Code reserved for when we want more intelligent saving of config file which preserves comments and layout

enum {
	CONFIG_LINE_TYPE_WHITESPACE,
	CONFIG_LINE_TYPE_COMMENT,
	CONFIG_LINE_TYPE_SECTION,
	CONFIG_LINE_TYPE_PROPERTY,
	CONFIG_LINE_TYPE_INVALID
};

typedef struct {
	uint8 type;
	utf8string line;
} config_line;

static config_line *_configLines = NULL;

*/

/**
 * Attempts to find the RCT2 installation directory.
 * This should be created from some other resource when OpenRCT2 grows.
 * @param resultPath Pointer to where the absolute path of the RCT2 installation directory will be copied to.
 * @returns 1 if successful, otherwise 0.
 */
static bool config_find_rct2_path(utf8 *resultPath)
{
	sint32 i;

	log_verbose("searching common installation locations.");

	const utf8 *searchLocations[] = {
		"C:\\Program Files\\Infogrames\\RollerCoaster Tycoon 2",
		"C:\\Program Files (x86)\\Infogrames\\RollerCoaster Tycoon 2",
		"C:\\Program Files\\Infogrames Interactive\\RollerCoaster Tycoon 2",
		"C:\\Program Files (x86)\\Infogrames Interactive\\RollerCoaster Tycoon 2",
		"C:\\Program Files\\Atari\\RollerCoaster Tycoon 2",
		"C:\\Program Files (x86)\\Atari\\RollerCoaster Tycoon 2",
		"C:\\GOG Games\\RollerCoaster Tycoon 2 Triple Thrill Pack",
		"C:\\Program Files\\GalaxyClient\\Games\\RollerCoaster Tycoon 2 Triple Thrill Pack",
		"C:\\Program Files (x86)\\GalaxyClient\\Games\\RollerCoaster Tycoon 2 Triple Thrill Pack",
		"C:\\Program Files\\Steam\\steamapps\\common\\Rollercoaster Tycoon 2",
		"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Rollercoaster Tycoon 2",
		gExePath
	};

	for (i = 0; i < countof(searchLocations); i++) {
		if (platform_original_game_data_exists(searchLocations[i])) {
			safe_strcpy(resultPath, searchLocations[i], MAX_PATH);
			return true;
		}
	}

	return false;
}

bool config_find_or_browse_install_directory()
{
	utf8 path[MAX_PATH];
	utf8 *installPath;

	if (config_find_rct2_path(path)) {
		SafeFree(gConfigGeneral.rct2_path);
		gConfigGeneral.rct2_path = malloc(strlen(path) + 1);
		safe_strcpy(gConfigGeneral.rct2_path, path, MAX_PATH);
	} else {
		if (gOpenRCT2Headless) {
			return false;
		}
		while (1) {
			platform_show_messagebox("OpenRCT2 needs files from the original RollerCoaster Tycoon 2 in order to work. Please select the directory where you installed RollerCoaster Tycoon 2.");
			installPath = platform_open_directory_browser("Please select your RCT2 directory");
			if (installPath == NULL)
				return false;

			SafeFree(gConfigGeneral.rct2_path);
			gConfigGeneral.rct2_path = installPath;

			if (platform_original_game_data_exists(installPath))
				return true;

			utf8 message[MAX_PATH];
			snprintf(message, MAX_PATH, "Could not find %s" PATH_SEPARATOR "Data" PATH_SEPARATOR "g1.dat at this path", installPath);
			platform_show_messagebox(message);
		}
	}

	return true;
}

#pragma region Shortcuts

// Current keyboard shortcuts
keypress gShortcutKeys[SHORTCUT_COUNT];

// Default keyboard shortcuts
static const keypress _defaultShortcutKeys[SHORTCUT_COUNT] = {
	{ SDLK_BACKSPACE,	KMOD_NONE },					// SHORTCUT_CLOSE_TOP_MOST_WINDOW
	{ SDLK_BACKSPACE,	KMOD_SHIFT },					// SHORTCUT_CLOSE_ALL_FLOATING_WINDOWS
	{ SDLK_ESCAPE,		KMOD_NONE },					// SHORTCUT_CANCEL_CONSTRUCTION_MODE
	{ SDLK_PAUSE,		KMOD_NONE },					// SHORTCUT_PAUSE_GAME
	{ SDLK_PAGEUP,		KMOD_NONE },					// SHORTCUT_ZOOM_VIEW_OUT
	{ SDLK_PAGEDOWN,	KMOD_NONE },					// SHORTCUT_ZOOM_VIEW_IN
	{ SDLK_RETURN,		KMOD_NONE },					// SHORTCUT_ROTATE_VIEW_CLOCKWISE
	{ SDLK_RETURN,		KMOD_SHIFT },					// SHORTCUT_ROTATE_VIEW_ANTICLOCKWISE
	{ SDLK_z,			KMOD_NONE },					// SHORTCUT_ROTATE_CONSTRUCTION_OBJECT
	{ SDLK_1,			KMOD_NONE },					// SHORTCUT_UNDERGROUND_VIEW_TOGGLE
	{ SDLK_h,			KMOD_NONE },					// SHORTCUT_REMOVE_BASE_LAND_TOGGLE
	{ SDLK_v,			KMOD_NONE },					// SHORTCUT_REMOVE_VERTICAL_LAND_TOGGLE
	{ SDLK_3,			KMOD_NONE },					// SHORTCUT_SEE_THROUGH_RIDES_TOGGLE
	{ SDLK_4,			KMOD_NONE },					// SHORTCUT_SEE_THROUGH_SCENERY_TOGGLE
	{ SDLK_5,			KMOD_NONE },					// SHORTCUT_INVISIBLE_SUPPORTS_TOGGLE
	{ SDLK_6,			KMOD_NONE },					// SHORTCUT_INVISIBLE_PEOPLE_TOGGLE
	{ SDLK_8,			KMOD_NONE },					// SHORTCUT_HEIGHT_MARKS_ON_LAND_TOGGLE
	{ SDLK_9,			KMOD_NONE },					// SHORTCUT_HEIGHT_MARKS_ON_RIDE_TRACKS_TOGGLE
	{ SDLK_0,			KMOD_NONE },					// SHORTCUT_HEIGHT_MARKS_ON_PATHS_TOGGLE
	{ SDLK_F1,			KMOD_NONE },					// SHORTCUT_ADJUST_LAND
	{ SDLK_F2,			KMOD_NONE },					// SHORTCUT_ADJUST_WATER
	{ SDLK_F3,			KMOD_NONE },					// SHORTCUT_BUILD_SCENERY
	{ SDLK_F4,			KMOD_NONE },					// SHORTCUT_BUILD_PATHS
	{ SDLK_F5,			KMOD_NONE },					// SHORTCUT_BUILD_NEW_RIDE
	{ SDLK_f,			KMOD_NONE },					// SHORTCUT_SHOW_FINANCIAL_INFORMATION
	{ SDLK_d,			KMOD_NONE },					// SHORTCUT_SHOW_RESEARCH_INFORMATION
	{ SDLK_r,			KMOD_NONE },					// SHORTCUT_SHOW_RIDES_LIST
	{ SDLK_p,			KMOD_NONE },					// SHORTCUT_SHOW_PARK_INFORMATION
	{ SDLK_g,			KMOD_NONE },					// SHORTCUT_SHOW_GUEST_LIST
	{ SDLK_s,			KMOD_NONE },					// SHORTCUT_SHOW_STAFF_LIST
	{ SDLK_m,			KMOD_NONE },					// SHORTCUT_SHOW_RECENT_MESSAGES
	{ SDLK_TAB,			KMOD_NONE },					// SHORTCUT_SHOW_MAP
	{ SDLK_s,			PLATFORM_MODIFIER },			// SHORTCUT_SCREENSHOT

	 // New
	{ SDLK_MINUS,		KMOD_NONE },					// SHORTCUT_REDUCE_GAME_SPEED,
	{ SDLK_EQUALS,		KMOD_NONE },					// SHORTCUT_INCREASE_GAME_SPEED,
	{ SDLK_c,			PLATFORM_MODIFIER | KMOD_ALT },	// SHORTCUT_OPEN_CHEAT_WINDOW,
	{ SDLK_t,			KMOD_NONE },					// SHORTCUT_REMOVE_TOP_BOTTOM_TOOLBAR_TOGGLE,
	{ SDLK_UP,			KMOD_NONE },					// SHORTCUT_SCROLL_MAP_UP
	{ SDLK_LEFT,		KMOD_NONE },					// SHORTCUT_SCROLL_MAP_LEFT
	{ SDLK_DOWN,		KMOD_NONE },					// SHORTCUT_SCROLL_MAP_DOWN
	{ SDLK_RIGHT,		KMOD_NONE },					// SHORTCUT_SCROLL_MAP_RIGHT
	{ SDLK_c,			KMOD_NONE },					// SHORTCUT_OPEN_CHAT_WINDOW
	{ SDLK_F10,			PLATFORM_MODIFIER },			// SHORTCUT_QUICK_SAVE_GAME

	SHORTCUT_UNDEFINED,									// SHORTCUT_SHOW_OPTIONS
	SHORTCUT_UNDEFINED,									// SHORTCUT_MUTE_SOUND
	{ SDLK_RETURN,		KMOD_ALT },						// SHORTCUT_WINDOWED_MODE_TOGGLE
	SHORTCUT_UNDEFINED,									// SHORTCUT_SHOW_MULTIPLAYER
	SHORTCUT_UNDEFINED,									// SHORTCUT_PAINT_ORIGINAL_TOGGLE
	SHORTCUT_UNDEFINED,									// SHORTCUT_DEBUG_PAINT_TOGGLE
	SHORTCUT_UNDEFINED									// SHORTCUT_SEE_THROUGH_PATHS_TOGGLE
};

#define SHORTCUT_FILE_VERSION 2

/**
 *
 *  rct2: 0x006E3604
 */
void config_reset_shortcut_keys()
{
	memcpy(gShortcutKeys, _defaultShortcutKeys, sizeof(gShortcutKeys));
}

static void config_shortcut_keys_get_path(utf8 *outPath, size_t size)
{
	platform_get_user_directory(outPath, NULL, size);
	safe_strcat_path(outPath, "hotkeys.cfg", size);
}

bool config_shortcut_keys_load()
{
	utf8 path[MAX_PATH];
	SDL_RWops *file;
	bool result;
	uint16 version;

	config_shortcut_keys_get_path(path, sizeof(path));

	file = SDL_RWFromFile(path, "rb");
	if (file != NULL) {
		result = SDL_RWread(file, &version, sizeof(version), 1) == 1;
		if (result && version == SHORTCUT_FILE_VERSION) {
			for (sint32 i = 0; i < SHORTCUT_COUNT; i++) {
				if (SDL_RWread(file, &gShortcutKeys[i], sizeof(keypress), 1) != 1) {
					break;
				}
			}
		} else {
			log_verbose("Shortcut config file is outdated (expected: %d, got %d). "\
						"Loading default shorcut keys.", version, SHORTCUT_FILE_VERSION);
			result = false;
		}
		SDL_RWclose(file);
	} else {
		result = false;
	}

	return result;
}

bool config_shortcut_keys_save()
{
	const uint16 version = SHORTCUT_FILE_VERSION;

	utf8 path[MAX_PATH];
	SDL_RWops *file;
	bool result;

	config_shortcut_keys_get_path(path, sizeof(path));

	file = SDL_RWFromFile(path, "wb");
	if (file != NULL) {
		result = SDL_RWwrite(file, &version, sizeof(version), 1) == 1;
		if (result) {
			result = SDL_RWwrite(file, gShortcutKeys, sizeof(gShortcutKeys), 1) == 1;
		}
		SDL_RWclose(file);
	} else {
		result = false;
	}

	return result;
}

#pragma endregion
