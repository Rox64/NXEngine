
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "settings.h"
#include "replay.h"
#include "settings.fdh"
#include "platform/platform.h"

const char *setfilename = "settings.dat";
const uint16_t SETTINGS_VERSION = 0x1608;		// serves as both a version and magic

Settings normal_settings;
Settings replay_settings;
Settings *settings = &normal_settings;


bool settings_load(Settings *setfile)
{
	if (!setfile) setfile = &normal_settings;
	
	if (tryload(settings))
	{
		stat("No saved settings; using defaults.");
		
		memset(setfile, 0, sizeof(Settings));
		setfile->resolution = 3;		// 640x480 Windowed, should be safe value
		setfile->last_save_slot = 0;
		setfile->multisave = true;
		
		setfile->enable_debug_keys = false;
		setfile->sound_enabled = true;
		setfile->music_enabled = 1;	// both Boss and Regular music
		
		setfile->instant_quit = false;
		setfile->emulate_bugs = false;
		setfile->no_quake_in_hell = false;
		setfile->inhibit_fullscreen = false;
		setfile->files_extracted = false;
		
		// I found that 8bpp->32bpp blits are actually noticably faster
		// than 32bpp->32bpp blits on several systems I tested. Not sure why
		// but calling SDL_DisplayFormat seems to actually be slowing things
		// down. This goes against established wisdom so if you want it back on,
		// run "displayformat 1" in the console and restart.
		setfile->displayformat = false;
        
        setfile->show_fps = true;
        
        
        setfile->tap[Settings::Tap::EAll]           = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EMovies]        = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::ETitle]         = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::ESaveLoad]      = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EIngameDialog]  = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EInventory]     = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EPause]         = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EOptions]       = Settings::Tap::ETAP;
        setfile->tap[Settings::Tap::EMapSystem]     = Settings::Tap::ETAP;
		
        setfile->vjoy_controls = VJoy::getPreset(0);
        setfile->vjoy_current_preset = 0;
        setfile->vjoy_show_mode = 0;
        VJoy::setUpdated();
        
		return 1;
	}
	else
	{
		#ifdef __SDLSHIM__
			stat("settings_load(): Hey FIXME!!!");
			settings->show_fps = true;
		#else
			input_set_mappings(settings->input_mappings);
		#endif
        
        VJoy::setUpdated();
	}
	
	return 0;
}

/*
void c------------------------------() {}
*/

static bool tryload(Settings *setfile)
{
FILE *fp;

	stat("Loading settings...");
	
	fp = fileopenRW(setfilename, "rb");
	if (!fp)
	{
		stat("Couldn't open file %s.", setfilename);
		return 1;
	}
	
	setfile->version = 0;
	fread(setfile, sizeof(Settings), 1, fp);
	if (setfile->version != SETTINGS_VERSION)
	{
		stat("Wrong settings version %04x.", setfile->version);
		return 1;
	}
	
	fclose(fp);
	return 0;
}


bool settings_save(Settings *setfile)
{
FILE *fp;

	if (!setfile)
		setfile = &normal_settings;
	
	stat("Writing settings...");
	fp = fileopenRW(setfilename, "wb");
	if (!fp)
	{
		stat("Couldn't open file %s.", setfilename);
		return 1;
	}
	
	for(int i=0;i<INPUT_COUNT;i++)
		setfile->input_mappings[i] = input_get_mapping(i);
	
	setfile->version = SETTINGS_VERSION;
	fwrite(setfile, sizeof(Settings), 1, fp);
	
	fclose(fp);
	return 0;
}




