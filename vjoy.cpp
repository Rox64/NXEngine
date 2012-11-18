#include <cmath>
#include <map>
#include <vector>
#include <SDL.h>

#include "vjoy.h"
#include "input.h"
#include "graphics/graphics.h"

struct Point
{
    float x, y;
    Point(float x, float y) : x(x), y(y) {}
    Point() : x(0), y(0) {}
    
    Point operator+(Point const& r) const
    {
        return Point(x + r.x, y + r.y);
    }
    
    Point operator-(Point const& r) const
    {
        return Point(x - r.x, y - r.y);
    }
    
    Point operator*(float k) const
    {
        return Point(k * x, k * y);
    }
};

struct Rect
{
    float x, y;
    float w, h;
    
    static Rect centred(Point const& p, float w, float h)
    {
        Rect r = {p.x - w/2, p.y - h/2, w, h};
        return r;
    }
    
    bool point_in(Point const& p) const
    {
        return point_in(p.x, p.y);
    }
    
    bool point_in(float px, float py) const
    {
        return !(px < x || x + w < px || py < y || y + h < py);
    }
    
    void to_screen_coord(int& x1, int& y1, int& x2, int& y2) const
    {
        x1 = Graphics::SCREEN_WIDTH  *  x;
        y1 = Graphics::SCREEN_HEIGHT *  y;
        x2 = Graphics::SCREEN_WIDTH  * (x + w);
        y2 = Graphics::SCREEN_HEIGHT * (y + h);
    }
    
    void draw_fill_rect(NXColor const& c) const
    {
        int x1, y1, x2, y2;
        to_screen_coord(x1, y1, x2, y2);
        Graphics::FillRect(x1, y1, x2, y2, c);
    }
    
    void draw_thick_rect(NXColor const& c) const
    {
        int x1, y1, x2, y2;
        to_screen_coord(x1, y1, x2, y2);
        Graphics::DrawRect(x1, y1, x2, y2, c);
    }
    
    void draw_thin_rect(NXColor const& c) const
    {
        int x1, y1, x2, y2;
        to_screen_coord(x1, y1, x2, y2);
        Graphics::DrawLine(x1, y1, x2, y1, c);
        Graphics::DrawLine(x1, y2, x2, y2, c);
        Graphics::DrawLine(x1, y1, x1, y2, c);
        Graphics::DrawLine(x2, y1, x2, y2, c);
    }
};

const NXColor col_released(0xff, 0xcf, 0x33);
const NXColor col_pressed (0xff, 0x00, 0x00);

const Rect vkeys[INPUT_COUNT] =
{
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
    {-1.f, -1.f, -1.f, -1.f}, // F2KEY
    {-1.f, -1.f, -1.f, -1.f}, // F3KEY
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
};


bool vjoy_enabled = true;
bool vjoy_visible = true;

float xres = -1.0f;
float yres = -1.0f;

typedef std::map<SDL_FingerID, Point> lastFingerPos_t;
lastFingerPos_t lastFingerPos;

namespace Pad
{
    bool enabled = false;
    SDL_FingerID finger;
    Point origin;
    Point current;
    
    const float border = 0.65f;
    const float max_r2 = 0.2*0.2;
    const float min_r2 = 0.02*0.02;
    
    struct Tri
    {
        Point a;
        Point b, c;
        
        Tri(Point const& a, float size, float rb, float rc) :
        a(a)
        {
#define P(a) (double(a) * M_PI / 8.0)
            b = Point(cos(P(rb)), sin(P(rb))) * size + a;
            c = Point(cos(P(rc)), sin(P(rc))) * size + a;
#undef P
        }
        
        static float sign(Point const& p1, Point const& p2, Point const& p3)
        {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        }
        
        bool in(Point const& pt) const
        {
            bool b1, b2, b3;
            
            b1 = sign(pt, a, b) < 0.0f;
            b2 = sign(pt, b, c) < 0.0f;
            b3 = sign(pt, c, a) < 0.0f;
            
            return ((b1 == b2) && (b2 == b3));
        }
    };
    
    const float seg_size = 0.13f;
    const Point seg_center(0.82f, 0.82f);
    
    const size_t seg_count = 8;
    Tri segments[seg_count] = {
        Tri(seg_center, seg_size, -1, 1),
        Tri(seg_center, seg_size, 1, 3),
        Tri(seg_center, seg_size, 3, 5),
        Tri(seg_center, seg_size, 5, 7),
        Tri(seg_center, seg_size, 7, -7),
        Tri(seg_center, seg_size, -7, -5),
        Tri(seg_center, seg_size, -5, -3),
        Tri(seg_center, seg_size, -3, -1)
    };
    
    bool pressed[seg_count];
    
    void insert_event(SDL_Event const& evt, Point const& p)
    {
        // if (evt.type == SDL_FINGERUP && Pad::enabled && evt.tfinger.fingerId == finger)
        // {
        //    if (enabled && evt.tfinger.fingerId == Pad::finger)
        //    {
        //       enabled = false;
        //    }
        // }
        // else
        // {
        //    if (!enabled && evt.type == SDL_FINGERDOWN && p.x > border)
        //    {
        //       enabled = true;
        //       origin = p;
        //       finger = evt.tfinger.fingerId;
        //    }
        
        //    if (enabled && evt.tfinger.fingerId == finger)
        //    {
        //       current = p;
        //    }
        // }
    }
    
    void update_buttons(Point const& p)
    {
        Point vec = p - seg_center;
        float r2 = vec.x * vec.x + vec.y * vec.y;
        if (r2 > seg_size*seg_size)
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
        
        Point vec = current - origin;
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
        Point const& a = seg_center;
        for (size_t i = 0; i < seg_count; ++i)
        {
            Point const& b = segments[i].b;
            Point const& c = segments[i].c;
            
            NXColor const& color = pressed[i] ? col_pressed : col_released;
            
            Graphics::DrawLine(a.x * Graphics::SCREEN_WIDTH, a.y * Graphics::SCREEN_HEIGHT,
                               b.x * Graphics::SCREEN_WIDTH, b.y * Graphics::SCREEN_HEIGHT, color);
            Graphics::DrawLine(b.x * Graphics::SCREEN_WIDTH, b.y * Graphics::SCREEN_HEIGHT,
                               c.x * Graphics::SCREEN_WIDTH, c.y * Graphics::SCREEN_HEIGHT, color);
        }
        
        // if (!enabled)
        //    return;
        
        // const NXColor origin_color(0xff, 0xcf, 0x33);
        // const NXColor current_color(0x89, 0xf5, 0x25);
        
        // const float size = 0.05f;
        
        // int x1, y1, x2, y2;
        // Rect o = {origin.x - size / 2, origin.y - size/2, size, size};
        // Rect c = {current.x - size / 2, current.y - size/2, size, size};
        
        // o.to_screen_coord(x1, y1, x2, y2);
        // Graphics::FillRect(x1, y1, x2, y2, origin_color);
        
        // c.to_screen_coord(x1, y1, x2, y2);
        // Graphics::FillRect(x1, y1, x2, y2, current_color);
    }
};

bool  VJoy::Init()
{
    vjoy_enabled = true;
    return true;
}

void VJoy::Destroy()
{
    vjoy_enabled = false;
}



void VJoy::DrawAll()
{
    if (!(vjoy_enabled && vjoy_visible))
        return;
    
    for (int i = 0; i < INPUT_COUNT; ++i)
    {
        Rect const& vkey = vkeys[i];
        
        if (vkey.x < 0)
            continue;
        
        NXColor const& c = inputs[i] ? col_pressed : col_released;
        vkey.draw_thin_rect(c);
    }
    
    Pad::draw();
    
    for (lastFingerPos_t::const_iterator it = lastFingerPos.begin(); it != lastFingerPos.end(); ++it)
    {
        Point const& p = it->second;
        Rect r = Rect::centred(p, 0.04f, 0.04f);
        
        const NXColor col(0xff, 0xcf, 0x33);
        r.draw_fill_rect(col);
    }
}

void VJoy::InjectInputEvent(SDL_Event const & evt)
{
    if (!vjoy_enabled)
        return;
    
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
    
    Point p((float)evt.tfinger.x / xres, (float)evt.tfinger.y / yres);
    
    
    if (evt.type == SDL_FINGERUP)
    {
        lastFingerPos_t::iterator it = lastFingerPos.find(evt.tfinger.fingerId);
        if (it != lastFingerPos.end())
        {
            lastFingerPos.erase(it);
        }
    }
    else
    {
        lastFingerPos[evt.tfinger.fingerId] = p;
    }
    
    Pad::insert_event(evt, p);
}

void updateButtons(Point const& p, bool state)
{
    for (int i = 0; i < INPUT_COUNT; ++i)
    {
        if (vkeys[i].x < 0)
            continue;
        
        if (vkeys[i].point_in(p))
            inputs[i] = state;
    }
}



void VJoy::ProcessInput()
{
    if (!vjoy_enabled)
        return;
    
    memset(inputs, 0, sizeof(inputs));
    memset(Pad::pressed, 0, sizeof(Pad::pressed));
    
    for (lastFingerPos_t::const_iterator it = lastFingerPos.begin(); it != lastFingerPos.end(); ++it)
    {
        Point const& p = it->second;
        updateButtons(p, true);
        Pad::update_buttons(p);
    }
}