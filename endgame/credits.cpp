
#include "../nx.h"
#include "credits.h"
#include "credits.fdh"

#define MARGIN			48
#define SCREEN_Y(Y)		( (Y) - (scroll_y >> CSF) )
#define TEXT_SPACING	5	// X-spacing between letters
	
Credits *credits = NULL;

/*
void c------------------------------() {}
*/

bool Credits::Init()
{
	if (script.OpenFile()) return 1;
	if (bigimage.Init()) return 1;
	Replay::end_record();
	
	spawn_y = (Graphics::SCREEN_HEIGHT + 8);
	scroll_y = 0 << CSF;
	
	xoffset = 0;
	roll_running = true;
	
	lines_out = lines_vis = 0;
	
	firstline = NULL;
	lastline = NULL;
	
	return 0;
}

Credits::~Credits()
{
	script.CloseFile();
}

/*
void c------------------------------() {}
*/

void Credits::Tick()
{
	/*debug("scroll_y: %d", scroll_y>>CSF);
	debug("spawn_y: %d", spawn_y);
	debug("scr_spawn_y: %d", SCREEN_Y(spawn_y));
	debug("trigger: %d", Graphics::SCREEN_HEIGHT+MARGIN);
	debug("");*/
	/*debug("imgno: %d", bigimage.imgno);
	debug("state: %d", bigimage.state);
	debug("imagex: %d", bigimage.imagex);*/
	
	if (roll_running || SCREEN_Y(spawn_y) >= (Graphics::SCREEN_HEIGHT + 8))
	{
		scroll_y += 0x100;
	}
	
	while(roll_running && SCREEN_Y(spawn_y) < (Graphics::SCREEN_HEIGHT + MARGIN))
	{
		RunNextCommand();
	}
	
	if (player)
	{
		player->hide = true;
		player->dead = true;	// should pretty much completely disable HandlePlayer()
	}
	
	game_tick_normal();
	bigimage.Draw();
	Draw();
}


void Credits::RunNextCommand()
{
CredCommand cmd;

	if (script.ReadCommand(&cmd))
	{
		console.Print("script.ReadCommand failed: credits terminated");
		roll_running = false;
		return;
	}
	
	cmd.DumpContents();
	
	switch(cmd.type)
	{
		case CC_TEXT:
		{
			CredLine *line = NewLine();
			
			maxcpy(line->text, cmd.text, sizeof(line->text));
			line->image = cmd.parm;
			line->x = xoffset;
			line->y = spawn_y;
			
			// the last line is supposed to be centered--slightly
			// varying font sizes can lead to it being a little bit off
			if (strstr(line->text, "The End"))
			{
				line->x = (Graphics::SCREEN_WIDTH / 2) - (GetFontWidth(line->text, TEXT_SPACING) / 2);
			}
			
			spawn_y += 1;
			lines_out++;
		}
		break;
		
		case CC_BLANK_SPACE:
			spawn_y += cmd.parm / 2;
		break;
		
		case CC_SET_XOFF:
			xoffset = cmd.parm;
		break;
		
		case CC_FLAGJUMP:
			if (game.flags[cmd.parm])
				Jump(cmd.parm2);
		break;
		
		case CC_JUMP:
			Jump(cmd.parm);
		break;
		
		case CC_LABEL:
		break;
		
		case CC_MUSIC:		 music(cmd.parm); break;
		case CC_FADE_MUSIC:	 org_fade(); break;
		
		case CC_END:		 roll_running = false; break;
		
		default:
			console.Print("Unhandled command '%c'; credits terminated", cmd.type);
			roll_running = false;
		break;
	}
}

bool Credits::Jump(int label)
{
CredCommand cmd;
bool tried_rewind = false;

	stat("- Jump to label %04d", label);
	
	for(;;)
	{
		if (script.ReadCommand(&cmd) || cmd.type == CC_END)
		{	// I think all the jumps in the original credits are forwards,
			// so only try looking back if there's a problem finding the label.
			if (!tried_rewind)
			{
				script.Rewind();
				tried_rewind = true;
			}
			else
			{
				console.Print("Missing label %04d; credits terminated", label);
				roll_running = false;
				return 1;
			}
		}
		
		if (cmd.type == CC_LABEL && cmd.parm == label)
		{
			return 0;
		}
	}
}

/*
void c------------------------------() {}
*/

bool Credits::DrawLine(CredLine *line)
{
	int x = line->x;
	int y = SCREEN_Y(line->y);
	if (y < -MARGIN) return true;	// line can be deleted now
	
	if (line->image)
	{
		draw_sprite(x - 24, y - 8, SPR_CASTS, line->image);
		//DrawBox(x, y, x+GetFontWidth(line->text, TEXT_SPACING), y+8,  56, 0, 0);
	}
	
	//int font_draw(int x, int y, const char *string, int font_spacing)
	//DrawRect(x, y, x+63, y+8, 128, 0, 0);
	font_draw(x, y, line->text, TEXT_SPACING);
	
	return false;
}


void Credits::Draw()
{
CredLine *line, *next;

	line = firstline;
	while(line)
	{
		next = line->next;
		
		if (DrawLine(line))
		{
			RemoveLine(line);
			delete line;
		}
		
		line = next;
	}
}

/*
void c------------------------------() {}
*/

CredLine *Credits::NewLine()
{
	return AddLine(new CredLine);
}

CredLine *Credits::AddLine(CredLine *line)
{
	line->prev = NULL;
	line->next = firstline;
	
	if (firstline)
	{
		firstline->prev = line;
		firstline = line;
	}
	else
	{
		firstline = lastline = line;
	}
	
	lines_vis++;
	return line;
}

void Credits::RemoveLine(CredLine *line)
{
	if (line->next) line->next->prev = line->prev;
	if (line->prev) line->prev->next = line->next;
	if (line == firstline) firstline = firstline->next;
	if (line == lastline) lastline = lastline->next;
	lines_vis--;
}

/*
void c------------------------------() {}
*/

enum BIStates
{
	BI_CLEAR,
	BI_SLIDE_IN,
	BI_SLIDE_OUT,
	BI_HOLD
};

bool BigImage::Init()
{
char fname[MAXPATHLEN];

	imagex = 0;
	imgno = 0;
	state = BI_CLEAR;
	memset(images, 0, sizeof(images));
	
	// load any images present
	for(int i=0;i<MAX_BIGIMAGES;i++)
	{
		sprintf(fname, "%s/credit%02d.bmp", pic_dir, i);
		if (file_exists(fname))
		{
			images[i] = NXSurface::FromFile(fname, false);
			if (!images[i])
				staterr("BigImage::Init: image '%s' exists but seems corrupt!", fname);
			else
				stat("BigImage: loaded %s ok", fname);
		}
	}
	
	return 0;
}

BigImage::~BigImage()
{
	for(int i=0;i<MAX_BIGIMAGES;i++)
	{
		if (images[i])
		{
			staterr("BigImage: freeing image %d", i);
			delete images[i];
			images[i] = NULL;
		}
	}
}


void BigImage::Set(int num)
{
	if (images[num])
	{
		imgno = num;
		imagex = -images[num]->Width();
		state = BI_SLIDE_IN;
	}
	else
	{
		staterr("BigImage::Set: invalid image number %d", num);
		state = BI_CLEAR;
	}
}

void BigImage::Clear()
{
	state = BI_SLIDE_OUT;
}

void BigImage::Draw()
{
	#define IMAGE_SPEED		32
	
	switch(state)
	{
		case BI_SLIDE_IN:
		{
			imagex += IMAGE_SPEED;
			if (imagex > 0)
			{
				imagex = 0;
				state = BI_HOLD;
			}
		}
		break;
		
		case BI_SLIDE_OUT:
		{
			imagex -= IMAGE_SPEED;
			if (imagex < -images[imgno]->Width())
				state = BI_CLEAR;
		}
	}
	
	// take up any unused space with blue
	if (state != BI_HOLD)
		FillRect(0, 0, Graphics::SCREEN_WIDTH/2, Graphics::SCREEN_HEIGHT, DK_BLUE);
	
	if (state != BI_CLEAR)
		DrawSurface(images[imgno], imagex, 0);
}


/*
void c------------------------------() {}
*/

bool credit_init(int parameter)
{
	credits = new Credits;
	if (credits->Init())
	{
		staterr("Credits initilization failed");
		return 1;
	}
	
	return 0;
}

void credit_close()
{
	delete credits;
	credits = NULL;
}

void credit_tick()
{
	if (credits)
		credits->Tick();
}

void credit_set_image(int imgno)
{
	if (credits)
		credits->bigimage.Set(imgno);
}

void credit_clear_image()
{
	if (credits)
		credits->bigimage.Clear();
}

/*
void c------------------------------() {}
*/




