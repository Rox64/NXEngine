
#include "../nx.h"
#include "title.fdh"
#include "../vjoy.h"

// music and character selections for the different Counter times
static struct
{
	uint32_t timetobeat;
	int sprite;
	int songtrack;
} titlescreens[] =
{
	(3*3000),	SPR_CS_SUE,    2,		// 3 mins	- Sue & Safety
	(4*3000),	SPR_CS_KING,   41,		// 4 mins	- King & White
	(5*3000),	SPR_CS_TOROKO, 40,		// 5 mins	- Toroko & Toroko's Theme
	(6*3000),	SPR_CS_CURLY,  36,		// 6 mins	- Curly & Running Hell
	0xFFFFFFFF, SPR_CS_MYCHAR, 24		// default
};

// artifical fake "loading" delay between selecting an option and it being executed,
// because it actually doesn't look good if we respond instantly.
#define SELECT_DELAY			30
#define SELECT_LOAD_DELAY		20		// delay when leaving the multisave Load dialog
#define SELECT_MENU_DELAY		8		// delay from Load to load menu

static struct
{
	int sprite;
	int cursel;
	int selframe, seltimer;
	int selchoice, seldelay;
	int kc_pos;
	bool in_multiload;
	
	uint32_t besttime;		// Nikumaru display
} title;


bool title_init(int param)
{
	memset(&title, 0, sizeof(title));
	game.switchstage.mapno = 0;
	game.switchstage.eventonentry = 0;
	game.showmapnametime = 0;
	textbox.SetVisible(false);
	
	if (niku_load(&title.besttime))
		title.besttime = 0xffffffff;
	
	// select a title screen based on Nikumaru time
	int t;
	for(t=0;;t++)
	{
		if (title.besttime < titlescreens[t].timetobeat || \
			titlescreens[t].timetobeat == 0xffffffff)
		{
			break;
		}
	}
	
	title.sprite = titlescreens[t].sprite;
	music(titlescreens[t].songtrack);
	
	if (AnyProfileExists())
		title.cursel = 1;	// Load Game
	else
		title.cursel = 0;	// New Game
	
	return 0;
}

void title_tick()
{
	if (!title.in_multiload)
	{
		if (title.seldelay > 0)
		{
			ClearScreen(BLACK);
			
			title.seldelay--;
			if (!title.seldelay)
				selectoption(title.selchoice);
			
			return;
		}
		
		handle_input();
		draw_title();
	}
	else
	{
		ClearScreen(BLACK);
		
		if (!textbox.SaveSelect.IsVisible())
		{	// selection was made, and settings.last_save_slot is now set appropriately
			
			sound(SND_MENU_SELECT);
			
			textbox.SetVisible(false);
			title.selchoice = 1;
			title.seldelay = SELECT_LOAD_DELAY;
			title.in_multiload = false;
		}
		else
		{
			textbox.Draw();
		}
	}
}


/*
void c------------------------------() {}
*/

static void selectoption(int index)
{
	switch(index)
	{
		case 0:		// New
		{
			music(0);
			
			game.switchstage.mapno = NEW_GAME_FROM_MENU;
			game.setmode(GM_NORMAL);
		}
		break;
		
		case 1:		// Load
		{
			music(0);
			
			game.switchstage.mapno = LOAD_GAME_FROM_MENU;
			game.setmode(GM_NORMAL);
		}
		break;
		
		case 2:		// Load Menu (multisave)
		{
			textbox.SetVisible(true);
			textbox.SaveSelect.SetVisible(true, SS_LOADING);
			title.in_multiload = true;
		}
		break;
	}
}


static void handle_input()
{
    bool button_pressed = false;
#ifdef CONFIG_USE_TAPS
    // tap controls
    {
        int cx = (Graphics::SCREEN_WIDTH / 2) - (sprites[SPR_MENU].w / 2) - 8;
        int cy = (Graphics::SCREEN_HEIGHT / 2) - 8;
        for(int i=0;i<sprites[SPR_MENU].nframes;i++)
        {
            RectI r = Sprites::get_sprite_rect(cx, cy, SPR_MENU, i);
            if (VJoy::ModeAware::wasTap(r))
            {
                if (title.cursel == i)
                {
                    button_pressed = true;
                    
                }
                else
                {
                    sound(SND_MENU_MOVE);
                    title.cursel = i;
                }
                
                break;
            }

            cy += (sprites[SPR_MENU].h + 18);
        }
    }
#endif
    
    // pad control
    {
        if (justpushed(DOWNKEY))
        {
            sound(SND_MENU_MOVE);
            if (++title.cursel >= sprites[SPR_MENU].nframes)
                title.cursel = 0;
        }
        else if (justpushed(UPKEY))
        {
            sound(SND_MENU_MOVE);
            if (--title.cursel < 0)
                title.cursel = sprites[SPR_MENU].nframes - 1;
        }
        
        button_pressed = button_pressed || buttonjustpushed();
	}
    
    
	if (button_pressed)
	{
		sound(SND_MENU_SELECT);
		int choice = title.cursel;
		
		// handle case where user selects Load but there is no savefile,
		// or the last_save_file is deleted.
		if (title.cursel == 1)
		{
			if (!ProfileExists(settings->last_save_slot))
			{
				bool foundslot = false;
				for(int i=0;i<MAX_SAVE_SLOTS;i++)
				{
					if (ProfileExists(i))
					{
						stat("Last save file %d missing. Defaulting to %d instead.", settings->last_save_slot, i);
						settings->last_save_slot = i;
						foundslot = true;
					}
				}
				
				// there are no save files. Start a new game instead.
				if (!foundslot)
				{
					stat("No save files found. Starting new game instead.");
					choice = 0;
				}
			}
		}
		
		if (choice == 1 && settings->multisave)
		{
			title.selchoice = 2;
			title.seldelay = SELECT_MENU_DELAY;
		}
		else
		{
			title.selchoice = choice;
			title.seldelay = SELECT_DELAY;
			music(0);
		}
	}
	
	run_konami_code();
}

static void draw_title()
{
	// background is dk grey, not pure black
	ClearScreen(0x20, 0x20, 0x20);
	
	// top logo
	int tx = (Graphics::SCREEN_WIDTH / 2) - (sprites[SPR_TITLE].w / 2) - 2;
	draw_sprite(tx, 40, SPR_TITLE);
	
	// draw menu
	int cx = (Graphics::SCREEN_WIDTH / 2) - (sprites[SPR_MENU].w / 2) - 8;
	int cy = (Graphics::SCREEN_HEIGHT / 2) - 8;
	for(int i=0;i<sprites[SPR_MENU].nframes;i++)
	{
		draw_sprite(cx, cy, SPR_MENU, i);
		if (i == title.cursel)
		{
			draw_sprite(cx - 16, cy - 1, title.sprite, title.selframe);
		}
		
        //RectI r = Sprites::get_sprite_rect(cx, cy, SPR_MENU, i);
        //Graphics::DrawRect(r.x, r.y, r.x + r.w, r.y + r.h, 255,255,255);
        
		cy += (sprites[SPR_MENU].h + 18);
	}
	
	// animate character
	if (++title.seltimer > 8)
	{
		title.seltimer = 0;
		if (++title.selframe >= sprites[title.sprite].nframes)
			title.selframe = 0;
	}
	
	// accreditation
	cx = (Graphics::SCREEN_WIDTH / 2) - (sprites[SPR_PIXEL_FOREVER].w / 2);
	int acc_y = Graphics::SCREEN_HEIGHT - 48;
	draw_sprite(cx, acc_y, SPR_PIXEL_FOREVER);
	
	// version
	static const char *VERSION = "NXEngine v. 1.0.0.4";
	static const int SPACING = 5;
	int wd = GetFontWidth(VERSION, SPACING);
	cx = (Graphics::SCREEN_WIDTH / 2) - (wd / 2);
	font_draw(cx, acc_y + sprites[SPR_PIXEL_FOREVER].h + 4, VERSION, SPACING);
	
	// draw Nikumaru display
	if (title.besttime != 0xffffffff)
		niku_draw(title.besttime, true);
    
    // options
    {
        const char *str = "F3:Options";
        cx = (Graphics::SCREEN_WIDTH / 2) - (GetFontWidth(str, 0) / 2) - 4;
        cy = (Graphics::SCREEN_HEIGHT - 8) - GetFontHeight();
        int f3wd = font_draw(cx, cy, "F3", 0);
        font_draw(cx + f3wd, cy, ":Options", 0, &bluefont);
        
#ifdef CONFIG_USE_TAPS
        RectI r = RectI(cx, cy, GetFontWidth(str, 0), GetFontHeight());
        debug_absbox(r.x, r.y, r.x + r.w, r.y + r.h, 255, 255, 255);
        if (VJoy::ModeAware::wasTap(r))
        {
            game.pause(GP_OPTIONS);
        }
#endif
        
    }
}



static int kc_table[] = { UPKEY, UPKEY, DOWNKEY, DOWNKEY,
						  LEFTKEY, RIGHTKEY, LEFTKEY, RIGHTKEY, -1 };

void run_konami_code()
{
	if (justpushed(UPKEY) || justpushed(DOWNKEY) || \
		justpushed(LEFTKEY) || justpushed(RIGHTKEY))
	{
		if (justpushed(kc_table[title.kc_pos]))
		{
			title.kc_pos++;
			if (kc_table[title.kc_pos] == -1)
			{
				sound(SND_MENU_SELECT);
				title.kc_pos = 0;
			}
		}
		else
		{
			title.kc_pos = 0;
		}
	}
}


