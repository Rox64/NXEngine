
/*
	The stage-select dialog when using the
	teleporter in Arthur's House.
*/

#include "../nx.h"
#include "StageSelect.h"
#include "StageSelect.fdh"
#include "../vjoy.h"

#define WARP_X			128
#define WARP_Y			46

#define WARP_Y_START	(WARP_Y + 8)
#define WARP_Y_SPEED	1

#define LOCS_Y			(WARP_Y + 16)
#define LOCS_SPACING	8

TB_StageSelect::TB_StageSelect()
{
	ClearSlots();
}

/*
void c------------------------------() {}
*/

void setSpecScreenState(int active_slots, bool enable)
{
	// EStageSelect1 - for the level selection
	// EStageSelect2 - like normal textbox mode and used when level is selected or
	// there are no levels to select.
	
	VJoy::ModeAware::SpecScreens s = active_slots ? VJoy::ModeAware::EStageSelect1 : VJoy::ModeAware::EStageSelect2;
	VJoy::ModeAware::specScreenChanged(s, enable);
}

void TB_StageSelect::ResetState()
{
	if (fVisible != false)
		setSpecScreenState(CountActiveSlots(), false);
	fVisible = false;
}

void TB_StageSelect::SetVisible(bool enable)
{
	if (fVisible != enable)
		setSpecScreenState(CountActiveSlots(), enable);
	
	fVisible = enable;
	fWarpY = WARP_Y_START;
	
	game.frozen = enable;
	textbox.SetFlags(TB_CURSOR_NEVER_SHOWN, enable);
	textbox.SetFlags(TB_LINE_AT_ONCE, enable);
	textbox.SetFlags(TB_VARIABLE_WIDTH_CHARS, enable);
	
	fSelectionIndex = 0;
	fLastButtonDown = true;
	
	if (enable)
	{
		fMadeSelection = false;
		textbox.ClearText();
		UpdateText();
	}
}

bool TB_StageSelect::IsVisible()
{
	return fVisible;
}

/*
void c------------------------------() {}
*/

void TB_StageSelect::Draw(void)
{
	if (!fVisible)
		return;
	
	// handle user input
	HandleInput();
	
	// draw "- WARP -" text
	fWarpY -= WARP_Y_SPEED;
	if (fWarpY < WARP_Y) fWarpY = WARP_Y;
	
	draw_sprite(WARP_X, fWarpY, SPR_TEXT_WARP, 0);
	
	// draw teleporter locations
	int nslots = CountActiveSlots();
	int total_spacing = ((nslots - 1) * LOCS_SPACING);
	int total_width = total_spacing + (nslots * sprites[SPR_STAGEIMAGE].w);
	int x = (Graphics::SCREEN_WIDTH / 2) - (total_width / 2);
	
	for(int i=0;i<nslots;i++)
	{
		int sprite;
		GetSlotByIndex(i, &sprite, NULL);
		
		draw_sprite(x, LOCS_Y, SPR_STAGEIMAGE, sprite);
		
		if (i == fSelectionIndex)
		{
			fSelectionFrame ^= 1;
			draw_sprite(x, LOCS_Y, SPR_SELECTOR_ITEMS, fSelectionFrame);
		}
		
		x += (sprites[SPR_STAGEIMAGE].w + LOCS_SPACING);
	}
}

/*
void c------------------------------() {}
*/

void TB_StageSelect::HandleInput()
{
	bool button_down = false;

	if (textbox.YesNoPrompt.IsVisible() || fMadeSelection)
		return;
	
#ifdef CONFIG_USE_TAPS
	// taps control
	{
		int nslots = CountActiveSlots();
		int total_spacing = ((nslots - 1) * LOCS_SPACING);
		int total_width = total_spacing + (nslots * sprites[SPR_STAGEIMAGE].w);
		int x = (Graphics::SCREEN_WIDTH / 2) - (total_width / 2);
		
		for (int i = 0; i < nslots; ++i)
		{
			RectI rect = Sprites::get_sprite_rect(x, LOCS_Y, SPR_STAGEIMAGE);
			if (VJoy::ModeAware::wasTap(rect))
			{
				if (fSelectionIndex == i)
				{
					button_down = true;
					fLastButtonDown = false;
					
					VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EStageSelect1, false);
					VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EStageSelect2, true);
				}
				else
				{
					fSelectionIndex = i;
					sound(SND_MENU_MOVE);
					UpdateText();
				}
				
				break;
			}
			
			x += (sprites[SPR_STAGEIMAGE].w + LOCS_SPACING);
		}
	}
#endif 
	
	// pad control
	{
		if (justpushed(LEFTKEY))
		{
			MoveSelection(LEFT);
		}
		else if (justpushed(RIGHTKEY))
		{
			MoveSelection(RIGHT);
		}
		
		// when user picks a location return the new script to execute
		button_down = button_down || buttondown();
	}
	
	if (button_down && !fLastButtonDown)
	{
		int scriptno;
		if (!GetSlotByIndex(fSelectionIndex, NULL, &scriptno))
		{
			stat("StageSelect: starting activation script %d", scriptno);
			JumpScript(scriptno, SP_MAP);
		}
		else
		{	// dismiss "no permission to teleport"
			StopScripts();
		}
		
		fMadeSelection = true;
	}
	
	fLastButtonDown = button_down;
}

void TB_StageSelect::MoveSelection(int dir)
{
	int numslots = CountActiveSlots();
	if (numslots == 0) return;
	
	if (dir == RIGHT)
	{
		if (++fSelectionIndex >= numslots)
			fSelectionIndex = 0;
	}
	else
	{
		if (--fSelectionIndex < 0)
			fSelectionIndex = (numslots - 1);
	}
	
	sound(SND_MENU_MOVE);
	UpdateText();
}

// updates the text by running the appropriate script
// from StageSelect.tsc
void TB_StageSelect::UpdateText()
{
int scriptno;

	if (GetSlotByIndex(fSelectionIndex, NULL, &scriptno))
	{	// no permission to teleport
		scriptno = 0;
	}
	else
	{
		scriptno %= 1000;
	}
	
	JumpScript(scriptno + 1000, SP_STAGESELECT);
}

/*
void c------------------------------() {}
*/

// set teleporter slot "slotno" to run script "scriptno" when selected.
// this adds the slot to the menu if scriptno is nonzero and removes it if zero.
// the parameters here map directory to the <PS+ in the script.
void TB_StageSelect::SetSlot(int slotno, int scriptno)
{
	if (slotno >= 0 && slotno < NUM_TELEPORTER_SLOTS)
	{
		fSlots[slotno] = scriptno;
	}
	else
	{
		stat("StageSelect::SetSlot: invalid slotno %d", slotno);
	}
}

void TB_StageSelect::ClearSlots()
{
	for(int i=0;i<NUM_TELEPORTER_SLOTS;i++)
		fSlots[i] = -1;
}

// return the slotno and scriptno associated with the n'th enabled teleporter slot,
// where n = index.
// i.e. passing 1 for index returns the 2nd potential teleporter destination.
// if index is higher than the number of active teleporter slots, returns nonzero.
bool TB_StageSelect::GetSlotByIndex(int index, int *slotno_out, int *scriptno_out)
{
	if (index >= 0)
	{
		int slots_found = 0;
		
		for(int i=0;i<NUM_TELEPORTER_SLOTS;i++)
		{
			if (fSlots[i] != -1)
			{
				if (++slots_found > index)
				{
					if (slotno_out)	  *slotno_out = i;
					if (scriptno_out) *scriptno_out = fSlots[i];
					return 0;
				}
			}
		}
	}
	
	if (slotno_out)   *slotno_out = -1;
	if (scriptno_out) *scriptno_out = -1;
	return 1;
}

int TB_StageSelect::CountActiveSlots()
{
	int count = 0;
	
	for(int i=0;i<NUM_TELEPORTER_SLOTS;i++)
	{
		if (fSlots[i] != -1)
			count++;
	}
	
	return count;
}



