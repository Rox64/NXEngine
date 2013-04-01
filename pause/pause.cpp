
#include "../nx.h"
#include "pause.fdh"
#include "../vjoy.h"

bool pause_init(int param)
{
	memset(lastinputs, 1, sizeof(lastinputs));
	return 0;
}

void pause_tick()
{
	ClearScreen(BLACK);
	
	int cx = (Graphics::SCREEN_WIDTH / 2) - (sprites[SPR_RESETPROMPT].w / 2);
	int cy = (Graphics::SCREEN_HEIGHT / 2) - (sprites[SPR_RESETPROMPT].h / 2);
	draw_sprite(cx, cy, SPR_RESETPROMPT);
	
	// tap control
	{
		RectI r = RectI(cx + 60, cy, 82, sprites[SPR_RESETPROMPT].h);
		debug_absbox(r.x, r.y, r.x + r.w, r.y + r.h, 255, 255, 255);
		if (VJoy::ModeAware::wasTap(r))
		{
			game.pause(false);
		}

		r = RectI(cx + 60 + 82, cy, 70, sprites[SPR_RESETPROMPT].h);
		debug_absbox(r.x, r.y, r.x + r.w, r.y + r.h, 255, 255, 255);
		if (VJoy::ModeAware::wasTap(r))
		{
			game.reset();
		}
	}
	
	const char *str = "F3:Options";
	cx = (Graphics::SCREEN_WIDTH / 2) - (GetFontWidth(str, 0) / 2) - 4;
	cy = (Graphics::SCREEN_HEIGHT - 8) - GetFontHeight();
	int f3wd = font_draw(cx, cy, "F3", 0);
	font_draw(cx + f3wd, cy, ":Options", 0, &bluefont);
	
	// tap control
	{
		RectI r = RectI(cx, cy, GetFontWidth(str, 0), GetFontHeight());
		debug_absbox(r.x, r.y, r.x + r.w, r.y + r.h, 255, 255, 255);
		if (VJoy::ModeAware::wasTap(r))
		{
			game.pause(GP_OPTIONS);
		}
	}
	
	// resume
	if (justpushed(F1KEY))
	{
		lastinputs[F1KEY] = true;
		game.pause(false);
		return;
	}
	
	// reset
	if (justpushed(F2KEY))
	{
		lastinputs[F2KEY] = true;
		game.reset();
		return;
	}
	
	// exit
	if (justpushed(ESCKEY))
	{
		lastinputs[ESCKEY] = true;
		game.running = false;
		return;
	}

}






