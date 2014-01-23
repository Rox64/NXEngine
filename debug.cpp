
#include "nx.h"
#include <cassert>
#include <stdarg.h>
#include <string.h>

#include "debug.fdh"

#define MAX_DEBUG_MARKS		80
static struct
{
	int x, y, x2, y2;
	char type;
	uchar r, g, b;
} debugmarks[MAX_DEBUG_MARKS];

static int ndebugmarks = 0;
static StringList DebugList;


void DrawDebug(void)
{
	if (settings->enable_debug_keys)
	{
		// handle debug keys
		if (justpushed(DEBUG_GOD_KEY))
		{
			game.debug.god ^= 1;
			sound(SND_MENU_SELECT);
		}
		
		if (justpushed(DEBUG_SAVE_KEY))
		{
			game_save(settings->last_save_slot);
			sound(SND_SWITCH_WEAPON);
			console.Print("Game saved.");
		}
		
		if (justpushed(F6KEY))
		{
			game.debug.DrawBoundingBoxes ^= 1;
			sound(SND_COMPUTER_BEEP);
		}
		
		if (justpushed(F9KEY))
		{
			AddXP(1);
		}
		
		if (inputs[DEBUG_FLY_KEY])
		{
			player->yinertia = -0x880;
			if (!player->hurt_time) player->hurt_time = 20;		// make invincible
		}
	}
	
	/*if (game.debug.debugmode)
	{
		//debug("%d fps", game.debug.fps);
		
		if (game.debug.god)
		{
			//debug("<GOD MODE>");
			player->weapons[player->curWeapon].level = 2;
			player->weapons[player->curWeapon].xp = player->weapons[player->curWeapon].max_xp[2];
			player->weapons[player->curWeapon].ammo = player->weapons[player->curWeapon].maxammo;
			player->hp = player->maxHealth;
		}
		
		debug("%d,%d", (player->x>>CSF)/TILE_W, (player->y>>CSF)/TILE_H);
		debug("[%c%c%c%c]", player->blockl?'l':' ', player->blockr?'r':' ', player->blocku?'u':' ', player->blockd?'d':' ');
		//debug("%d", player->xinertia);
		//debug("%d", player->yinertia);*/
		/*
		debug("Have Puppy: %d", game.flags[274]);
		debug("Kakeru: %d", game.flags[275]);
		debug("Runner Gone: %d", game.flags[276]);
		debug("No Shinobu: %d", game.flags[277]);
		debug("Door Open: %d", game.flags[278]);
		debug("Mick: %d", game.flags[279]);
		debug("Gave 1st: %d", game.flags[590]);
		debug("Gave 2nd: %d", game.flags[591]);
		debug("Gave 3rd: %d", game.flags[592]);
		debug("Gave 4th: %d", game.flags[593]);
		debug("Gave 5th: %d", game.flags[594]);
		debug("-");
		{
			int i;
			for(i=0;i<player->ninventory;i++)
				debug("%d", player->inventory[i]);
		}
		*/
	//}
	
	debug_draw();
	DrawDebugMarks();
}


void DrawBoundingBoxes()
{
	Object *o;
	FOREACH_OBJECT(o)
	{
		if (o->onscreen || o == player)
		{
			uint32_t color;
			
			if (o == player)
			{
				color = 0xffff00;
			}
			else if (o->flags & FLAG_INVULNERABLE)
			{
				color = 0xffffff;
			}
			else if (o->flags & FLAG_SHOOTABLE)
			{
				color = 0x00ff00;
			}
			else if (o->flags & FLAG_SOLID_MUSHY)
			{
				color = 0xff0080;
			}
			else
			{
				color = 0xff0000;
			}
			
			AddDebugMark(o->Left(), o->Top(), o->Right(), o->Bottom(),
						DM_BOX, color>>16, (color>>8)&0xff, color&0xff);
		}
	}
}

void DrawAttrPoints()
{
	Object *o;
	FOREACH_OBJECT(o)
	{
		draw_pointlist(o, &sprites[o->sprite].block_l);
		draw_pointlist(o, &sprites[o->sprite].block_r);
		draw_pointlist(o, &sprites[o->sprite].block_u);
		draw_pointlist(o, &sprites[o->sprite].block_d);
	}
}

void draw_pointlist(Object *o, SIFPointList *points)
{
	int xoff = (o->x >> CSF);
	int yoff = (o->y >> CSF);
	
	for(int i=0;i<points->count;i++)
	{
		DebugPixel((xoff + points->point[i].x) << CSF, \
			(yoff + points->point[i].y) << CSF, 255, 0, 255);
	}
}

/*
void c------------------------------() {}
*/

// debug text display debug() useful for reporting game vars etc
void debug(const char *fmt, ...)
{
char buffer[128];
va_list ar;

	va_start(ar, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ar);
	va_end(ar);
	
	DebugList.AddString(buffer);
}

void debug_draw(void)
{
	for(int i=0;;i++)
	{
		const char *text = DebugList.StringAt(i);
		if (!text) break;
		
		int x = (Graphics::SCREEN_WIDTH - 8) - GetFontWidth(text, 0, true);
		int y = 14 + (i * (GetFontHeight() + 1));
		font_draw_shaded(x, y, text, 0, &greenfont);
	}
}

void debug_clear()
{
	DebugList.MakeEmpty();
}

/*
void c------------------------------() {}
*/

extern const char *object_names[];	// from autogen'd objnames.cpp

// given an object type returns the name of the object e.g. "OBJ_TOROKO"
const char *DescribeObjectType(int type)
{
	if (type >= 0 && type < OBJ_LAST && object_names[type])
		return stprintf("OBJ_%s(%d)", object_names[type], type);
	
	return stprintf("[Type %d]", type);
}

// tries to convert a string, such as OBJ_TOROKO, into it's numeric type,
// with a small bit of english-language intelligence.
int ObjectNameToType(const char *name_in)
{
	// if all characters are numeric they're specifying by number
	// so simply return the atoi
	for(int i=0;;i++)
	{
		if (name_in[i] == 0)
		{
			return atoi(name_in);
		}
		
		if (!isdigit(name_in[i])) break;
	}
	
	char *name = strdup(name_in);	// make string writeable
	
	// some string preprocessing
	for(int i=0;name[i];i++)
	{
		if (name[i] == ' ')
		{
			name[i] = '_';
		}
		else
		{
			name[i] = toupper(name[i]);
		}
	}
	
	// remove the "OBJ_" suffix if it's present
	const char *searchstring = name;
	if (strbegin(name, "OBJ_"))
		searchstring += 4;
	
	// search for it in the object_names table
	for(int i=0;i<OBJ_LAST;i++)
	{
		if (object_names[i] && !strcmp(object_names[i], searchstring))
		{
			free(name);
			return i;
		}
	}
	
	stat("ObjectNameToType: couldn't find object 'OBJ_%s'", searchstring);
	free(name);
	return -1;
}

const char *DescribeDir(int dir)
{
	switch(dir)
	{
		case LEFT: return "LEFT";
		case RIGHT: return "RIGHT";
		case UP: return "UP";
		case DOWN: return "DOWN";
		case CENTER: return "CENTER";
		default: return stprintf("[Invalid Direction %d]", dir);
	}
}

/*
void c------------------------------() {}
*/

const char *strhex(int value)
{
	if (value < 0)
		return stprintf("-0x%x", -value);
	else
		return stprintf("0x%x", value);
}

const char *strhex(void const* value, size_t size)
{
    char *str = GetStaticStr();
	if (size > 510)
        size = 510;
    size_t i, j;
    for (i = j = 0; j < size; i+=2, j++)
    {
        sprintf(&str[i], "%02x", (unsigned int)(*(((unsigned char const*)value) + j)));
    }
    return str;
}

/*
void c------------------------------() {}
*/

void DrawDebugMarks(void)
{
int i;
int x, y, x2, y2;
uchar r, g, b;

	for(i=0;i<ndebugmarks;i++)
	{
		x = (debugmarks[i].x >> CSF) - (map.displayed_xscroll >> CSF);
		y = (debugmarks[i].y >> CSF) - (map.displayed_yscroll >> CSF);
		x2 = (debugmarks[i].x2 >> CSF) - (map.displayed_xscroll >> CSF);
		y2 = (debugmarks[i].y2 >> CSF) - (map.displayed_yscroll >> CSF);
		r = debugmarks[i].r;
		g = debugmarks[i].g;
		b = debugmarks[i].b;
		
		switch(debugmarks[i].type)
		{
			case DM_PIXEL:
				DrawPixel(x, y, r, g, b);
			break;
			
			case DM_CROSSHAIR:
				DrawPixel(x, y, r, g, b);
				DrawPixel(x+1, y, r, g, b);
				DrawPixel(x-1, y, r, g, b);
				DrawPixel(x, y+1, r, g, b);
				DrawPixel(x, y-1, r, g, b);
			break;
			
			case DM_XLINE:
				FillRect(x, 0, x, Graphics::SCREEN_HEIGHT, r, g, b);
			break;
			
			case DM_YLINE:
				FillRect(0, y, Graphics::SCREEN_WIDTH, y, r, g, b);
			break;
			
			case DM_BOX:
				DrawRect(x, y, x2, y2, r, g, b);
			break;
                
            case DM_ABS_BOX:
                DrawRect(debugmarks[i].x, debugmarks[i].y, debugmarks[i].x2, debugmarks[i].y2, r, g, b);
                break;
		}
	}
	
	ndebugmarks = 0;
}

void AddDebugMark(int x, int y, int x2, int y2, char type, uchar r, uchar g, uchar b)
{
	if (ndebugmarks >= MAX_DEBUG_MARKS)
		return;
	
	debugmarks[ndebugmarks].x = x;
	debugmarks[ndebugmarks].y = y;
	debugmarks[ndebugmarks].x2 = x2;
	debugmarks[ndebugmarks].y2 = y2;
	debugmarks[ndebugmarks].r = r;
	debugmarks[ndebugmarks].g = g;
	debugmarks[ndebugmarks].b = b;
	debugmarks[ndebugmarks].type = type;
	ndebugmarks++;
}

// draw a pixel of the specified color at [x,y] in object coordinates
void DebugPixel(int x, int y, uchar r, uchar g, uchar b)
{
	AddDebugMark(x, y, 0, 0, DM_PIXEL, r, g, b);
}

void DebugCrosshair(int x, int y, uchar r, uchar g, uchar b)
{
	AddDebugMark(x, y, 0, 0, DM_CROSSHAIR, r, g, b);
}

void crosshair(int x, int y)
{
	debugVline(x, 255, 0, 0);
	debugHline(y, 0, 255, 0);
}

void DebugPixelNonCSF(int x, int y, uchar r, uchar g, uchar b) { DebugPixel(x<<CSF,y<<CSF,r,g,b); }
void DebugCrosshairNonCSF(int x, int y, uchar r, uchar g, uchar b) { DebugCrosshair(x<<CSF,y<<CSF,r,g,b); }

void debugVline(int x, uchar r, uchar g, uchar b)
{
	AddDebugMark(x, 0, 0, 0, DM_XLINE, r, g, b);
}

void debugHline(int y, uchar r, uchar g, uchar b)
{
	AddDebugMark(0, y, 0, 0, DM_YLINE, r, g, b);
}

void debugbox(int x1, int y1, int x2, int y2, uchar r, uchar g, uchar b)
{
	AddDebugMark(x1, y1, x2, y2, DM_BOX, r, g, b);
}

void debug_absbox(int x1, int y1, int x2, int y2, uchar r, uchar g, uchar b)
{
	AddDebugMark(x1, y1, x2, y2, DM_ABS_BOX, r, g, b);
}

void debugtile(int x, int y, uchar r, uchar g, uchar b)
{
int x1, y1, x2, y2;

	x *= (TILE_W << CSF);
	y *= (TILE_H << CSF);
	
	x1 = x; y1 = y;
	x2 = x1 + (TILE_W << CSF);
	y2 = y1 + (TILE_H << CSF);
	AddDebugMark(x1, y1, x2, y2, DM_BOX, r, g, b);
}


Uint32 debug_timer_b = 0;
Uint32 debug_timer_l = 0;
void debug_timer_begin() 
{
	Uint32 now = SDL_GetTicks();
	debug("last frame %10d", now - debug_timer_b);
	debug_timer_l = debug_timer_b = now;
}

void debug_timer_point(char const* msg)
{
	Uint32 now = SDL_GetTicks();

	debug("%s %10d %10d", msg, now - debug_timer_l, now - debug_timer_b);

	debug_timer_l = now;
}
