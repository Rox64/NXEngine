#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <SDL.h>

#include "config.h"
#include "vjoy.h"
#include "input.h"
#include "graphics/graphics.h"
#include "game_modes.h"
#include "nx_math.h"
#include "settings.h"

#include "platform/platform.h"
#ifdef IPHONE
# include "platform/iOS/touch_control.h"
#endif

namespace VJoy
{
    void ignoreAllCurrentFingers();
}


// Helpers
namespace  {
    
    bool point_in(RectI const& rect, PointF const& p)
    {
        RectF r = RectF::fromRectI(rect);
        return r.point_in(p);
    }
    
}


const NXColor col_released(0xff, 0xcf, 0x33);
const NXColor col_pressed (0xff, 0x00, 0x00);

typedef std::map<SDL_FingerID, PointF> lastFingerPos_t;
lastFingerPos_t lastFingerPos;

typedef std::set<SDL_FingerID> ingnoredFinger_t;
ingnoredFinger_t ignoredFingers;

class VjoyMode
{
public:
    enum Mode
    {
        ETOUCH,
        EGESTURE,
        EBOTH
    };
    
    VjoyMode()
    {
        setMode(ETOUCH);
    }
    
    void setMode(Mode newmode)
    {
        if (newmode == mode)
            return;
        
        mode = newmode;
        
#ifdef CONFIG_USE_TAPS
        toggleGestureRecognizer(mode != ETOUCH);
        
        if (mode == EGESTURE)
        {
            lastFingerPos.clear();
        }
#endif // CONFIG_USE_TAPS
    }
    
    Mode getMode() const { return mode; }
    
    static Mode getModeFromSettings(Settings::Tap::Place place)
    {
        switch (settings->tap[place])
        {
            case Settings::Tap::ETAP:
                return EGESTURE;
            case Settings::Tap::EPAD:
                return ETOUCH;
            case Settings::Tap::EBOTH:
                return EBOTH;
        }
        
        assert(false);
        return EBOTH;
    }
    
private:
    Mode mode;
};

bool vjoy_enabled = true;
bool vjoy_visible = true;
VjoyMode vjoy_mode;



float xres = -1.0f;
float yres = -1.0f;


// VKeys
namespace VKeys {
    
    static const size_t presets_count = 2;
    
    static const VJoy::Preset presets[presets_count] =
    {
        // 0 - default
        {
            { // positions
                {/*0.7f*/-1.f, 0.8f, 0.1f, 0.1f}, // LEFTKEY
                {/*0.9f*/-1.f, 0.8f, 0.1f, 0.1f}, // RIGHTKEY
                {/*0.8f*/-1.f, 0.7f, 0.1f, 0.1f}, // UPKEY
                {/*0.8f*/-1.f, 0.9f, 0.1f, 0.1f}, // DOWNKEY
                
                {0.00f, 0.8f, 0.14f, 0.2f}, // JUMPKEY
                {0.15f, 0.8f, 0.14f, 0.2f}, // FIREKEY
                
                {0.00f, 0.55f, 0.1f, 0.1f}, // PREVWPNKEY
                {0.15f, 0.55f, 0.1f, 0.1f}, // NEXTWPNKEY
                
                {0.00f, 0.0f, 0.1f, 0.1f}, // INVENTORYKEY
                {0.15f, 0.0f, 0.1f, 0.1f}, // MAPSYSTEMKEY
                
                {0.40f, 0.0f, 0.1f, 0.1f}, // ESCKEY
                {0.55f, 0.0f, 0.1f, 0.1f}, // F1KEY
                {0.70f, 0.0f, 0.1f, 0.1f}, // F2KEY
                {0.85f, 0.0f, 0.1f, 0.1f}, // F3KEY
                {-1.f, -1.f, -1.f, -1.f}, // F4KEY
                {-1.f, -1.f, -1.f, -1.f}, // F5KEY
                {-1.f, -1.f, -1.f, -1.f}, // F6KEY
                {-1.f, -1.f, -1.f, -1.f}, // F7KEY
                {-1.f, -1.f, -1.f, -1.f}, // F8KEY
                {-1.f, -1.f, -1.f, -1.f}, // F9KEY
                {-1.f, -1.f, -1.f, -1.f}, // F10KEY
                {-1.f, -1.f, -1.f, -1.f}, // F11KEY
                {-1.f, -1.f, -1.f, -1.f}, // F12KEY
                
                {-1.f, -1.f, -1.f, -1.f}, // FREEZE_FRAME_KEY
                {-1.f, -1.f, -1.f, -1.f}, // FRAME_ADVANCE_KEY
                {-1.f, -1.f, -1.f, -1.f}  // DEBUG_FLY_KEY
            },
            PointF(0.82f, 0.82f),
            0.13f
        },
        // 1 - swap left-rigth
        {
            { // positions
                {/*0.7f*/-1.f, 0.8f, 0.1f, 0.1f}, // LEFTKEY
                {/*0.9f*/-1.f, 0.8f, 0.1f, 0.1f}, // RIGHTKEY
                {/*0.8f*/-1.f, 0.7f, 0.1f, 0.1f}, // UPKEY
                {/*0.8f*/-1.f, 0.9f, 0.1f, 0.1f}, // DOWNKEY
                
                {0.85f, 0.8f, 0.14f, 0.2f}, // JUMPKEY
                {0.70f, 0.8f, 0.14f, 0.2f}, // FIREKEY
                
                {0.74f, 0.55f, 0.1f, 0.1f}, // PREVWPNKEY
                {0.89f, 0.55f, 0.1f, 0.1f}, // NEXTWPNKEY
                
                {0.00f, 0.0f, 0.1f, 0.1f}, // INVENTORYKEY
                {0.15f, 0.0f, 0.1f, 0.1f}, // MAPSYSTEMKEY
                
                {0.40f, 0.0f, 0.1f, 0.1f}, // ESCKEY
                {0.55f, 0.0f, 0.1f, 0.1f}, // F1KEY
                {0.70f, 0.0f, 0.1f, 0.1f}, // F2KEY
                {0.85f, 0.0f, 0.1f, 0.1f}, // F3KEY
                {-1.f, -1.f, -1.f, -1.f}, // F4KEY
                {-1.f, -1.f, -1.f, -1.f}, // F5KEY
                {-1.f, -1.f, -1.f, -1.f}, // F6KEY
                {-1.f, -1.f, -1.f, -1.f}, // F7KEY
                {-1.f, -1.f, -1.f, -1.f}, // F8KEY
                {-1.f, -1.f, -1.f, -1.f}, // F9KEY
                {-1.f, -1.f, -1.f, -1.f}, // F10KEY
                {-1.f, -1.f, -1.f, -1.f}, // F11KEY
                {-1.f, -1.f, -1.f, -1.f}, // F12KEY
                
                {-1.f, -1.f, -1.f, -1.f}, // FREEZE_FRAME_KEY
                {-1.f, -1.f, -1.f, -1.f}, // FRAME_ADVANCE_KEY
                {-1.f, -1.f, -1.f, -1.f}  // DEBUG_FLY_KEY
            },
            PointF(0.19f, 0.82f),
            0.13f
        }
    };
    
namespace Pad
{
    bool enabled = false;
    SDL_FingerID finger;
    PointF origin;
    PointF current;
    
    const float border = 0.65f;
    const float max_r2 = 0.2*0.2;
    const float min_r2 = 0.02*0.02;
    
    const size_t seg_count = 8;
    TriF segments[seg_count];
    
    void init()
    {
        for (int i = 0, p = 7; i < seg_count; ++i, p += 2)
        {
#define F(p) ((p) % 16) - 8
            int rb = F(p);
            int rc = F(p + 2);
#undef F
            
            segments[i] = TriF(settings->vjoy_controls.pad_center, settings->vjoy_controls.pad_size, rb, rc);
        }
        
        //seg_size = preset.pad_size;
        //seg_center = preset.pad_center;
    }
    
    bool pressed[seg_count];
    
    void update_buttons(PointF const& p)
    {
        
        PointF vec = p - settings->vjoy_controls.pad_center;
        float r2 = vec.x * vec.x + vec.y * vec.y;
        if (r2 > settings->vjoy_controls.pad_size * settings->vjoy_controls.pad_size)
            return;
        
        // left
        inputs[0] = segments[3].in(p) || segments[4].in(p) || segments[5].in(p);
        // right
        inputs[1] = segments[7].in(p) || segments[0].in(p) || segments[1].in(p);
        // up
        inputs[2] = segments[5].in(p) || segments[6].in(p) || segments[7].in(p);
        // down
        inputs[3] = segments[1].in(p) || segments[2].in(p) || segments[3].in(p);
        
        for (int i = 0; i < seg_count; ++i)
        {
            pressed[i] = segments[i].in(p);
        }
    }
    
    void process()
    {
        if (!enabled)
            return;
        
        PointF vec = current - origin;
        float r2 = vec.x * vec.x + vec.y * vec.y;
        if (r2 < min_r2)
            return;
        // if (r2 > max_r2)
        //    r2 = max_r2;
        
        float t = atan2(vec.y, vec.x);
        
#define P(a) (float(double(a) * M_PI / 8.0))
#define RANGE(a, b) (P(a) <= t && t <= P(b))
        
        inputs[0] = (RANGE(-8, -5) || RANGE(5, 8));  // left
        inputs[1] = (RANGE(-3, 0) || RANGE(0, 3));   // rigth
        inputs[2] = (RANGE(-7, -1));                 // up
        inputs[3] = (RANGE(1, 7));                   // down
        
#undef RANGE
#undef P
        
    }
    
    void draw()
    {
        PointF const& a = settings->vjoy_controls.pad_center;
        for (size_t i = 0; i < seg_count; ++i)
        {
            PointF const& b = segments[i].b;
            PointF const& c = segments[i].c;
            
            NXColor const& color = pressed[i] ? col_pressed : col_released;
            
            Graphics::DrawLine(a.x * Graphics::SCREEN_WIDTH, a.y * Graphics::SCREEN_HEIGHT,
                               b.x * Graphics::SCREEN_WIDTH, b.y * Graphics::SCREEN_HEIGHT, color);
            Graphics::DrawLine(b.x * Graphics::SCREEN_WIDTH, b.y * Graphics::SCREEN_HEIGHT,
                               c.x * Graphics::SCREEN_WIDTH, c.y * Graphics::SCREEN_HEIGHT, color);
        }
    }
} // namespace Pad
    
    void init()
    {
        Pad::init();
    }
    
    void update_buttons(PointF const& p)
    {
        for (int i = 0; i < INPUT_COUNT; ++i)
        {
            if (settings->vjoy_controls.positions[i].x < 0)
                continue;
            
            if (settings->vjoy_controls.positions[i].point_in(p))
                inputs[i] = true;
        }
        
        Pad::update_buttons(p);
    }
    
    void draw()
    {
        for (int i = 0; i < INPUT_COUNT; ++i)
        {
            RectF const& vkey = settings->vjoy_controls.positions[i];
            
            if (vkey.x < 0)
                continue;
            
            NXColor const& c = inputs[i] ? col_pressed : col_released;
            vkey.draw_thin_rect(c);
        }
        
        Pad::draw();
    }
    
} // namespace Vkeys


class GestureObserver : public IGestureObserver
{
    typedef std::vector<PointF> tapLocation_t;
public:
    virtual void tap(float x, float y)
    {
        taps.push_back(PointF(x, y));
    }
    
public:
    
    bool wasTap(RectF const& rect)
    {
        for (tapLocation_t::const_iterator it = taps.begin(); it != taps.end(); ++it)
        {
            if (rect.point_in(*it))
                return true;
        }
        
        return false;
    }
    
    bool wasTap()
    {
        return !taps.empty();
    }
    
    void flushEvents()
    {
        taps.clear();
    }
    
private:
    
    tapLocation_t taps;
};

static GestureObserver gestureObserver;

namespace VJoy {
namespace ModeAware
{
    
    struct DefaultControl
    {
        DefaultControl() : disable_draw(false) {}
        virtual void on_enter() {}
        virtual void update_buttons(PointF const& p)
        {
            if (VjoyMode::EGESTURE == vjoy_mode.getMode())
                return;
            
            VKeys::update_buttons(p);
        }
        virtual void draw()
        {
            if (VjoyMode::EGESTURE == vjoy_mode.getMode())
                return;
            
            VKeys::draw();
        }
        
        bool disable_draw;
        
        virtual ~DefaultControl() {}
    };
 
    DefaultControl* const* ppads = NULL;
    
    class NoneModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::ETOUCH);
        }
    };    
    class NormalModePad : public DefaultControl
    { 
    public:
        bool textbox_mode;

        NormalModePad() : textbox_mode(false) {}
        
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::ETOUCH);
        }
        
        virtual void update_buttons(PointF const& p)
        {
            if (textbox_mode)
            {
                // This is used to speed up text in textboxes
                inputs[FIREKEY] = true;
            }
            else
            {
                DefaultControl::update_buttons(p);
            }
        }
        
//        virtual void draw()
//        {
//            DefaultControl::draw();
//        }
    };
    class InventoryModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EInventory));
        }
    };
    class MapSystemModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EMapSystem));
        }
    };
    class IslandModePad : public DefaultControl
    {
        
    };
    class CreditsModePad : public DefaultControl
    {
        
    };
    class IntroModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EMovies));
        }
    };
    class TitleModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::ETitle));
        }
    };
    class PausedModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EPause));
        }
        
        virtual void update_buttons(PointF const& p)
        {
            ppads[GM_NORMAL]->update_buttons(p);
        }
        
        virtual void draw()
        {
            ppads[GM_NORMAL]->draw();
        }
    };
    class OptionsModePad : public DefaultControl
    {
        virtual void on_enter()
        {
            vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EOptions));
        }
        
        virtual void update_buttons(PointF const& p)
        {
            ppads[GM_NORMAL]->update_buttons(p);
        }
        
        virtual void draw()
        {
            ppads[GM_NORMAL]->draw();
        }
    };
    
    
    NoneModePad noneModePad;
    NormalModePad normalModePad;
    InventoryModePad inventoryModePad;
    MapSystemModePad mapSystemModePad;
    IslandModePad islandModePad;
    CreditsModePad creditsModePad;
    IntroModePad introModePad;
    TitleModePad titleModePad;
    PausedModePad pausedModePad;
    OptionsModePad optionsModePad;    
    
    DefaultControl* const pads[NUM_GAMEMODES] =
    {
        &noneModePad,
        &normalModePad,
        &inventoryModePad,
        &mapSystemModePad,
        &islandModePad,
        &creditsModePad,
        &introModePad,
        &titleModePad,
        &pausedModePad,
        &optionsModePad
    };
    
    static void dispatch(PointF const& p)
    {
        pads[getGamemode()]->update_buttons(p);
    }
    
    static void draw()
    {
        if (!pads[getGamemode()]->disable_draw)
            pads[getGamemode()]->draw();
    }

    bool wasTap(RectI rect)
    {
        if (VjoyMode::ETOUCH == vjoy_mode.getMode())
            return false;
        
        return gestureObserver.wasTap(RectF::fromRectI(rect));
    }
    
    bool wasTap()
    {
        if (VjoyMode::ETOUCH == vjoy_mode.getMode())
            return false;
        
        return gestureObserver.wasTap();
    }
    
    void gameModeChanged(int newMode)
    {
        ppads = pads;
        pads[newMode]->on_enter();
        
        ignoreAllCurrentFingers();
    }
    
    void specScreenChanged(SpecScreens newScreen, bool enter)
    {
        ignoreAllCurrentFingers();
        
        {
            struct state_t
            {
                VjoyMode::Mode mode;
                GameModes gm;
                bool gm_draw;
                bool normal_textbox_mode;
                
                void push()
                {
                    mode = vjoy_mode.getMode();
                    gm = getGamemode();
                    gm_draw = pads[gm]->disable_draw;
                    normal_textbox_mode = static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode;
                }
                
                void pop()
                {
                    vjoy_mode.setMode(mode);
                    pads[gm]->disable_draw = gm_draw;
                    static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = normal_textbox_mode;
                }
            };
            
            static std::stack<state_t> states;
        
        
            state_t s;
            if (enter)
            {
                s.push();
                states.push(s);
            }
            else
            {
                if (states.empty())
                    return;
                s = states.top();
                s.pop();
                states.pop();
                return;
            }
        }
        
        switch (newScreen) {
            case ETextBox:
            {
                vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EIngameDialog));
                static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = (vjoy_mode.getMode() != VjoyMode::ETOUCH);
                break;
            }
            case ESaveLoad:
            {
                vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::ESaveLoad));
                static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = false;
                break;
            }
            case EYesNo:
            {
                vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EIngameDialog));
                static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = false;
                break;
            }
            case EStageSelect1:
            {
                vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EIngameDialog));
                static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = false;
                break;
            }
            case EStageSelect2:
            {
                vjoy_mode.setMode(VjoyMode::getModeFromSettings(Settings::Tap::EIngameDialog));
                static_cast<NormalModePad*>(pads[GM_NORMAL])->textbox_mode = (vjoy_mode.getMode() != VjoyMode::ETOUCH);
                break;
            }
        }
        
    }
    
} // namespace ModeAware


    
Preset const& getPreset(size_t num)
{
    if (num >= VKeys::presets_count)
        num = 0;
    
    return VKeys::presets[num];
}

size_t VJoy::getPresetsCount()
{
    return VKeys::presets_count;
}

void setFromPreset(size_t num)
{
    settings->vjoy_controls = getPreset(num);
    settings->vjoy_current_preset = num;
    setUpdated();
}
    
void setUpdated()
{
    VKeys::init();
}

bool  Init()
{
    vjoy_enabled = true;
    registerGetureObserver(&gestureObserver);
    return true;
}

void Destroy()
{
    vjoy_enabled = false;
}

void DrawAll()
{
    if (!(vjoy_enabled && vjoy_visible))
        return;
    
    ModeAware::draw();
    
    
    
    for (lastFingerPos_t::const_iterator it = lastFingerPos.begin(); it != lastFingerPos.end(); ++it)
    {
        PointF const& p = it->second;
        RectF r = RectF::centred(p, 0.04f, 0.04f);
        
        const NXColor col(0xff, 0xcf, 0x33);
        r.draw_fill_rect(col);
    }
}

void InjectInputEvent(SDL_Event const & evt)
{
    if (!vjoy_enabled)
        return;
    
    if (evt.type == SDL_FINGERUP)
    {
        lastFingerPos_t::iterator it = lastFingerPos.find(evt.tfinger.fingerId);
        if (it != lastFingerPos.end())
        {
            lastFingerPos.erase(it);
        }
        
        ignoredFingers.erase(evt.tfinger.fingerId);
        
        return;
    }
    
    if (xres < 0)
    {
        SDL_Touch const * const state = SDL_GetTouch(evt.tfinger.touchId);
        if (state)
        {
            xres = state->xres;
            yres = state->yres;
        }
        else
        {
            return;
        }
    }
    
    PointF p((float)evt.tfinger.x / xres, (float)evt.tfinger.y / yres);
    
    
    if (ignoredFingers.end() == ignoredFingers.find(evt.tfinger.fingerId))
        lastFingerPos[evt.tfinger.fingerId] = p;
}

void PreProcessInput()
{
    if (!vjoy_enabled)
        return;
    
    gestureObserver.flushEvents();
}

void ProcessInput()
{
    if (!vjoy_enabled)
        return;
    
    memset(inputs, 0, sizeof(inputs));
    memset(VKeys::Pad::pressed, 0, sizeof(VKeys::Pad::pressed));
    
    for (lastFingerPos_t::const_iterator it = lastFingerPos.begin(); it != lastFingerPos.end(); ++it)
    {
        PointF const& p = it->second;
        ModeAware::dispatch(p);
    }
}

void ignoreAllCurrentFingers()
{
    for (lastFingerPos_t::const_iterator it = lastFingerPos.begin(); it != lastFingerPos.end(); ++it)
    {
        ignoredFingers.insert(it->first);
    }
    
    lastFingerPos.clear();
}

} // namespace VJoy
