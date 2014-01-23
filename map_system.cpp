
// the Map System
#include "nx.h"
#include "map_system.h"
#include "map_system.fdh"

#include "vjoy.h"

#define MS_EXPANDING		0
#define MS_DISPLAYED		1
#define MS_CONTRACTING		2

// # of frames in the expand/contract animation
#define EXPAND_LENGTH		8

#define BANNER_TOP			7
#define BANNER_BTM			23

static struct
{
	NXSurface *sfc;			// surface for the map image
	
	int x, y;				// the position of the upper-left corner of the image
	int w, h;				// size of image
	
	int expandframe;		// for expand/contract effect
	int current_row;		// scan down effect
	
	int px, py;				// the position of the you-are-here dot
	int timer;				// for the flashing
	
	int state;				// expanding, displayed, contracting
	int return_gm;			// game mode to return to
	
	const char *bannertext;
	int textx, texty;
	
	bool lastbuttondown;
} ms;


bool ms_init(int return_to_mode)
{
	memset(&ms, 0, sizeof(ms));
	ms.return_gm = return_to_mode;
	ms.lastbuttondown = true;
	ms.w = map.xsize;
	ms.h = map.ysize;
	
	// create the surface
	ms.sfc = new NXSurface(ms.w, ms.h);
	ms.sfc->FillRect(0, 0, ms.w, ms.h, DK_BLUE);
	
	ms.x = (Graphics::SCREEN_WIDTH / 2) - (ms.w / 2);
	ms.y = (Graphics::SCREEN_HEIGHT / 2) - (ms.h / 2);
	
	// where will we put the dot?
	ms.px = ms.x + ((player->x >> CSF) / TILE_W);
	ms.py = ms.y + ((player->y >> CSF) / TILE_H);
	
	ms.bannertext = stages[game.curmap].stagename;
	ms.textx = (Graphics::SCREEN_WIDTH / 2) - (GetFontWidth(ms.bannertext, 0) / 2);
	ms.texty = BANNER_TOP+3;
	
	return 0;
}

void ms_close(void)
{
	memset(inputs, 0, sizeof(inputs));
	delete ms.sfc;
}


/*
void c------------------------------() {}
*/

void ms_tick(void)
{
	DrawScene();
	draw_banner();
	
	if (ms.state == MS_EXPANDING)
	{
		ms.expandframe++;
		
		if (ms.expandframe > EXPAND_LENGTH)
			ms.state = MS_DISPLAYED;
		else
			draw_expand();
	}
	
	if (ms.state == MS_DISPLAYED)
	{
		// scan down effect
		if (ms.current_row < map.ysize)
		{
			draw_row(ms.current_row++);
			
			if (ms.current_row < map.ysize)
				draw_row(ms.current_row++);
		}
		
		// draw map
		DrawRect(ms.x - 1, ms.y - 1, ms.x + ms.w, ms.y + ms.h, DK_BLUE);
		DrawSurface(ms.sfc, ms.x, ms.y);
		
		// you-are-here dot
		if (++ms.timer & 8)
			draw_sprite(ms.px, ms.py, SPR_MAP_PIXELS, 4);
		
        if (VJoy::ModeAware::wasTap())
            ms.state = MS_CONTRACTING;
        
		// dismissal
		if (ms.lastbuttondown)
		{
			if (!buttondown())
				ms.lastbuttondown = false;
		}
		else if (buttondown())
		{
			ms.state = MS_CONTRACTING;
		}
	}
	else if (ms.state == MS_CONTRACTING)
	{
		ms.expandframe--;
		
		if (ms.expandframe <= 0)
		{
			int param = (ms.return_gm == GM_INVENTORY) ? 1 : 0;
			game.setmode(ms.return_gm, param);
		}
		else
		{
			draw_expand();
		}
	}
}



// expand/contract effect
static void draw_expand(void)
{
int x1, y1, x2, y2;

	int wd = (map.xsize * ms.expandframe) / EXPAND_LENGTH;
	int ht = (map.ysize * ms.expandframe) / EXPAND_LENGTH;
	
	x1 = (Graphics::SCREEN_WIDTH / 2)  - (wd / 2);
	y1 = (Graphics::SCREEN_HEIGHT / 2) - (ht / 2);
	x2 = (Graphics::SCREEN_WIDTH / 2)  + (wd / 2);
	y2 = (Graphics::SCREEN_HEIGHT / 2) + (ht / 2);
	
	FillRect(x1, y1, x2, y2, DK_BLUE);
}


static void draw_banner(void)
{
	FillRect(0, BANNER_TOP, Graphics::SCREEN_WIDTH, BANNER_BTM, NXColor(0, 0, 0));
	font_draw(ms.textx, ms.texty, ms.bannertext, 0);
}


/*
void c------------------------------() {}
*/


// draw the specified row of map onto the spritesheet
// the colors for the map system are not actually plotted as pixels,
// but exist as 1x1 sprites on the TextBox spritesheet.
static void draw_row(int y)
{
int x;

	Graphics::SetDrawTarget(ms.sfc);
    
    Graphics::DrawBatchBegin(map.xsize);
	Sprites::draw_in_batch(true);
    
    
	for(x=0;x<map.xsize;x++)
	{
		int tc = tilecode[map.tiles[x][y]];
		draw_sprite(x, y, SPR_MAP_PIXELS, get_color(tc));
        
	}

    Sprites::draw_in_batch(false);
    Graphics::DrawBatchEnd();
	
	Graphics::SetDrawTarget(screen);
}


static int get_color(int tilecode)
{
	switch(tilecode)
	{
		case 0:
			return 0;
		
		case 0x01:
		case 0x02:
		case 0x40:
		case 0x44:
		case 0x51:
		case 0x52:
		case 0x55:
		case 0x56:
		case 0x60:
		case 0x71:
		case 0x72:
		case 0x75:
		case 0x76:
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
			return 1;
		
		case 0x43:
		case 0x50:
		case 0x53:
		case 0x54:
		case 0x57:
		case 0x63:
		case 0x70:
		case 0x73:
		case 0x74:
		case 0x77:
			return 2;
		
		default:
			return 3;
	}
}


