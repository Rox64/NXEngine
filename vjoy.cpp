#include <SDL.h>

#include "vjoy.h"
#include "input.h"
#include "graphics/graphics.h"

struct Rect
{
   float x, y;
   float w, h;

   bool point_in(float px, float py) const
   {
      return !(px < x || x + w < px || py < y || y + h < py);
   }
};

const Rect vkeys[INPUT_COUNT] = 
{
   {0.7f, 0.8f, 0.1f, 0.1f}, // LEFTKEY
   {0.9f, 0.8f, 0.1f, 0.1f}, // RIGHTKEY
   {0.8f, 0.7f, 0.1f, 0.1f}, // UPKEY
   {0.8f, 0.9f, 0.1f, 0.1f}, // DOWNKEY

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

   const NXColor pressed(0, 0, 0x21);
   const NXColor released(0, 0x21, 0);


   for (int i = 0; i < INPUT_COUNT; ++i)
   {
      Rect const& vkey = vkeys[i];

      if (vkey.x < 0)
         continue;

      int x1 = SCREEN_WIDTH  *  vkey.x;
      int y1 = SCREEN_HEIGHT *  vkey.y;
      int x2 = SCREEN_WIDTH  * (vkey.x + vkey.w);
      int y2 = SCREEN_HEIGHT * (vkey.y + vkey.h);

      NXColor const& c = inputs[i] ? pressed : released; 

      Graphics::FillRect(x1, y1, x2, y2, c);
   }
}

void VJoy::ProcessInput(SDL_Event const & evt)
{
   if (!vjoy_enabled)
      return;

   SDL_Touch const * const state = SDL_GetTouch(evt.tfinger.touchId);
   float x = (float)evt.tfinger.x / state->xres;
   float y = (float)evt.tfinger.y / state->yres;

   for (int i = 0; i < INPUT_COUNT; ++i)
   {
      if (vkeys[i].x < 0)
         continue;


      inputs[i] = vkeys[i].point_in(x, y) &&
         (evt.type == SDL_FINGERDOWN || evt.type == SDL_FINGERMOTION);

   }
}