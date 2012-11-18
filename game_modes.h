//
//  game_modes.h
//  CaveStory
//
//  Created by Pavlo Ilin on 18.11.12.
//
//

#ifndef CaveStory_game_modes_h
#define CaveStory_game_modes_h

// game modes (changes *tickfunction)
enum GameModes
{
	GM_NONE,			// default mode at startup & shutdown
	GM_NORMAL,			// playing the game
	GM_INVENTORY,		// in inventory screen
	GM_MAP_SYSTEM,		// viewing Map System
	GM_ISLAND,			// XX1 good-ending island-crash cutscene
	GM_CREDITS,			// <CRE credits
	GM_INTRO,			// intro
	GM_TITLE,			// title screen
	
	GP_PAUSED,			// pausemode: Pause (use game.pause())
	GP_OPTIONS,			// pausemode: Options (use game.pause())
	
	NUM_GAMEMODES
};

GameModes getGamemode();


#endif
