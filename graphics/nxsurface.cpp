#include <cassert>
#include <string.h>
#include <stdint.h>
#include "../settings.h"
#include "../config.h"
#include "graphics.h"
#include "nxsurface.h"
#include "nxsurface.fdh"
#include "../platform/platform.h"


#ifdef CONFIG_MUTABLE_SCALE
	int SCALE = 3;
#endif

extern SDL_Renderer * renderer;


NXSurface::NXSurface() :
	fTexture(NULL),
	tex_w(0),
	tex_h(0),
	tex_format()
{
}


NXSurface::NXSurface(int wd, int ht, NXFormat *format) :
	fTexture(NULL),
	tex_w(0),
	tex_h(0),
	tex_format()
{
	AllocNew(wd, ht, format);
	setFormat(format);
}

NXSurface* NXSurface::createScreen(int wd, int ht, Uint32 pixel_format)
{
	NXSurface* s = new NXSurface();
	s->tex_w = wd;
	s->tex_h = ht;
	s->setPixelFormat(pixel_format);
	return s;
}

NXSurface::~NXSurface()
{
	Free();
}

// static function, and requires a reload of all surfaces
void NXSurface::SetScale(int factor)
{
	#ifdef CONFIG_MUTABLE_SCALE
		SCALE = factor;
	#else
		staterr("NXSurface::SetScale: CONFIG_MUTABLE_SCALE not set");
	#endif
}

/*
void c------------------------------() {}
*/

// allocate for an empty surface of the given size
bool NXSurface::AllocNew(int wd, int ht, NXFormat* format)
{
	Free();

	stat("NXSurface::AllocNew this = %p", this);

	fTexture = SDL_CreateTexture(renderer, format->format, SDL_TEXTUREACCESS_TARGET, wd*SCALE, ht*SCALE);
	
	if (!fTexture)
	{
		staterr("NXSurface::AllocNew: failed to allocate texture: %s", SDL_GetError());
		return true;
	}

	tex_w = wd*SCALE;
	tex_h = ht*SCALE;
	
	return false;
}


// load the surface from a .pbm or bitmap file
bool NXSurface::LoadImage(const char *pbm_name, bool use_colorkey, int use_display_format)
{
	stat("NXSurface::LoadImage name = %s, this = %p", pbm_name, this);
	SDL_Surface *image;

	Free();
	
	// if (use_display_format == -1)
	// {	// use value specified in settings
	// 	use_display_format = settings->displayformat;
	// }
	
	image = SDL_LoadBMP_RW(SDL_RWFromFP(fileopenRO(pbm_name), SDL_TRUE), 1);
	if (!image)
	{
		staterr("NXSurface::LoadImage: load failed of '%s'!", pbm_name);
		return 1;
	}
	
	if (use_colorkey)
	{
		SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0, 0, 0));
	}

	SDL_Texture * tmptex = SDL_CreateTextureFromSurface(renderer, image);
	if (!tmptex)
	{
		staterr("NXSurface::LoadImage: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
		SDL_FreeSurface(image);
		return 1;
	}

	SDL_FreeSurface(image);

	{
		int wd, ht, access;
		Uint32 format;
		NXFormat nxformat;
		if (SDL_QueryTexture(tmptex, &format, &access, &wd, &ht)) goto error;
		nxformat.format = format;
		if (AllocNew(wd, ht, &nxformat)) goto error;
		if (SDL_SetTextureBlendMode(tmptex, SDL_BLENDMODE_MOD))	goto error;
		if (SDL_SetRenderTarget(renderer, fTexture)) goto error;
		if (SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)) goto error;
		if (SDL_RenderClear(renderer)) goto error;
		if (SDL_RenderCopy(renderer, tmptex, NULL, NULL)) goto error;
		if (SDL_SetRenderTarget(renderer, NULL)) goto error;
		if (SDL_SetTextureBlendMode(fTexture, SDL_BLENDMODE_BLEND)) goto error;

		SDL_DestroyTexture(tmptex);

		goto done;
error:
		{
			staterr("NXSurface::LoadImage failed: %s", SDL_GetError());
			if (tmptex)  { SDL_DestroyTexture(tmptex); tmptex = NULL; }
			if (fTexture){ SDL_DestroyTexture(fTexture); fTexture = NULL; }
			SDL_SetRenderTarget(renderer, NULL);
		}
done:
		;
	}

	stat("NXSurface::LoadImage name = %s, this = %p done", pbm_name, this);

	return (fTexture == NULL);
}


NXSurface *NXSurface::FromFile(const char *pbm_name, bool use_colorkey, int use_display_format)
{
	NXSurface *sfc = new NXSurface;
	if (sfc->LoadImage(pbm_name, use_colorkey, use_display_format))
	{
		delete sfc;
		return NULL;
	}
	
	return sfc;
}


/*
void c------------------------------() {}
*/

// draw some or all of another surface onto this surface.
void NXSurface::DrawSurface(NXSurface *src, \
							int dstx, int dsty, int srcx, int srcy, int wd, int ht)
{
	if (this != screen)
		SetAsTarget(true);

	assert(renderer);
	assert(src->fTexture);

	SDL_Rect srcrect, dstrect;

	srcrect.x = srcx * SCALE;
	srcrect.y = srcy * SCALE;
	srcrect.w = wd * SCALE;
	srcrect.h = ht * SCALE;
	
	dstrect.x = dstx * SCALE;
	dstrect.y = dsty * SCALE;
	dstrect.w = srcrect.w;
	dstrect.h = srcrect.h;
	
	if (SDL_RenderCopy(renderer, src->fTexture, &srcrect, &dstrect))
	{
		staterr("NXSurface::DrawSurface: SDL_RenderCopy failed: %s", SDL_GetError());
	}

	if (this != screen)
		SetAsTarget(false);
}

void NXSurface::DrawSurface(NXSurface *src, int dstx, int dsty)
{
	DrawSurface(src, dstx, dsty, 0, 0, src->Width(), src->Height());
}

// draw the given source surface in a repeating pattern across the entire width of the surface.
// x_dst: an starting X with which to offset the pattern horizontally (usually negative).
// y_dst: the Y coordinate to copy to on the destination.
// y_src: the Y coordinate to copy from.
// height: the number of pixels tall to copy.
void NXSurface::BlitPatternAcross(NXSurface *src,
						   int x_dst, int y_dst, int y_src, int height)
{
	if (this != screen)
		SetAsTarget(true);

	SDL_Rect srcrect, dstrect;

	srcrect.x = 0;
	srcrect.w = src->tex_w;
	srcrect.y = (y_src * SCALE);
	srcrect.h = (height * SCALE);

	dstrect.w = srcrect.w;
	dstrect.h = srcrect.h;

	int x = (x_dst * SCALE);
	int y = (y_dst * SCALE);
	int destwd = this->tex_w;
	
	do
	{
		dstrect.x = x;
		dstrect.y = y;
		
		SDL_RenderCopy(renderer, src->fTexture, &srcrect, &dstrect);
		x += src->tex_w;
	}
	while(x < destwd);

	if (this != screen)
		SetAsTarget(false);
}


/*
void c------------------------------() {}
*/

void NXSurface::DrawLine(int x1, int y1, int x2, int y2, NXColor color)
{
	if (this != screen)
		SetAsTarget(true);

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, x1 * SCALE, y1 * SCALE, x2 * SCALE, y2 * SCALE);

	if (this != screen)
		SetAsTarget(false);	
}

void NXSurface::DrawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	if (this != screen)
		SetAsTarget(true);

	SDL_Rect rects[4] = {
		{x1 * SCALE, y1 * SCALE, ((x2 - x1) + 1) * SCALE, SCALE},
		{x1 * SCALE, y2 * SCALE, ((x2 - x1) + 1) * SCALE, SCALE},
		{x1 * SCALE, y1 * SCALE, SCALE,                   ((y2 - y1) + 1) * SCALE},
		{x2 * SCALE, y1 * SCALE, SCALE,                   ((y2 - y1) + 1) * SCALE}
	};

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRects(renderer, rects, 4);

	if (this != screen)
		SetAsTarget(false);
}


void NXSurface::FillRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	if (this != screen)
		SetAsTarget(true);

	SDL_Rect rect;

	rect.x = x1 * SCALE;
	rect.y = y1 * SCALE;
	rect.w = ((x2 - x1) + 1) * SCALE;
	rect.h = ((y2 - y1) + 1) * SCALE;
	
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	if (this != screen)
		SetAsTarget(false);
}

void NXSurface::Clear(uint8_t r, uint8_t g, uint8_t b)
{
	if (this != screen)
		SetAsTarget(true);

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	//SDL_RenderFillRect(renderer, NULL);
	SDL_RenderClear(renderer);

	if (this != screen)
		SetAsTarget(false);
}


void NXSurface::DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	FillRect(x, y, x, y, r, g, b);
}

/*
void c------------------------------() {}
*/

int NXSurface::Width()
{
	return tex_w / SCALE;
}

int NXSurface::Height()
{
	return tex_h / SCALE;
}

NXFormat *NXSurface::Format()
{
	return &tex_format;
}

void NXSurface::Flip()
{
	if (this == screen)
	{
		SDL_RenderPresent(renderer);
	}
}

/*
void c------------------------------() {}
*/

void NXSurface::set_clip_rect(int x, int y, int w, int h)
{
	NXRect rect(x * SCALE, y * SCALE, w * SCALE, h * SCALE);
	// SDL_SetClipRect(fSurface, &rect);
}

void NXSurface::set_clip_rect(NXRect *rect)
{
	// SDL_SetClipRect(fSurface, rect);
}

void NXSurface::clear_clip_rect()
{
	// SDL_SetClipRect(fSurface, NULL);
}

/*
void c------------------------------() {}
*/


/*
void c------------------------------() {}
*/

// void NXSurface::EnableColorKey()
// {
// 	SDL_SetColorKey(fSurface, SDL_TRUE, SDL_MapRGB(fSurface->format, 0, 0, 0));
// }

// uint32_t NXSurface::MapColor(uint8_t r, uint8_t g, uint8_t b)
// {
// 	return SDL_MapRGB(fSurface->format, r, g, b);
// }


void NXSurface::Free()
{
	if (fTexture)
	{
		SDL_DestroyTexture(fTexture);
		fTexture = NULL;
	}
}



void NXSurface::SetAsTarget(bool enabled)
{
	stat("NXSurface::SetAsTarget this = %p, enabled = %d", this, (int)enabled);

	if (SDL_SetRenderTarget(renderer, (enabled ? fTexture : NULL)))
	{
		staterr("NXSurface::SetAsTarget: SDL_SetRenderTarget failed: %s" , SDL_GetError());
	}
}

void NXSurface::setFormat(NXFormat const* format)
{
	tex_format = *format;
	tex_format.palette = NULL;
}

void NXSurface::setPixelFormat(Uint32 format)
{
	tex_format.format = format;
}
