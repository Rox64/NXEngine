
#include "nx.h"
#include "map.h"
#include "map.fdh"

stMap map;

MapRecord stages[MAX_STAGES];
int num_stages;

#define MAX_BACKDROPS			32
NXSurface *backdrop[MAX_BACKDROPS];

// for FindObject--finding NPC's by ID2
Object *ID2Lookup[65536];

unsigned char tilecode[MAX_TILES];			// tile codes for every tile in current tileset
unsigned int tileattr[MAX_TILES];			// tile attribute bits for every tile in current tileset
unsigned int tilekey[MAX_TILES];			// mapping from tile codes -> tile attributes


// load stage "stage_no", this entails loading the map (pxm), enemies (pxe), tileset (pbm),
// tile attributes (pxa), and script (tsc).
bool load_stage(int stage_no)
{
char stage[MAXPATHLEN];
char fname[MAXPATHLEN];

	stat(" >> Entering stage %d: '%s'.", stage_no, stages[stage_no].stagename);
	game.curmap = stage_no;		// do it now so onspawn events will have it
	
	if (use_palette)
	{
		palette_reset();
		Sprites::FlushSheets();
		map_flush_graphics();
	}

	if (Tileset::Load(stages[stage_no].tileset))
		return 1;
	
	// get the base name of the stage without extension
	const char *mapname = stages[stage_no].filename;
	if (!strcmp(mapname, "lounge")) mapname = "Lounge";
	sprintf(stage, "%s/%s", stage_dir, mapname);
	
	sprintf(fname, "%s.pxm", stage);
	if (load_map(fname)) return 1;
	
	sprintf(fname, "%s/%s.pxa", stage_dir, tileset_names[stages[stage_no].tileset]);
	if (load_tileattr(fname)) return 1;
	
	sprintf(fname, "%s.pxe", stage);
	if (load_entities(fname)) return 1;
	
	sprintf(fname, "%s.tsc", stage);
	if (tsc_load(fname, SP_MAP) == -1) return 1;
	
	map_set_backdrop(stages[stage_no].bg_no);
	map.scrolltype = stages[stage_no].scroll_type;
	map.motionpos = 0;
	
	return 0;
}

/*
void c------------------------------() {}
*/

// load a PXM map
bool load_map(const char *fname)
{
FILE *fp;
int x, y;

	fp = fileopenRO(fname);
	if (!fp)
	{
		staterr("load_map: no such file: '%s'", fname);
		return 1;
	}
	
	if (!fverifystring(fp, "PXM"))
	{
		staterr("load_map: invalid map format: '%s'", fname);
		return 1;
	}
	
	memset(&map, 0, sizeof(map));
	
	fgetc(fp);
	map.xsize = fgeti(fp);
	map.ysize = fgeti(fp);
	
	if (map.xsize > MAP_MAXSIZEX || map.ysize > MAP_MAXSIZEY)
	{
		staterr("load_map: map is too large -- size %dx%d but max is %dx%d", map.xsize, map.ysize, MAP_MAXSIZEX, MAP_MAXSIZEY);
		fclose(fp);
		return 1;
	}
	else
	{
		stat("load_map: level size %dx%d", map.xsize, map.ysize);
	}
	
	for(y=0;y<map.ysize;y++)
	for(x=0;x<map.xsize;x++)
	{
		map.tiles[x][y] = fgetc(fp);
	}
	
	fclose(fp);
	
	map.maxxscroll = (((map.xsize * TILE_W) - Graphics::SCREEN_WIDTH) - 8) << CSF;
	map.maxyscroll = (((map.ysize * TILE_H) - Graphics::SCREEN_HEIGHT) - 8) << CSF;
	
	stat("load_map: '%s' loaded OK! - %dx%d", fname, map.xsize, map.ysize);
	return 0;
}


// load a PXE (entity list for a map)
bool load_entities(const char *fname)
{
FILE *fp;
int i;
int nEntities;

	// gotta destroy all objects before creating new ones
	Objects::DestroyAll(false);
	FloatText::ResetAll();
	
	stat("load_entities: reading in %s", fname);
	// now we can load in the new objects
	fp = fileopenRO(fname);
	if (!fp)
	{
		staterr("load_entities: no such file: '%s'", fname);
		return 1;
	}
	
	if (!fverifystring(fp, "PXE"))
	{
		staterr("load_entities: not a PXE: '%s'", fname);
		return 1;
	}
	
	fgetc(fp);
	nEntities = fgetl(fp);
	
	for(i=0;i<nEntities;i++)
	{
		int x = fgeti(fp);
		int y = fgeti(fp);
		int id1 = fgeti(fp);
		int id2 = fgeti(fp);
		int type = fgeti(fp);
		int flags = fgeti(fp);
		
		int dir = (flags & FLAG_FACES_RIGHT) ? RIGHT : LEFT;
		
		//lprintf(" %d:   [%d, %d]\t id1=%d\t id2=%d   Type %d   flags %04x\n", i, x, y, id1, id2, type, flags);
		
		// most maps have apparently garbage entities--invisible do-nothing objects??
		// i dunno but no point in spawning those...
		if (type || id1 || id2 || flags)
		{
			bool addobject = false;
			
			// check if object is dependent on a flag being set/not set
			if (flags & FLAG_APPEAR_ON_FLAGID)
			{
				if (game.flags[id1])
				{
					addobject = true;
					stat(" -- Appearing object %02d (%s) because flag %d is set", id2, DescribeObjectType(type), id1);
				}
			}
			else if (flags & FLAG_DISAPPEAR_ON_FLAGID)
			{
				if (!game.flags[id1])
				{
					addobject = true;
				}
				else
				{
					stat(" -- Disappearing object %02d (%s) because flag %d is set", id2, DescribeObjectType(type), id1);
				}
			}
			else
			{
				addobject = true;
			}
			
			if (addobject)
			{
				// hack for chests (can we do this elsewhere?)
				if (type == OBJ_CHEST_OPEN) y++;
				
				Object *o = CreateObject((x * TILE_W) << CSF, \
										 (y * TILE_H) << CSF, type,
										 0, 0, dir, NULL, CF_NO_SPAWN_EVENT);
				
				o->id1 = id1;
				o->id2 = id2;
				o->flags |= flags;
				
				ID2Lookup[o->id2] = o;
				
				// now that it's all set up, execute OnSpawn,
				// since we didn't do it in CreateObject.
				o->OnSpawn();
			}
		}
	}
	
	//stat("load_entities: loaded %d objects", nEntities);
	fclose(fp);
	return 0;
}

/*const int ta[] =
{ 0, TA_SOLID, TA_SOLID, TA_SOLID, TA_SOLID,
  TA_SLOPE_BACK1|TA_FOREGROUND, TA_SLOPE_BACK2|TA_FOREGROUND, TA_SLOPE_FWD1|TA_FOREGROUND, TA_SLOPE_FWD2|TA_FOREGROUND,
  TA_FOREGROUND, 0,0,0, TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND, 0, TA_SOLID, TA_SOLID, TA_FOREGROUND, TA_FOREGROUND,
  TA_SOLID,TA_SOLID,TA_SOLID,TA_SOLID,TA_FOREGROUND,0,0,0,TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,
  0,TA_SOLID,TA_FOREGROUND,TA_DESTROYABLE|TA_SOLID,TA_SOLID,TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,TA_SLOPE_CEIL_BACK1|TA_FOREGROUND,TA_SOLID,TA_SOLID,TA_SLOPE_CEIL_FWD2|TA_FOREGROUND,TA_SLOPE_FWD1|TA_FOREGROUND,TA_SLOPE_FWD2|TA_FOREGROUND,
  TA_FOREGROUND,TA_FOREGROUND,TA_SLOPE_CEIL_FWD1|TA_FOREGROUND,TA_SLOPE_CEIL_FWD2|TA_FOREGROUND,TA_SLOPE_CEIL_BACK1|TA_FOREGROUND,TA_SLOPE_CEIL_BACK2|TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,TA_FOREGROUND,0,0,TA_SOLID,TA_SOLID,TA_FOREGROUND,TA_SOLID,TA_SOLID,
  TA_SOLID,TA_SOLID,TA_FOREGROUND|TA_SLOPE_BACK1,TA_SLOPE_BACK2|TA_FOREGROUND,TA_SLOPE_FWD1|TA_FOREGROUND,TA_SLOPE_FWD2|TA_FOREGROUND,TA_SPIKES,TA_SPIKES,TA_SPIKES,TA_SPIKES,0,TA_SOLID,TA_SOLID,0,TA_SOLID,TA_SOLID,
  0,TA_SOLID,0,TA_SOLID,TA_SOLID,0,0,0,0,0,0,TA_SOLID,TA_SOLID,TA_SOLID,TA_SOLID,TA_SOLID,
  TA_SOLID,TA_FOREGROUND,TA_FOREGROUND,0,0,0,0,0,0,0,0,0,TA_SOLID,TA_SOLID,TA_SOLID,TA_SOLID
};
	memset(tileattr, 0, sizeof(tileattr));
	memcpy(&tileattr, &ta, sizeof(ta));
*/

// loads a pxa (tileattr) file
bool load_tileattr(const char *fname)
{
FILE *fp;
int i;
unsigned char tc;

	map.nmotiontiles = 0;
	
	stat("load_pxa: reading in %s", fname);
	fp = fileopenRO(fname);
	if (!fp)
	{
		staterr("load_pxa: no such file: '%s'", fname);
		return 1;
	}
	
	for(i=0;i<256;i++)
	{
		tc = fgetc(fp);
		tilecode[i] = tc;
		tileattr[i] = tilekey[tc];
		//stat("Tile %02x   TC %02x    Attr %08x   tilekey[%02x] = %08x", i, tc, tileattr[i], tc, tilekey[tc]);
		
		if (tc == 0x43)	// destroyable block - have to replace graphics
		{
			CopySpriteToTile(SPR_DESTROYABLE, i, 0, 0);
		}
		
		// add water currents to animation list
		if (tileattr[i] & TA_CURRENT)
		{
			map.motiontiles[map.nmotiontiles].tileno = i;
			map.motiontiles[map.nmotiontiles].dir = CVTDir(tc & 3);
			map.motiontiles[map.nmotiontiles].sprite = SPR_WATER_CURRENT;
			
			map.nmotiontiles++;
			stat("Added tile %02x to animation list, tc=%02x", i, tc);
		}
	}
	
	fclose(fp);
	return 0;
}

bool load_stages(void)
{
FILE *fp;

	fp = fileopenRO("stage.dat");
	if (!fp)
	{
		staterr("%s(%d): failed to open stage.dat", __FILE__, __LINE__);
		num_stages = 0;
		return 1;
	}
	
	num_stages = fgetc(fp);
	for(int i=0;i<num_stages;i++)
		fread(&stages[i], sizeof(MapRecord), 1, fp);
	
	return 0;
}


bool initmapfirsttime(void)
{
FILE *fp;
int i;

	stat("initmapfirsttime: loading tilekey.dat.");
	if (!(fp = fileopenRO("tilekey.dat")))
	{
		staterr("tilekey.dat is missing!");
		return 1;
	}
	
	for(i=0;i<256;i++)
		tilekey[i] = fgetl(fp);
	
	fclose(fp);
	return load_stages();
}

void initmap(void)
{
	map_focus(NULL);
	map.parscroll_x = map.parscroll_y = 0;
}

/*
void c------------------------------() {}
*/

// backdrop_no 	- backdrop # to switch to
void map_set_backdrop(int backdrop_no)
{
	if (!LoadBackdropIfNeeded(backdrop_no))
		map.backdrop = backdrop_no;
}


void map_draw_backdrop(void)
{
int x, y;

	if (!backdrop[map.backdrop])
	{
		LoadBackdropIfNeeded(map.backdrop);
		if (!backdrop[map.backdrop])
			return;
	}
	
	switch(map.scrolltype)
	{
		case BK_FIXED:
			map.parscroll_x = 0;
			map.parscroll_y = 0;
		break;
		
		case BK_FOLLOWFG:
			map.parscroll_x = (map.displayed_xscroll >> CSF);
			map.parscroll_y = (map.displayed_yscroll >> CSF);
		break;
		
		case BK_PARALLAX:
			map.parscroll_y = (map.displayed_yscroll >> CSF) / 2;
			map.parscroll_x = (map.displayed_xscroll >> CSF) / 2;
		break;
		
		case BK_FASTLEFT:		// Ironhead
			map.parscroll_x += 6;
			map.parscroll_y = 0;
		break;
		
		case BK_FASTLEFT_LAYERS:
		case BK_FASTLEFT_LAYERS_NOFALLLEFT:
		{
			DrawFastLeftLayered();
			return;
		}
		break;
		
		case BK_HIDE:
		case BK_HIDE2:
		case BK_HIDE3:
		{
			if (game.curmap == STAGE_KINGS)		// intro cutscene
				ClearScreen(BLACK);
			else
				ClearScreen(DK_BLUE);
		}
		return;
		
		default:
			map.parscroll_x = map.parscroll_y = 0;
			staterr("map_draw_backdrop: unhandled map scrolling type %d", map.scrolltype);
		break;
	}
	
	map.parscroll_x %= backdrop[map.backdrop]->Width();
	map.parscroll_y %= backdrop[map.backdrop]->Height();
	int w = backdrop[map.backdrop]->Width();
	int h = backdrop[map.backdrop]->Height();

	const int max_x = (Graphics::SCREEN_WIDTH+map.parscroll_x) / w + 1;	
	const int max_y = (Graphics::SCREEN_HEIGHT+map.parscroll_y) / h + 1;

	Graphics::DrawBatchBegin(max_x * max_y);

	for(y=0;y<Graphics::SCREEN_HEIGHT+map.parscroll_y; y+=h)
	{
		for(x=0;x<Graphics::SCREEN_WIDTH+map.parscroll_x; x+=w)
		{
			//DrawSurface(backdrop[map.backdrop], x - map.parscroll_x, y - map.parscroll_y);
			Graphics::DrawBatchAdd(backdrop[map.backdrop], x - map.parscroll_x, y - map.parscroll_y);
		}
	}

	Graphics::DrawBatchEnd();
}

// blit OSide's BK_FASTLEFT_LAYERS
static void DrawFastLeftLayered(void)
{
static const int layer_ys[] = { 80, 122, 145, 176, 240 };
static const int move_spd[] = { 0,    1,   2,   4,   8 };
const int nlayers = 5;
int y1, y2;
int i, x;

	const int W = backdrop[map.backdrop]->Width();
	
	if (--map.parscroll_x <= -(W*2))
		map.parscroll_x = 0;
	
    Graphics::DrawBatchBegin(0);
    
	y1 = x = 0;
	for(i=0;i<nlayers;i++)
	{
		y2 = layer_ys[i];
		
		if (i)	// not the static moon layer?
		{
			x = (map.parscroll_x * move_spd[i]) >> 1;
			x %= W;
		}
		
        Graphics::DrawBatchAddPatternAcross(backdrop[map.backdrop], x, y1, y1, (y2-y1)+1);
		y1 = (y2 + 1);
	}
	
	if (Graphics::SCREEN_HEIGHT > 240)
	{
		// It seems, that texture will be stretched even if gap between last
		// layer and screen bottom is bigger than 240-217 pixels. So, we don't
		// have to put loop here.
		// 217 is the first point without clouds border - clouds below have solid color.
		Graphics::DrawBatchAddPatternAcross(backdrop[map.backdrop], x, 240, 217, Graphics::SCREEN_HEIGHT - 240);
	}
    
    Graphics::DrawBatchEnd();
}


// loads a backdrop into memory, if it hasn't already been loaded
static bool LoadBackdropIfNeeded(int backdrop_no)
{
char fname[MAXPATHLEN];

	// load backdrop now if it hasn't already been loaded
	if (!backdrop[backdrop_no])
	{
		// use chromakey (transparency) on bkwater, all others don't
		bool use_chromakey = (backdrop_no == 8);
		
		sprintf(fname, "%s/%s.pbm", data_dir, backdrop_names[backdrop_no]);
		
		backdrop[backdrop_no] = NXSurface::FromFile(fname, use_chromakey);
		if (!backdrop[backdrop_no])
		{
			staterr("Failed to load backdrop '%s'", fname);
			return 1;
		}
	}
	
	return 0;
}

void map_flush_graphics()
{
int i;

	for(i=0;i<MAX_BACKDROPS;i++)
	{
		delete backdrop[i];
		backdrop[i] = NULL;
	}
	
	// re-copy star files
	for(i=0;i<256;i++)
	{
		if (tilecode[i] == 0x43)
		{
			CopySpriteToTile(SPR_DESTROYABLE, i, 0, 0);
		}
	}
}


/*
void c------------------------------() {}
*/

// draw rising/falling water from eg Almond etc
void map_drawwaterlevel(void)
{
// water_sfc: 16 tall at 0
// just under: 16 tall at 32
// main tile: 32 tall at 16 (yes, overlapping)
int water_x, water_y;

	if (!map.waterlevelobject)
		return;
	
	water_x = -(map.displayed_xscroll >> CSF);
	water_x %= Graphics::SCREEN_WIDTH;
	
	water_y = (map.waterlevelobject->y >> CSF) - (map.displayed_yscroll >> CSF);
    
    Graphics::DrawBatchBegin(0);
	
	// draw the surface and just under the surface
	Graphics::DrawBatchAddPatternAcross(backdrop[map.backdrop], water_x, water_y, 0, 16);
	water_y += 16;
	
	Graphics::DrawBatchAddPatternAcross(backdrop[map.backdrop], water_x, water_y, 32, 16);
	water_y += 16;
	
	// draw the rest of the pattern all the way down
	while(water_y < (Graphics::SCREEN_HEIGHT-1))
	{
        Graphics::DrawBatchAddPatternAcross(backdrop[map.backdrop], water_x, water_y, 16, 32);
		water_y += 32;
	}
    
    Graphics::DrawBatchEnd();
}


// draw the map.
// 	if foreground = TA_FOREGROUND, draws the foreground tile layer.
//  if foreground = 0, draws backdrop and background tiles.
void map_draw(uint8_t foreground)
{
int x, y;
int mapx, mapy;
int blit_x, blit_y, blit_x_start;
int scroll_x, scroll_y;

	const int max_x = (Graphics::SCREEN_WIDTH  / TILE_W) + MAP_DRAW_EXTRA_X;
	const int max_y = (Graphics::SCREEN_HEIGHT / TILE_H) + MAP_DRAW_EXTRA_Y;
	
	scroll_x = (map.displayed_xscroll >> CSF);
	scroll_y = (map.displayed_yscroll >> CSF);
	
	mapx = (scroll_x / TILE_W);
	mapy = (scroll_y / TILE_H);
	
	blit_y = -(scroll_y % TILE_H);
	blit_x_start = -(scroll_x % TILE_W);

	Tileset::draw_tilegrid_begin(max_x * max_y);
	
	// MAP_DRAW_EXTRA_Y etc is 1 if resolution is changed to
	// something not a multiple of TILE_H.
	for(y=0; y <= max_y; y++)
	{
		blit_x = blit_x_start;
		
		for(x=0; x <= max_x; x++)
		{
			int t = map.tiles[mapx+x][mapy+y];
			if ((tileattr[t] & TA_FOREGROUND) == foreground)
			{
				Tileset::draw_tilegrid_add(blit_x, blit_y, t);
			}
			
			blit_x += TILE_W;
		}
		
		blit_y += TILE_H;
	}

	Tileset::draw_tilegrid_end();
}


/*
void c------------------------------() {}
*/

// map scrolling code
void scroll_normal(void)
{
const int scroll_adj_rate = (0x2000 / map.scrollspeed);
	
	// how many pixels to let player stray from the center of the screen
	// before we start scrolling. high numbers let him reach closer to the edges,
	// low numbers keep him real close to the center.
	#define P_VARY_FROM_CENTER			(64 << CSF)
	
	if (player->dir == LEFT)
	{
		map.scrollcenter_x -= scroll_adj_rate;
		if (map.scrollcenter_x < -P_VARY_FROM_CENTER)
			map.scrollcenter_x = -P_VARY_FROM_CENTER;
	}
	else
	{
		map.scrollcenter_x += scroll_adj_rate;
		if (map.scrollcenter_x > P_VARY_FROM_CENTER)
			map.scrollcenter_x = P_VARY_FROM_CENTER;
	}
	
	// compute where the map "wants" to be
	map.target_x = (player->CenterX() + map.scrollcenter_x) - ((Graphics::SCREEN_WIDTH / 2) << CSF);
	
	// Y scrolling
	if (player->lookscroll == UP)
	{
		map.scrollcenter_y -= scroll_adj_rate;
		if (map.scrollcenter_y < -P_VARY_FROM_CENTER) map.scrollcenter_y = -P_VARY_FROM_CENTER;
	}
	else if (player->lookscroll == DOWN)
	{
		map.scrollcenter_y += scroll_adj_rate;
		if (map.scrollcenter_y > P_VARY_FROM_CENTER) map.scrollcenter_y = P_VARY_FROM_CENTER;
	}
	else
	{
		if (map.scrollcenter_y <= -scroll_adj_rate)
		{
			map.scrollcenter_y += scroll_adj_rate;
		}
		else if (map.scrollcenter_y >= scroll_adj_rate)
		{
			map.scrollcenter_y -= scroll_adj_rate;
		}
	}
	
	map.target_y = (player->CenterY() + map.scrollcenter_y) - ((Graphics::SCREEN_HEIGHT / 2) << CSF);
}

void map_scroll_do(void)
{
	bool doing_normal_scroll = false;
	
	if (!map.scroll_locked)
	{
		if (map.focus.has_target)
		{	// FON command
			// this check makes it so if we <FON on an object which
			// gets destroyed, the scroll stays locked at the last known
			// position of the object.
			if (map.focus.target)
			{
				Object *t = map.focus.target;
				
				// Generally we want to focus on the center of the object, not it's UL corner.
				// But a few objects (Cage in mimiga village) have offset drawpoints
				// that affect the positioning of the scene. If the object has a drawpoint,
				// we'll assume it's in an appropriate position, otherwise, we'll try to find
				// the center ourselves.
				if (sprites[t->sprite].frame[t->frame].dir[t->dir].drawpoint.equ(0, 0))
				{
					map.target_x = map.focus.target->CenterX() - ((Graphics::SCREEN_WIDTH / 2) << CSF);
					map.target_y = map.focus.target->CenterY() - ((Graphics::SCREEN_HEIGHT / 2) << CSF);
				}
				else
				{
					map.target_x = map.focus.target->x - ((Graphics::SCREEN_WIDTH / 2) << CSF);
					map.target_y = map.focus.target->y - ((Graphics::SCREEN_HEIGHT / 2) << CSF);
				}
			}
		}
		else
		{
			if (!player->hide)
			{
				scroll_normal();
				
				if (!inputs[DEBUG_MOVE_KEY] || !settings->enable_debug_keys)
					doing_normal_scroll = true;
			}
		}
	}
	
	map.real_xscroll += (map.target_x - map.real_xscroll) / map.scrollspeed;
	map.real_yscroll += (map.target_y - map.real_yscroll) / map.scrollspeed;
	
	map.displayed_xscroll = (map.real_xscroll + map.phase_adj);
	map.displayed_yscroll = map.real_yscroll;	// we don't compensate on Y, because player falls > 2 pixels per frame
	
	if (doing_normal_scroll)
	{
		run_phase_compensator();
		//dump_phase_data();
	}
	else
	{
		map.phase_adj -= MAP_PHASE_ADJ_SPEED;
		if (map.phase_adj < 0) map.phase_adj = 0;
	}
	
	map_sanitycheck();
	
	// do quaketime after sanity check so quake works in
	// small levels like Shack.
	if (game.quaketime)
	{
		if (!map.scroll_locked)
		{
			int pushx, pushy;
			
			if (game.megaquaketime)		// Ballos fight
			{
				game.megaquaketime--;
				pushx = random(-5, 5) << CSF;
				pushy = random(-3, 3) << CSF;
			}
			else
			{
				pushx = random(-1, 1) << CSF;
				pushy = random(-1, 1) << CSF;
			}
			
			map.real_xscroll += pushx;
			map.real_yscroll += pushy;
			map.displayed_xscroll += pushx;
			map.displayed_yscroll += pushy;
		}
		else
		{
			// quake after IronH battle...special case cause we don't
			// want to show the walls of the arena.
			int pushy = random(-0x500, 0x500);
			
			map.real_yscroll += pushy;
			if (map.real_yscroll < 0) map.real_yscroll = 0;
			if (map.real_yscroll > (15 << CSF)) map.real_yscroll = (15 << CSF);
			
			map.displayed_yscroll += pushy;
			if (map.displayed_yscroll < 0) map.displayed_yscroll = 0;
			if (map.displayed_yscroll > (15 << CSF)) map.displayed_yscroll = (15 << CSF);
		}
		
		game.quaketime--;
	}
}

// this attempts to prevent jitter most visible when the player is walking on a
// long straight stretch. the jitter occurs because map.xscroll and player->x
// tend to be out-of-phase, and thus cross over pixel boundaries at different times.
// what we do here is try to tweak/fudge the displayed xscroll value by up to 512 subpixels
// (1 real pixel), so that it crosses pixel boundaries on exactly the same frame as
// the player does.
void run_phase_compensator(void)
{
	int displayed_phase_offs = (map.displayed_xscroll - player->x) % 512;
	
	if (displayed_phase_offs != 0)
	{
		int phase_offs = abs(map.real_xscroll - player->x) % 512;
		//debug("%d", phase_offs);
		
		// move phase_adj towards phase_offs; phase_offs is how far
		// out of sync we are with the player and so once we reach it
		// we will compensating exactly.
		if (map.phase_adj < phase_offs)
		{
			map.phase_adj += MAP_PHASE_ADJ_SPEED;
			if (map.phase_adj > phase_offs)
				map.phase_adj = phase_offs;
		}
		else
		{
			map.phase_adj -= MAP_PHASE_ADJ_SPEED;
			if (map.phase_adj < phase_offs)
				map.phase_adj = phase_offs;
		}
	}
}

// debug function
void dump_phase_data()
{
	int phase_offs = abs(map.real_xscroll - player->x) % 512;
	int final_phase = abs(map.displayed_xscroll - player->x) % 512;
	debug("phase_offs: %d", phase_offs);
	debug("");
	debug("real xscroll: %d", map.real_xscroll);
	debug("displayed xscroll: %d", map.displayed_xscroll);
	debug("difference: %d", map.real_xscroll - map.displayed_xscroll);
	debug("");
	debug("phase_adj: %d", map.phase_adj);
	debug("final_phase: %d", final_phase);
}

/*
void c------------------------------() {}
*/


// scroll position sanity checking
void map_sanitycheck(void)
{
	#define MAP_BORDER_AMT		(8<<CSF)
	if (map.real_xscroll < MAP_BORDER_AMT) map.real_xscroll = MAP_BORDER_AMT;
	if (map.real_yscroll < MAP_BORDER_AMT) map.real_yscroll = MAP_BORDER_AMT;
	if (map.real_xscroll > map.maxxscroll) map.real_xscroll = map.maxxscroll;
	if (map.real_yscroll > map.maxyscroll) map.real_yscroll = map.maxyscroll;
	
	if (map.displayed_xscroll < MAP_BORDER_AMT) map.displayed_xscroll = MAP_BORDER_AMT;
	if (map.displayed_yscroll < MAP_BORDER_AMT) map.displayed_yscroll = MAP_BORDER_AMT;
	if (map.displayed_xscroll > map.maxxscroll) map.displayed_xscroll = map.maxxscroll;
	if (map.displayed_yscroll > map.maxyscroll) map.displayed_yscroll = map.maxyscroll;
}


void map_scroll_jump(int x, int y)
{
	map.target_x = x - ((Graphics::SCREEN_WIDTH / 2) << CSF);
	map.target_y = y - ((Graphics::SCREEN_HEIGHT / 2) << CSF);
	map.real_xscroll = map.target_x;
	map.real_yscroll = map.target_y;
	
	map.displayed_xscroll = map.real_xscroll;
	map.displayed_yscroll = map.real_yscroll;
	map.phase_adj = 0;
	
	map.scrollcenter_x = map.scrollcenter_y = 0;
	map_sanitycheck();
}

// lock the scroll in it's current position. the target position will not change,
// however if the scroll is moved off the target (really only a quake could do this)
// the map will still seek it's old position.
void map_scroll_lock(bool lockstate)
{
	map.scroll_locked = lockstate;
	if (lockstate)
	{	// why do we do this?
		map.real_xscroll = map.target_x;
		map.real_yscroll = map.target_y;
	}
}

// set the map focus and scroll speed.
// if o is specified, focuses on that object.
// if o is NULL, focuses on the player.
void map_focus(Object *o, int spd)
{
	map.focus.target = o;
	map.focus.has_target = (o != NULL);
	
	map.scrollspeed = spd;
	map.scroll_locked = false;
}

/*
void c------------------------------() {}
*/

// change tile at x,y into newtile while optionally spawning smoke clouds and boomflash
void map_ChangeTileWithSmoke(int x, int y, int newtile, int nclouds, bool boomflash, Object *push_behind)
{
	if (x < 0 || y < 0 || x >= map.xsize || y >= map.ysize)
		return;
	
	map.tiles[x][y] = newtile;
	
	int xa = ((x * TILE_W) + (TILE_W / 2)) << CSF;
	int ya = ((y * TILE_H) + (TILE_H / 2)) << CSF;
	SmokeXY(xa, ya, nclouds, TILE_W/2, TILE_H/2, push_behind);
	
	if (boomflash)
		effect(xa, ya, EFFECT_BOOMFLASH);
}



const char *map_get_stage_name(int mapno)
{
	if (mapno == STAGE_KINGS)
		return "";//Studio Pixel Presents";
	
	return stages[mapno].stagename;
}

// show map name for "ticks" ticks
void map_show_map_name()
{
	game.mapname_x = (Graphics::SCREEN_WIDTH / 2) - (GetFontWidth(map_get_stage_name(game.curmap), 0) / 2);
	game.showmapnametime = 120;
}

void map_draw_map_name(void)
{
	if (game.showmapnametime)
	{
		font_draw(game.mapname_x, 84, map_get_stage_name(game.curmap), 0, &shadowfont);
		game.showmapnametime--;
	}
}


// animate all motion tiles
void AnimateMotionTiles(void)
{
int i;
int x_off, y_off;

	for(i=0;i<map.nmotiontiles;i++)
	{
		switch(map.motiontiles[i].dir)
		{
			case LEFT: y_off = 0; x_off = map.motionpos; break;
			case RIGHT: y_off = 0; x_off = (TILE_W - map.motionpos); break;
			
			case UP: x_off = 0; y_off = map.motionpos; break;
			case DOWN: x_off = 0; y_off = (TILE_H - map.motionpos); break;
			
			default: x_off = y_off = 0; break;
		}
		
		CopySpriteToTile(map.motiontiles[i].sprite, map.motiontiles[i].tileno, x_off, y_off);
	}
	
	map.motionpos += 2;
	if (map.motionpos >= TILE_W) map.motionpos = 0;
}


// attempts to find an object with id2 matching the given value else returns NULL
Object *FindObjectByID2(int id2)
{
	Object *result = ID2Lookup[id2];
	
	if (result)
		staterr("FindObjectByID2: ID2 %04d found: type %s; coords: (%d, %d)", id2, DescribeObjectType(ID2Lookup[id2]->type), ID2Lookup[id2]->x>>CSF,ID2Lookup[id2]->y>>CSF);
	else
		staterr("FindObjectByID2: no such object %04d", id2);
	
	return result;
}

