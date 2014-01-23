#include <cassert>

#include "../hacks_internal.hpp"

#include "../../../config.h"
#include "../../../common/basics.h"

#include "SDL_opengl.h"

#include "glfuncs.h"

using namespace GraphicHacks;


GL_Functions funcs;

/////////////////
	// from sdl
	typedef struct
	{
	    GLuint texture;
	    GLenum type;
	    GLfloat texw;
	    GLfloat texh;
	    GLenum format;
	    GLenum formattype;
	    void *pixels;
	    int pitch;
	    SDL_Rect locked_rect;

	    /* YV12 texture support */
	    SDL_bool yuv;
	    GLuint utexture;
	    GLuint vtexture;
	    
		typedef void GL_FBOList;

	    GL_FBOList *fbo;
	} GL_TextureData;

//////////////

template <typename T>
static T nextPowerOfTwo(T n)
{
	size_t k=1;
	n--;
	do {
		n |= n >> k ;
		k <<=1;
	}
	while (k < sizeof(T)*8);
	return ++n;
}

struct hack_gl_t : public Hack
{
	virtual char const* name() const { return "opengl"; }



	virtual bool Init(SDL_Renderer * renderer)
	{
		if (LoadFunctions(&funcs))
			return true;
		
		return false;
	}

	struct Coords
	{
		struct XY
		{
			GLfloat x;
			GLfloat y;

			XY() : x(0), y(0) {}
			XY(GLfloat x, GLfloat y) : x(x), y(y) {}
		};
		
		XY xy[4];

		enum { SIZE = 4 };
	};

	struct QuadToRender
	{
		Coords* vert;
		Coords* uv;

		size_t count;
		size_t max_count;

		bool auto_realloc;

		QuadToRender() : 
			vert(NULL),
			uv(NULL),
			count(0),
			max_count(0),
			auto_realloc(false)
		{}

		~QuadToRender() {
			dealloc();
		}

		void dealloc()
		{
			delete [] vert;
			delete [] uv;

			vert = NULL;
			uv = NULL;

			count = 0;
			max_count = 0;
		}

		bool alloc(size_t new_max_count, bool realloc)
		{            
			if (max_count >= new_max_count)
				return false;

			if (!realloc)
				dealloc();

			new_max_count = nextPowerOfTwo(new_max_count);

			Coords* nuv = new Coords[new_max_count];
			Coords* nvert = new Coords[new_max_count];

			if (! (nuv && nvert))
				return true;

			if (realloc)
			{
				memcpy(nuv,   uv,   sizeof(uv[0])   * count);
				memcpy(nvert, vert, sizeof(vert[0]) * count);

				size_t old_count = count;
				dealloc();
				
				count = old_count;
				uv = nuv;
				vert = nvert;
			}
			else
			{
				uv = nuv;
				vert = nvert;
			}

			max_count = new_max_count;

			return false;
		}

		bool check(size_t needed)
		{
			if (count + needed >= max_count)
			{
				if (auto_realloc)
					return alloc(count + needed, true);
				else
					return true;
			}
			
			return false;
		}

	} quads;


	GL_TextureData* texInfo;

	virtual bool BatchBegin(SDL_Renderer * renderer, size_t max_count)
	{
		texInfo = NULL;
		
		quads.count = 0;
		
		if (0 == max_count)
		{
			quads.auto_realloc = true;
			return false;
		}

		quads.auto_realloc = false;
		if (quads.alloc(max_count, false))
			return true;

		return false;
	}

	virtual bool BatchAddCopy(SDL_Renderer * renderer, SDL_Texture * texture,
	                          const SDL_Rect * srcrect, const SDL_FRect * dstrect)
	{
		if (quads.check(1))
		{
			staterr("no enough memory for batch or batch operation is bigger than have to be");
			assert(false);
			return true;
		}


		//GL_RenderData *data = (GL_RenderData *) renderer->driverdata;
		GL_TextureData *texturedata = (GL_TextureData *) texture->driverdata;
		GLfloat minx, miny, maxx, maxy;
		GLfloat minu, maxu, minv, maxv;

		if (texInfo)
		{
			if (texInfo != texturedata)
			{
				staterr("batch operation have to be with the same texture");
				assert(false);
				return true;
			}
		}
		else
		{
			texInfo = texturedata;
		}
		
		//GL_ActivateRenderer(renderer);

		// /*data->*/glEnable(texturedata->type);
		// if (texturedata->yuv) {
		//     /*data->*/glActiveTextureARB(GL_TEXTURE2_ARB);
		//     /*data->*/glBindTexture(texturedata->type, texturedata->vtexture);

		//     /*data->*/glActiveTextureARB(GL_TEXTURE1_ARB);
		//     /*data->*/glBindTexture(texturedata->type, texturedata->utexture);

		//     /*data->*/glActiveTextureARB(GL_TEXTURE0_ARB);
		// }
		// /*data->*/glBindTexture(texturedata->type, texturedata->texture);

		// if (texture->modMode) {
		//     GL_SetColor(data, texture->r, texture->g, texture->b, texture->a);
		// } else {
		//     GL_SetColor(data, 255, 255, 255, 255);
		// }

		// GL_SetBlendMode(data, texture->blendMode);

		// if (texturedata->yuv) {
		//     GL_SetShader(data, SHADER_YV12);
		// } else {
		    // GL_SetShader(data, SHADER_RGB);
		// }

		minx = dstrect->x;
		miny = dstrect->y;
		maxx = dstrect->x + dstrect->w;
		maxy = dstrect->y + dstrect->h;

		minu = (GLfloat) srcrect->x / texture->w;
		minu *= texturedata->texw;
		maxu = (GLfloat) (srcrect->x + srcrect->w) / texture->w;
		maxu *= texturedata->texw;
		minv = (GLfloat) srcrect->y / texture->h;
		minv *= texturedata->texh;
		maxv = (GLfloat) (srcrect->y + srcrect->h) / texture->h;
		maxv *= texturedata->texh;


		quads.uv  [quads.count].xy[0] = Coords::XY(minu, minv);
		quads.vert[quads.count].xy[0] = Coords::XY(minx, miny);
		quads.uv  [quads.count].xy[1] = Coords::XY(maxu, minv);
		quads.vert[quads.count].xy[1] = Coords::XY(maxx, miny);
		quads.uv  [quads.count].xy[2] = Coords::XY(maxu, maxv);
		quads.vert[quads.count].xy[2] = Coords::XY(maxx, maxy);
		quads.uv  [quads.count].xy[3] = Coords::XY(minu, maxv);
		quads.vert[quads.count].xy[3] = Coords::XY(minx, maxy);

		++quads.count;

		// GL_CheckError("", renderer);

		return false;
	}

	virtual bool BatchEnd(SDL_Renderer * renderer)
	{
		if (0 == quads.count)
			return false;

		if (!texInfo)
		{
			staterr("no texture data");
			return true;
		}

		funcs.glEnable(texInfo->type);
		funcs.glEnableClientState(GL_VERTEX_ARRAY);
		funcs.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		funcs.glBindTexture(texInfo->type, texInfo->texture);

		// if (texture->modMode) {
		//     GL_SetColor(data, texture->r, texture->g, texture->b, texture->a);
		// } else {
		//     GL_SetColor(data, 255, 255, 255, 255);
		// }

		funcs.glTexCoordPointer(2, GL_FLOAT, 0, quads.uv);
		funcs.glVertexPointer(2, GL_FLOAT, 0, quads.vert);

		funcs.glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(quads.count * Coords::SIZE));

		funcs.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		funcs.glDisableClientState(GL_VERTEX_ARRAY);

		return false;
	}

} hack_gl_impl;
Hack* hack_gl = &hack_gl_impl;




