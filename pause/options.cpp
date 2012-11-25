
#include "../nx.h"
#include "../replay.h"
#include "options.h"
#include "dialog.h"
#include "message.h"
#include "../platform/platform.h"

using namespace Options;
#include "options.fdh"
FocusStack optionstack;

#define SLIDE_SPEED				32
#define SLIDE_DIST				(3 * SLIDE_SPEED)

static struct
{
	Dialog *dlg, *subdlg;
	Dialog *dismiss_on_focus;
	int mm_cursel;
	bool InMainMenu;
	int xoffset;
	
	int selected_replay;
	int remapping_key, new_sdl_key;
} opt;

static void _vkey_edit_draw();

bool options_init(int retmode)
{
	memset(&opt, 0, sizeof(opt));
	Options::init_objects();
	opt.dlg = new Dialog;
	
	opt.xoffset = SLIDE_DIST;
	opt.dlg->offset(-SLIDE_DIST, 0);
	
	EnterMainMenu();
	opt.dlg->ondismiss = DialogDismissed;
	opt.dlg->ShowFull();
	
	inputs[F3KEY] = 0;
	sound(SND_MENU_MOVE);
	return 0;
}

void options_close()
{
	Options::close_objects();
	while(FocusHolder *fh = optionstack.ItemAt(0))
		delete fh;
	
	settings_save();
}

/*
void c------------------------------() {}
*/

void options_tick()
{
int i;
FocusHolder *fh;

	if (justpushed(F3KEY))
	{
		game.pause(0);
		return;
	}
	
	ClearScreen(BLACK);
	Options::run_and_draw_objects();
	
	fh = optionstack.ItemAt(optionstack.CountItems() - 1);
	if (fh)
	{
		fh->RunInput();
		if (game.paused != GP_OPTIONS) return;
		
		fh = optionstack.ItemAt(optionstack.CountItems() - 1);
		if (fh == opt.dismiss_on_focus && fh)
		{
			opt.dismiss_on_focus = NULL;
			delete fh;
		}
	}
	
	for(i=0;;i++)
	{
		fh = optionstack.ItemAt(i);
		if (!fh) break;
		
		fh->Draw();
	}
    
    _vkey_edit_draw();
	
	if (opt.xoffset > 0)
	{
		opt.dlg->offset(SLIDE_SPEED, 0);
		opt.xoffset -= SLIDE_SPEED;
	}
}


/*
void c------------------------------() {}
*/

void DialogDismissed()
{
	if (opt.InMainMenu)
	{
		memset(inputs, 0, sizeof(inputs));
		game.pause(false);
	}
	else
	{
		EnterMainMenu();
	}
}


/*
void c------------------------------() {}
*/

static void EnterTapControlsMenu(ODItem *item, int dir);
static void EnterVjoyControlsMenu(ODItem *item, int dir);

static void EnterMainMenu()
{
Dialog *dlg = opt.dlg;

	dlg->Clear();
	
#if !defined(IPHONE)
	dlg->AddItem("Resolution: ", _res_change, _res_get);
	dlg->AddItem("Controls", EnterControlsMenu);
#endif
    
#ifdef CONFIG_USE_TAPS
    dlg->AddItem("Tap controls", EnterTapControlsMenu);
#endif
    
#ifdef CONFIG_USE_VJOY
    dlg->AddItem("Virtual keys", EnterVjoyControlsMenu);
#endif
    
	dlg->AddItem("Replay", EnterReplayMenu);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Enable Debug Keys", _debug_change, _debug_get);
	dlg->AddItem("Save Slots: ", _save_change, _save_get);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Music: ", _music_change, _music_get);
	dlg->AddItem("Sound: ", _sound_change, _sound_get);
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
	
	dlg->SetSelection(opt.mm_cursel);
	dlg->onclear = LeavingMainMenu;
	opt.InMainMenu = true;
}

void LeavingMainMenu()
{
	opt.mm_cursel = opt.dlg->GetSelection();
	opt.dlg->onclear = NULL;
	opt.InMainMenu = false;
}

void _res_get(ODItem *item)
{
	const char **reslist = Graphics::GetResolutions();
	
	if (settings->resolution < 0 || \
		settings->resolution >= count_string_list(reslist))
	{
		item->suffix[0] = 0;
	}
	else
	{
		strcpy(item->suffix, reslist[settings->resolution]);
	}
}


void _res_change(ODItem *item, int dir)
{
const char **reslist = Graphics::GetResolutions();
int numres = count_string_list(reslist);
int newres;

	sound(SND_DOOR);
	
	newres = (settings->resolution + dir);
	if (newres >= numres) newres = 0;
	if (newres < 0) newres = (numres - 1);
	
	// because on my computer, a SDL bug causes switching to fullscreen to
	// not restore the resolution properly on exit, and it keeps messing up all
	// the windows when I press it accidently.
	if (newres == 0 && settings->inhibit_fullscreen)
	{
		new Message("Fullscreen disabled via", "inhibit-fullscreen console setting");
		sound(SND_GUN_CLICK);
		return;
	}
	
	if (!Graphics::SetResolution(newres, true))
	{
		settings->resolution = newres;
	}
	else
	{
		new Message("Resolution change failed");
		sound(SND_GUN_CLICK);
	}
}


void _debug_change(ODItem *item, int dir)
{
	settings->enable_debug_keys ^= 1;
	sound(SND_MENU_SELECT);
}

void _debug_get(ODItem *item)
{
	static const char *strs[] = { "", " =" };
	strcpy(item->suffix, strs[settings->enable_debug_keys]);
}


void _save_change(ODItem *item, int dir)
{
	settings->multisave ^= 1;
	sound(SND_MENU_MOVE);
}

void _save_get(ODItem *item)
{
	static const char *strs[] = { "1", "5" };
	strcpy(item->suffix, strs[settings->multisave]);
}


void _sound_change(ODItem *item, int dir)
{
	settings->sound_enabled ^= 1;
	sound(SND_MENU_SELECT);
}

void _sound_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On" };
	strcpy(item->suffix, strs[settings->sound_enabled]);
}



void _music_change(ODItem *item, int dir)
{
	music_set_enabled((settings->music_enabled + 1) % 3);
	sound(SND_MENU_SELECT);
}

void _music_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On", "Boss Only" };
	strcpy(item->suffix, strs[settings->music_enabled]);
}

/*
void c------------------------------() {}
*/

static void EnterReplayMenu(ODItem *item, int dir)
{
Dialog *dlg = opt.dlg;
ReplaySlotInfo slot;
bool have_replays = false;

	dlg->Clear();
	sound(SND_MENU_MOVE);
	
	for(int i=0;i<MAX_REPLAYS;i++)
	{
		Replay::GetSlotInfo(i, &slot);
		
		if (slot.status != RS_UNUSED)
		{
			const char *mapname = map_get_stage_name(slot.hdr.stageno);
			dlg->AddItem(mapname, EnterReplaySubmenu, _upd_replay, i);
			have_replays = true;
		}
	}
	
	if (!have_replays)
		dlg->AddDismissalItem("[no replays yet]");
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

void _upd_replay(ODItem *item)
{
ReplaySlotInfo slot;

	Replay::GetSlotInfo(item->id, &slot);
	
	Replay::FramesToTime(slot.hdr.total_frames, &item->raligntext[1]);
	item->raligntext[0] = (slot.hdr.locked) ? '=':' ';
}

/*
void c------------------------------() {}
*/

void EnterReplaySubmenu(ODItem *item, int dir)
{
	opt.selected_replay = item->id;
	sound(SND_MENU_MOVE);
	
	opt.subdlg = new Dialog;
	opt.subdlg->SetSize(80, 60);
	
	opt.subdlg->AddItem("Play", _play_replay);
	opt.subdlg->AddItem("Keep", _keep_replay);
}


void _keep_replay(ODItem *item, int dir)
{
char fname[MAXPATHLEN];
ReplayHeader hdr;

	GetReplayName(opt.selected_replay, fname);
	
	if (Replay::LoadHeader(fname, &hdr))
	{
		new Message("Failed to load header.");
		sound(SND_GUN_CLICK);
		opt.dismiss_on_focus = opt.subdlg;
		return;
	}
	
	hdr.locked ^= 1;
	
	if (Replay::SaveHeader(fname, &hdr))
	{
		new Message("Failed to write header.");
		sound(SND_GUN_CLICK);
		opt.dismiss_on_focus = opt.subdlg;
		return;
	}
	
	sound(SND_MENU_MOVE);
	opt.subdlg->Dismiss();
	opt.dlg->Refresh();
}


void _play_replay(ODItem *item, int dir)
{
	game.switchstage.param = opt.selected_replay;
	game.switchmap(START_REPLAY);
	
	game.setmode(GM_NORMAL);
	game.pause(false);
}

/*
void c------------------------------() {}
*/

static void EnterControlsMenu(ODItem *item, int dir)
{
Dialog *dlg = opt.dlg;

	dlg->Clear();
	sound(SND_MENU_MOVE);
	
	dlg->AddItem("Left", _edit_control, _upd_control, LEFTKEY);
	dlg->AddItem("Right", _edit_control, _upd_control, RIGHTKEY);
	dlg->AddItem("Up", _edit_control, _upd_control, UPKEY);
	dlg->AddItem("Down", _edit_control, _upd_control, DOWNKEY);
	
	dlg->AddSeparator();
	
	dlg->AddItem("Jump", _edit_control, _upd_control, JUMPKEY);
	dlg->AddItem("Fire", _edit_control, _upd_control,  FIREKEY);
	dlg->AddItem("Wpn Prev", _edit_control, _upd_control, PREVWPNKEY);
	dlg->AddItem("Wpn Next", _edit_control, _upd_control, NEXTWPNKEY);
	dlg->AddItem("Inventory", _edit_control, _upd_control, INVENTORYKEY);
	dlg->AddItem("Map", _edit_control, _upd_control, MAPSYSTEMKEY);
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

static void _upd_control(ODItem *item)
{
	int keysym = input_get_mapping(item->id);
	const char *keyname = SDL_GetKeyName((SDL_Keycode)keysym);
	
	maxcpy(item->righttext, keyname, sizeof(item->righttext) - 1);
}

static void _edit_control(ODItem *item, int dir)
{
Message *msg;

	opt.remapping_key = item->id;
	opt.new_sdl_key = -1;
	
	msg = new Message("Press new key for:", input_get_name(opt.remapping_key));
	msg->rawKeyReturn = &opt.new_sdl_key;
	msg->on_dismiss = _finish_control_edit;
	
	sound(SND_DOOR);
}

static void _finish_control_edit(Message *msg)
{
int inputno = opt.remapping_key;
int new_sdl_key = opt.new_sdl_key;
int i;

	if (input_get_mapping(inputno) == new_sdl_key)
	{
		sound(SND_MENU_MOVE);
		return;
	}
	
	// check if key is already in use
	for(i=0;i<INPUT_COUNT;i++)
	{
		if (i != inputno && input_get_mapping(i) == new_sdl_key)
		{
			new Message("Key already in use by:", input_get_name(i));
			sound(SND_GUN_CLICK);
			return;
		}
	}
	
	input_remap(inputno, new_sdl_key);
	sound(SND_MENU_SELECT);
	opt.dlg->Refresh();
}


/*
 void c------------------------------() {}
 */

static void _get_tap_control(ODItem *item);
static void _edit_tap_control(ODItem *item, int dir);

static void EnterTapControlsMenu(ODItem *item, int dir)
{
    Dialog *dlg = opt.dlg;
    
	dlg->Clear();
	sound(SND_MENU_MOVE);
	
	dlg->AddItem("Tap controls", _edit_tap_control, _get_tap_control, Settings::Tap::EAll);
    
	dlg->AddSeparator();
    
	dlg->AddItem("Movies",      _edit_tap_control, _get_tap_control, Settings::Tap::EMovies);
	dlg->AddItem("Title",       _edit_tap_control, _get_tap_control, Settings::Tap::ETitle);
	dlg->AddItem("Save/load",   _edit_tap_control, _get_tap_control, Settings::Tap::ESaveLoad);
	dlg->AddItem("Dialogs",     _edit_tap_control, _get_tap_control, Settings::Tap::EIngameDialog);
	dlg->AddItem("Inventory",   _edit_tap_control, _get_tap_control, Settings::Tap::EInventory);
	dlg->AddItem("Pause",       _edit_tap_control, _get_tap_control, Settings::Tap::EPause);
	dlg->AddItem("Options",     _edit_tap_control, _get_tap_control, Settings::Tap::EOptions);
	dlg->AddItem("MapSystem",   _edit_tap_control, _get_tap_control, Settings::Tap::EMapSystem);
    
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

static void _get_tap_control(ODItem *item)
{
    static char const * const values[Settings::Tap::EMODELAST] = {"Tap only", "Pad only", "Tap/pad"};
	
	maxcpy(item->righttext, values[settings->tap[item->id]], sizeof(item->righttext) - 1);
}

static void _edit_tap_control(ODItem *item, int dir)
{
    settings->tap[item->id] = (settings->tap[item->id] + 1) % Settings::Tap::EMODELAST;
    sound(SND_MENU_SELECT);
    
    if (Settings::Tap::EAll == item->id)
    {
        for (int i = Settings::Tap::EAll + 1; i < Settings::Tap::ELASTPLACE; ++i)
        {
            settings->tap[i] = settings->tap[Settings::Tap::EAll];
        }
        
        item->parent->UpdateAllItems();
    }
}

/*
 void c------------------------------() {}
 */

static void _edit_view_preset(ODItem *item, int dir);
static void _get_view_preset(ODItem *item);
static void _apply_preset(ODItem *item, int dir);
static void _enter_edit_buttons(ODItem *item, int dir);

void (*beforeVjoyControlsDisiss)() = NULL;
struct {
    VJoy::Preset preset;
    int num;
    bool need_restore;
} preset_restore;

static void _vjoy_controls_menu_dissmiss()
{
    if (preset_restore.need_restore)
    {
        settings->vjoy_controls = preset_restore.preset;
        settings->vjoy_current_preset = preset_restore.num;
        VJoy::setUpdated();
    }
    
    VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EOptsVkeyMenu, false);
    
    Dialog *dlg = opt.dlg;
    dlg->ondismiss = beforeVjoyControlsDisiss;
    EnterMainMenu();
}

static void _setup_vjoy_controls_menu()
{
    Dialog *dlg = opt.dlg;
    
    dlg->Clear();
	sound(SND_MENU_MOVE);
	
	dlg->AddItem("View preset", _edit_view_preset, _get_view_preset);
    dlg->AddItem("Apply preset", _apply_preset);
    
	dlg->AddSeparator();
    
    dlg->AddItem("Edit buttons", _enter_edit_buttons);
    
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

static void EnterVjoyControlsMenu(ODItem *item, int dir)
{
    Dialog *dlg = opt.dlg;
    
    _setup_vjoy_controls_menu();
    
    beforeVjoyControlsDisiss = dlg->ondismiss;
    dlg->ondismiss = _vjoy_controls_menu_dissmiss;
    
    preset_restore.preset = settings->vjoy_controls;
    preset_restore.num = settings->vjoy_current_preset;
    preset_restore.need_restore = true;
    
    VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EOptsVkeyMenu, true);
}

static void _edit_view_preset(ODItem *item, int dir)
{
    int newpres = (settings->vjoy_current_preset + dir);
	if (newpres >= VJoy::getPresetsCount()) newpres = 0;
	if (newpres < 0) newpres = (VJoy::getPresetsCount() - 1);
    
    settings->vjoy_controls = VJoy::getPreset(newpres);
    settings->vjoy_current_preset = newpres;
    VJoy::setUpdated();
    
    preset_restore.need_restore = true;
    
    sound(SND_DOOR);
}

static void _get_view_preset(ODItem *item)
{
    snprintf(item->righttext, sizeof(item->righttext), "%d", settings->vjoy_current_preset);
}

static void _apply_preset(ODItem *item, int dir)
{
    preset_restore.preset = settings->vjoy_controls;
    preset_restore.num = settings->vjoy_current_preset;
    preset_restore.need_restore = false;
    
    sound(SND_MENU_SELECT);
}

struct VkeyEdit
{
    VkeyEdit() :
        enabled(false),
        selected(-1)
    {}
    
    bool enabled;
    int selected;
} vkeyEdit;

struct EditEventHandler : public VJoy::IEditEventHandler
{
    virtual void end()
    {
        VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EOptsVkeyEdit, false);
        VJoy::setEditEventHandler(NULL);
        vkeyEdit.enabled = false;
        _setup_vjoy_controls_menu();
    }
    
    virtual void selected(int k)
    {
        //stat("selected k = %d", k);
        vkeyEdit.selected = k;
    }
    
    virtual void selectedPad(bool enter)
    {
        //stat("selected pad");
        vkeyEdit.selected = enter ? INPUT_COUNT : -1;
    }
} editEventhandler;


static void _enter_edit_buttons(ODItem *item, int dir)
{
    opt.dlg->Clear();
    VJoy::ModeAware::specScreenChanged(VJoy::ModeAware::EOptsVkeyEdit, true);
    VJoy::setEditEventHandler(&editEventhandler);
    vkeyEdit.enabled = true;
}

static void _vkey_edit_draw()
{
    static char const * const key_names[INPUT_COUNT + 1] =
    {
        "Left", "Right", "Up", "Down",
        "Jump", "Fire",
        "Wpn Prev", "Wpn Next",
        "Inventory", "Map",
        
        "Esc",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        
        "Freeze frame",
        "Frame advance",
        "Debug fly",
        
        "Pad"
    };
    
    if (!vkeyEdit.enabled)
        return;
    
    if (vkeyEdit.selected < 0)
    {
        int x = 10;
        int y = 10;
        font_draw(x, y, "Pinch to exit");
        font_draw(x, y += GetFontHeight(), "Tap to select button");
    }
    else
    {
        int x = 10;
        int y = 10;
        font_draw(x, y, key_names[vkeyEdit.selected], 0, &greenfont);
        font_draw(x, y += GetFontHeight(), "Tap to unselect button/select next");
        font_draw(x, y += GetFontHeight(), "Pan to move button");
        font_draw(x, y += GetFontHeight(), "Pinch/spread to resize button");
    }
}
