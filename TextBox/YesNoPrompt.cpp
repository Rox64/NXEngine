
#include "../nx.h"
#include "YesNoPrompt.h"
#include "YesNoPrompt.fdh"
#include "../vjoy.h"

enum
{
	STATE_APPEAR,
	STATE_WAIT,
	STATE_YES_SELECTED,
	STATE_NO_SELECTED
};

#define YESNO_X				216
#define YESNO_Y				144
#define YESNO_POP_SPEED		4

/*
void c------------------------------() {}
*/

void TB_YNJPrompt::ResetState()
{
    if (fVisible != false)
        VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EYesNo, false);
    
	fVisible = false;
	fAnswer = -1;
}

void TB_YNJPrompt::SetVisible(bool enable)
{
    if (fVisible != enable)
        VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EYesNo, enable);
    
	fVisible = enable;
    

	
	if (fVisible)
	{
		fState = STATE_APPEAR;
		fCoords.y = YESNO_Y + (YESNO_POP_SPEED * 2);
		fAnswer = -1;
		
		sound(SND_MENU_PROMPT);
	}
}

/*
void c------------------------------() {}
*/

void TB_YNJPrompt::Draw()
{
	if (!fVisible)
		return;
	
	draw_sprite(YESNO_X, fCoords.y, SPR_YESNO, 0, 0);
	
    RectI yes_rect = RectI(YESNO_X - 20,  fCoords.y - 20, 20 + 39, sprites[SPR_YESNO].h + 20 + 20);
    RectI no_rect  = RectI(YESNO_X + 39, fCoords.y - 20, 20 + 41, sprites[SPR_YESNO].h + 20 + 20);
    
	// draw hand selector
	if (fState == STATE_YES_SELECTED || \
		fState == STATE_NO_SELECTED)
	{
		int xoff = (fState == STATE_YES_SELECTED) ? -4 : 37;
		draw_sprite(YESNO_X+xoff, fCoords.y+12, SPR_YESNOHAND, 0, 0);
        
        //Graphics::DrawRect(YESNO_X + xoff + 4, fCoords.y, YESNO_X + xoff + 4 + 37, fCoords.y + sprites[SPR_YESNO].h, 255, 255, 255);
	}
    
//    Graphics::DrawRect(yes_rect.x, yes_rect.y, yes_rect.x + yes_rect.w, yes_rect.y + yes_rect.h, 255, 255, 255);
//    Graphics::DrawRect(no_rect.x, no_rect.y, no_rect.x + no_rect.w, no_rect.y + no_rect.h, 255, 255, 255);

	switch(fState)
	{
		case STATE_APPEAR:
		{
			fCoords.y -= YESNO_POP_SPEED;
			
			if (fCoords.y <= YESNO_Y)
			{
				fCoords.y = YESNO_Y;
				fState = STATE_WAIT;
				fTimer = 15;
			}
			else break;
		}
		case STATE_WAIT:
		{
			if (fTimer)
			{
				fTimer--;
				break;
			}
			
			fState = STATE_YES_SELECTED;
		}
		break;
		
		case STATE_YES_SELECTED:
		case STATE_NO_SELECTED:
		{
            bool accept = false;
#ifdef CONFIG_USE_TAPS
            if (VJoy::ModeAware::wasTap(yes_rect))
            {
                if (fState == STATE_YES_SELECTED)
                    accept = true;
                else
                    fState = STATE_YES_SELECTED;
            }
            if (VJoy::ModeAware::wasTap(no_rect))
            {
                if (fState == STATE_NO_SELECTED)
                    accept = true;
                else
                    fState = STATE_NO_SELECTED;
            }
#else
            
            
			if (justpushed(LEFTKEY) || justpushed(RIGHTKEY))
			{
				sound(SND_MENU_MOVE);
				
				fState = (fState == STATE_YES_SELECTED) ?
							STATE_NO_SELECTED : STATE_YES_SELECTED;
			}
            accept = justpushed(JUMPKEY);
#endif
			
			if (accept)
			{
				sound(SND_MENU_SELECT);
				lastinputs[JUMPKEY] = true;
				lastpinputs[JUMPKEY] = true;
				
				fAnswer = (fState == STATE_YES_SELECTED) ? YES : NO;
				SetVisible(false);
			}
		}
		break;
	}
}

/*
void c------------------------------() {}
*/

bool TB_YNJPrompt::ResultReady()
{
	return (fAnswer != -1);
}

int TB_YNJPrompt::GetResult()
{
	return fAnswer;
}

