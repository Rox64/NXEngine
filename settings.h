
#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "input.h"
#include "nx_math.h"
#include "vjoy.h"


struct Settings
{
	uint16_t version;
	int resolution;
	int last_save_slot;
	bool multisave;
	bool files_extracted;
	bool show_fps;
	bool displayformat;
	
	bool enable_debug_keys;
	bool sound_enabled;
	int music_enabled;
	
	bool instant_quit;
	bool emulate_bugs;
	bool no_quake_in_hell;
	bool inhibit_fullscreen;
	
	bool skip_intro;
	int reserved[8];
	
	int input_mappings[INPUT_COUNT];
    
    struct Tap
    {
        enum Mode
        {
            ETAP,
            EPAD,
            EBOTH,
            
            EMODELAST
        };
        
        enum Place
        {
            EAll,
            EMovies,
            ETitle,
            ESaveLoad,
            EIngameDialog,
            EInventory,
            EPause,
            EOptions,
            EMapSystem,
            
            ELASTPLACE
        };
    };
    
    uint8_t tap[Tap::ELASTPLACE];
    
    VJoy::Preset vjoy_controls;
    int vjoy_current_preset;
    int vjoy_show_mode;
};

bool settings_load(Settings *settings=NULL);
bool settings_save(Settings *settings=NULL);

extern Settings *settings;
extern Settings normal_settings;
extern Settings replay_settings;


#endif
