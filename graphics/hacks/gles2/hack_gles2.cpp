#include <cassert>

#include "../hacks_internal.hpp"

#include "../../../config.h"
#include "../../../common/basics.h"

#include <SDL_config.h>
#include <SDL_video.h>
#include <SDL_opengles2.h>

extern "C" {
#include <opengles2/SDL_shaders_gles2.h>
}

//#include "glfuncs.h"

using namespace GraphicHacks;

//////////////

	// from sdl
typedef struct GLES2_FBOList GLES2_FBOList;

struct GLES2_FBOList
{
   Uint32 w, h;
   GLuint FBO;
   GLES2_FBOList *next;
};

typedef struct GLES2_TextureData
{
    GLenum texture;
    GLenum texture_type;
    GLenum pixel_format;
    GLenum pixel_type;
    void *pixel_data;
    size_t pitch;
    GLES2_FBOList *fbo;
} GLES2_TextureData;

typedef struct GLES2_ShaderCacheEntry
{
    GLuint id;
    GLES2_ShaderType type;
    const GLES2_ShaderInstance *instance;
    int references;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    struct GLES2_ShaderCacheEntry *prev;
    struct GLES2_ShaderCacheEntry *next;
} GLES2_ShaderCacheEntry;

typedef struct GLES2_ShaderCache
{
    int count;
    GLES2_ShaderCacheEntry *head;
} GLES2_ShaderCache;

typedef struct GLES2_ProgramCacheEntry
{
    GLuint id;
    SDL_BlendMode blend_mode;
    GLES2_ShaderCacheEntry *vertex_shader;
    GLES2_ShaderCacheEntry *fragment_shader;
    GLuint uniform_locations[16];
    Uint8 color_r, color_g, color_b, color_a;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    GLfloat projection[4][4];
    struct GLES2_ProgramCacheEntry *prev;
    struct GLES2_ProgramCacheEntry *next;
} GLES2_ProgramCacheEntry;

typedef struct GLES2_ProgramCache
{
    int count;
    GLES2_ProgramCacheEntry *head;
    GLES2_ProgramCacheEntry *tail;
} GLES2_ProgramCache;

typedef enum
{
    GLES2_ATTRIBUTE_POSITION = 0,
    GLES2_ATTRIBUTE_TEXCOORD = 1,
    GLES2_ATTRIBUTE_ANGLE = 2,
    GLES2_ATTRIBUTE_CENTER = 3,
} GLES2_Attribute;

typedef enum
{
    GLES2_UNIFORM_PROJECTION,
    GLES2_UNIFORM_TEXTURE,
    GLES2_UNIFORM_MODULATION,
    GLES2_UNIFORM_COLOR
} GLES2_Uniform;

typedef enum
{
    GLES2_IMAGESOURCE_SOLID,
    GLES2_IMAGESOURCE_TEXTURE_ABGR,
    GLES2_IMAGESOURCE_TEXTURE_ARGB,
    GLES2_IMAGESOURCE_TEXTURE_RGB,
    GLES2_IMAGESOURCE_TEXTURE_BGR
} GLES2_ImageSource;

typedef struct GLES2_DriverContext
{
    SDL_GLContext *context;

    SDL_bool debug_enabled;

    struct {
        int blendMode;
        SDL_bool tex_coords;
    } current;

#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "opengles2/SDL_gles2funcs.h"
#undef SDL_PROC
    GLES2_FBOList *framebuffers;
    GLuint window_framebuffer;

    int shader_format_count;
    GLenum *shader_formats;
    GLES2_ShaderCache shader_cache;
    GLES2_ProgramCache program_cache;
    GLES2_ProgramCacheEntry *current_program;
    Uint8 clear_r, clear_g, clear_b, clear_a;
} GLES2_DriverContext;

static const float inv255f = 1.0f / 255.0f;

static int
GLES2_SetOrthographicProjection(SDL_Renderer *renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat projection[4][4];

    if (!renderer->viewport.w || !renderer->viewport.h) {
        return 0;
    }

    /* Prepare an orthographic projection */
    projection[0][0] = 2.0f / renderer->viewport.w;
    projection[0][1] = 0.0f;
    projection[0][2] = 0.0f;
    projection[0][3] = 0.0f;
    projection[1][0] = 0.0f;
    if (renderer->target) {
        projection[1][1] = 2.0f / renderer->viewport.h;
    } else {
        projection[1][1] = -2.0f / renderer->viewport.h;
    }
    projection[1][2] = 0.0f;
    projection[1][3] = 0.0f;
    projection[2][0] = 0.0f;
    projection[2][1] = 0.0f;
    projection[2][2] = 0.0f;
    projection[2][3] = 0.0f;
    projection[3][0] = -1.0f;
    if (renderer->target) {
        projection[3][1] = -1.0f;
    } else {
        projection[3][1] = 1.0f;
    }
    projection[3][2] = 0.0f;
    projection[3][3] = 1.0f;

    /* Set the projection matrix */
    if (SDL_memcmp(data->current_program->projection, projection, sizeof (projection)) != 0) {
        const GLuint locProjection = data->current_program->uniform_locations[GLES2_UNIFORM_PROJECTION];
        data->glUniformMatrix4fv(locProjection, 1, GL_FALSE, (GLfloat *)projection);
        SDL_memcpy(data->current_program->projection, projection, sizeof (projection));
    }

    return 0;
}


#define GLES2_MAX_CACHED_PROGRAMS 8

static int GLES2_LoadFunctions(GLES2_DriverContext * data)
{
#if SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_ANDROID
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret,func,params) data->func=func;
#else
#define SDL_PROC(ret,func,params) \
	do { \
		data->func = SDL_GL_GetProcAddress(#func); \
		if ( ! data->func ) { \
			SDL_SetError("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
			return -1; \
		} \
	} while ( 0 );
#endif /* _SDL_NOGETPROCADDR_ */

#include "opengles2/SDL_gles2funcs.h"
#undef SDL_PROC
	return 0;
}


static GLES2_ShaderCacheEntry *GLES2_CacheShader(SDL_Renderer *renderer, GLES2_ShaderType type,
												 SDL_BlendMode blendMode);
static void GLES2_EvictShader(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *entry);
static GLES2_ProgramCacheEntry *GLES2_CacheProgram(SDL_Renderer *renderer,
												   GLES2_ShaderCacheEntry *vertex,
												   GLES2_ShaderCacheEntry *fragment,
												   SDL_BlendMode blendMode);
static int GLES2_SelectProgram(SDL_Renderer *renderer, GLES2_ImageSource source,
							   SDL_BlendMode blendMode);

static GLES2_ProgramCacheEntry *
GLES2_CacheProgram(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *vertex,
                   GLES2_ShaderCacheEntry *fragment, SDL_BlendMode blendMode)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_ProgramCacheEntry *entry;
    GLES2_ShaderCacheEntry *shaderEntry;
    GLint linkSuccessful;

    /* Check if we've already cached this program */
    entry = data->program_cache.head;
    while (entry)
    {
        if (entry->vertex_shader == vertex && entry->fragment_shader == fragment)
            break;
        entry = entry->next;
    }
    if (entry)
    {
        if (data->program_cache.head != entry)
        {
            if (entry->next)
                entry->next->prev = entry->prev;
            if (entry->prev)
                entry->prev->next = entry->next;
            entry->prev = NULL;
            entry->next = data->program_cache.head;
            data->program_cache.head->prev = entry;
            data->program_cache.head = entry;
        }
        return entry;
    }

    /* Create a program cache entry */
    entry = (GLES2_ProgramCacheEntry *)SDL_calloc(1, sizeof(GLES2_ProgramCacheEntry));
    if (!entry)
    {
        SDL_OutOfMemory();
        return NULL;
    }
    entry->vertex_shader = vertex;
    entry->fragment_shader = fragment;
    entry->blend_mode = blendMode;

    /* Create the program and link it */
    entry->id = data->glCreateProgram();
    data->glAttachShader(entry->id, vertex->id);
    data->glAttachShader(entry->id, fragment->id);
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_POSITION, "a_position");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_TEXCOORD, "a_texCoord");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_ANGLE, "a_angle");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_CENTER, "a_center");
    data->glLinkProgram(entry->id);
    data->glGetProgramiv(entry->id, GL_LINK_STATUS, &linkSuccessful);
    if (!linkSuccessful)
    {
        data->glDeleteProgram(entry->id);
        SDL_free(entry);
        SDL_SetError("Failed to link shader program");
        return NULL;
    }

    /* Predetermine locations of uniform variables */
    entry->uniform_locations[GLES2_UNIFORM_PROJECTION] =
        data->glGetUniformLocation(entry->id, "u_projection");
    entry->uniform_locations[GLES2_UNIFORM_TEXTURE] =
        data->glGetUniformLocation(entry->id, "u_texture");
    entry->uniform_locations[GLES2_UNIFORM_MODULATION] =
        data->glGetUniformLocation(entry->id, "u_modulation");
    entry->uniform_locations[GLES2_UNIFORM_COLOR] =
        data->glGetUniformLocation(entry->id, "u_color");

    entry->modulation_r = entry->modulation_g = entry->modulation_b = entry->modulation_a = 1.0f;
    entry->color_r = entry->color_g = entry->color_b = entry->color_a = 1.0f;

    data->glUseProgram(entry->id);
    data->glUniformMatrix4fv(entry->uniform_locations[GLES2_UNIFORM_PROJECTION], 1, GL_FALSE, (GLfloat *)entry->projection);
    data->glUniform1i(entry->uniform_locations[GLES2_UNIFORM_TEXTURE], 0);  /* always texture unit 0. */
    data->glUniform4f(entry->uniform_locations[GLES2_UNIFORM_MODULATION], 1.0f, 1.0f, 1.0f, 1.0f);
    data->glUniform4f(entry->uniform_locations[GLES2_UNIFORM_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

    /* Cache the linked program */
    if (data->program_cache.head)
    {
        entry->next = data->program_cache.head;
        data->program_cache.head->prev = entry;
    }
    else
    {
        data->program_cache.tail = entry;
    }
    data->program_cache.head = entry;
    ++data->program_cache.count;

    /* Increment the refcount of the shaders we're using */
    ++vertex->references;
    ++fragment->references;

    /* Evict the last entry from the cache if we exceed the limit */
    if (data->program_cache.count > GLES2_MAX_CACHED_PROGRAMS)
    {
        shaderEntry = data->program_cache.tail->vertex_shader;
        if (--shaderEntry->references <= 0)
            GLES2_EvictShader(renderer, shaderEntry);
        shaderEntry = data->program_cache.tail->fragment_shader;
        if (--shaderEntry->references <= 0)
            GLES2_EvictShader(renderer, shaderEntry);
        data->glDeleteProgram(data->program_cache.tail->id);
        data->program_cache.tail = data->program_cache.tail->prev;
        SDL_free(data->program_cache.tail->next);
        data->program_cache.tail->next = NULL;
        --data->program_cache.count;
    }
    return entry;
}

static GLES2_ShaderCacheEntry *
GLES2_CacheShader(SDL_Renderer *renderer, GLES2_ShaderType type, SDL_BlendMode blendMode)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    const GLES2_Shader *shader;
    const GLES2_ShaderInstance *instance = NULL;
    GLES2_ShaderCacheEntry *entry = NULL;
    GLint compileSuccessful = GL_FALSE;
    int i, j;

    /* Find the corresponding shader */
    shader = GLES2_GetShader(type, blendMode);
    if (!shader)
    {
        SDL_SetError("No shader matching the requested characteristics was found");
        return NULL;
    }

    /* Find a matching shader instance that's supported on this hardware */
    for (i = 0; i < shader->instance_count && !instance; ++i)
    {
        for (j = 0; j < data->shader_format_count && !instance; ++j)
        {
            if (!shader->instances)
                continue;
            if (!shader->instances[i])
                continue;
            if (shader->instances[i]->format != data->shader_formats[j])
                continue;
            instance = shader->instances[i];
        }
    }
    if (!instance)
    {
        SDL_SetError("The specified shader cannot be loaded on the current platform");
        return NULL;
    }

    /* Check if we've already cached this shader */
    entry = data->shader_cache.head;
    while (entry)
    {
        if (entry->instance == instance)
            break;
        entry = entry->next;
    }
    if (entry)
        return entry;

    /* Create a shader cache entry */
    entry = (GLES2_ShaderCacheEntry *)SDL_calloc(1, sizeof(GLES2_ShaderCacheEntry));
    if (!entry)
    {
        SDL_OutOfMemory();
        return NULL;
    }
    entry->type = type;
    entry->instance = instance;

    /* Compile or load the selected shader instance */
    entry->id = data->glCreateShader(instance->type);
    if (instance->format == (GLenum)-1)
    {
        data->glShaderSource(entry->id, 1, (const char **)&instance->data, NULL);
        data->glCompileShader(entry->id);
        data->glGetShaderiv(entry->id, GL_COMPILE_STATUS, &compileSuccessful);
    }
    else
    {
        data->glShaderBinary(1, &entry->id, instance->format, instance->data, instance->length);
        compileSuccessful = GL_TRUE;
    }
    if (!compileSuccessful)
    {
        char *info = NULL;
        int length = 0;

        data->glGetShaderiv(entry->id, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            info = SDL_stack_alloc(char, length);
            if (info) {
                data->glGetShaderInfoLog(entry->id, length, &length, info);
            }
        }
        if (info) {
            SDL_SetError("Failed to load the shader: %s", info);
            SDL_stack_free(info);
        } else {
            SDL_SetError("Failed to load the shader");
        }
        data->glDeleteShader(entry->id);
        SDL_free(entry);
        return NULL;
    }

    /* Link the shader entry in at the front of the cache */
    if (data->shader_cache.head)
    {
        entry->next = data->shader_cache.head;
        data->shader_cache.head->prev = entry;
    }
    data->shader_cache.head = entry;
    ++data->shader_cache.count;
    return entry;
}

static void
GLES2_EvictShader(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *entry)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    /* Unlink the shader from the cache */
    if (entry->next)
        entry->next->prev = entry->prev;
    if (entry->prev)
        entry->prev->next = entry->next;
    if (data->shader_cache.head == entry)
        data->shader_cache.head = entry->next;
    --data->shader_cache.count;

    /* Deallocate the shader */
    data->glDeleteShader(entry->id);
    SDL_free(entry);
}

static int
GLES2_SelectProgram(SDL_Renderer *renderer, GLES2_ImageSource source, SDL_BlendMode blendMode)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_ShaderCacheEntry *vertex = NULL;
    GLES2_ShaderCacheEntry *fragment = NULL;
    GLES2_ShaderType vtype, ftype;
    GLES2_ProgramCacheEntry *program;

    /* Select an appropriate shader pair for the specified modes */
    vtype = GLES2_SHADER_VERTEX_DEFAULT;
    switch (source)
    {
    case GLES2_IMAGESOURCE_SOLID:
        ftype = GLES2_SHADER_FRAGMENT_SOLID_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_ABGR:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_ARGB:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_RGB:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_BGR:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC;
        break;
    default:
        goto fault;
    }

    /* Load the requested shaders */
    vertex = GLES2_CacheShader(renderer, vtype, blendMode);
    if (!vertex)
        goto fault;
    fragment = GLES2_CacheShader(renderer, ftype, blendMode);
    if (!fragment)
        goto fault;

    /* Check if we need to change programs at all */
    if (data->current_program &&
        data->current_program->vertex_shader == vertex &&
        data->current_program->fragment_shader == fragment)
        return 0;

    /* Generate a matching program */
    program = GLES2_CacheProgram(renderer, vertex, fragment, blendMode);
    if (!program)
        goto fault;

    /* Select that program in OpenGL */
    data->glUseProgram(program->id);

    /* Set the current program */
    data->current_program = program;

    /* Activate an orthographic projection */
    if (GLES2_SetOrthographicProjection(renderer) < 0)
        goto fault;

    /* Clean up and return */
    return 0;
fault:
    if (vertex && vertex->references <= 0)
        GLES2_EvictShader(renderer, vertex);
    if (fragment && fragment->references <= 0)
        GLES2_EvictShader(renderer, fragment);
    data->current_program = NULL;
    return -1;
}

static void
GLES2_SetBlendMode(GLES2_DriverContext *data, int blendMode)
{
    if (blendMode != data->current.blendMode) {
        switch (blendMode) {
        default:
        case SDL_BLENDMODE_NONE:
            data->glDisable(GL_BLEND);
            break;
        case SDL_BLENDMODE_BLEND:
            data->glEnable(GL_BLEND);
            data->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case SDL_BLENDMODE_ADD:
            data->glEnable(GL_BLEND);
            data->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
            break;
        case SDL_BLENDMODE_MOD:
            data->glEnable(GL_BLEND);
            data->glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
            break;
        }
        data->current.blendMode = blendMode;
    }
}

static void
GLES2_SetTexCoords(GLES2_DriverContext * data, SDL_bool enabled)
{
    if (enabled != data->current.tex_coords) {
        if (enabled) {
            data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
        } else {
            data->glDisableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
        }
        data->current.tex_coords = enabled;
    }
}

#define RGBA8888_FROM_RGBA(Pixel, r, g, b, a)                           \
{                                                                       \
    Pixel = (r<<24)|(g<<16)|(b<<8)|a;                                   \
}

static bool
CompareColors(Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
              Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2)
{
    Uint32 Pixel1, Pixel2;
    RGBA8888_FROM_RGBA(Pixel1, r1, g1, b1, a1);
    RGBA8888_FROM_RGBA(Pixel2, r2, g2, b2, a2);
    return (Pixel1 == Pixel2);
}

/////////////////

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


struct hack_gles2_t : public Hack
{
	virtual char const* name() const { return "opengles2"; }



	virtual bool Init(SDL_Renderer * renderer)
	{
		//GLES2_DriverContext *rdata = (GLES2_DriverContext *)renderer->driverdata;
		//if (GLES2_LoadFunctions(rdata))
		//	return true;
		
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
		
		XY xya[3];
		XY xyb[3];

		enum { SIZE = 6 };
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


	SDL_Texture* texInfo;

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
//		GLES2_TextureData *texturedata = (GLES2_TextureData *) texture->driverdata;
		GLfloat minx, miny, maxx, maxy;
		GLfloat minu, maxu, minv, maxv;

		if (texInfo)
		{
			if (texInfo != texture)
			{
				staterr("batch operation have to be with the same texture");
				assert(false);
				return true;
			}
		}
		else
		{
			texInfo = texture;
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

		minu = srcrect->x / (GLfloat)texture->w;
		minv = srcrect->y / (GLfloat)texture->h;
		maxu = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
		maxv = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
		

		quads.uv  [quads.count].xya[0] = Coords::XY(minu, minv);
		quads.vert[quads.count].xya[0] = Coords::XY(minx, miny);
		quads.uv  [quads.count].xya[1] = Coords::XY(maxu, minv);
		quads.vert[quads.count].xya[1] = Coords::XY(maxx, miny);
		quads.uv  [quads.count].xya[2] = Coords::XY(minu, maxv);
		quads.vert[quads.count].xya[2] = Coords::XY(minx, maxy);

		quads.uv  [quads.count].xyb[0] = Coords::XY(minu, maxv);
		quads.vert[quads.count].xyb[0] = Coords::XY(minx, maxy);
		quads.uv  [quads.count].xyb[1] = Coords::XY(maxu, minv);
		quads.vert[quads.count].xyb[1] = Coords::XY(maxx, miny);
		quads.uv  [quads.count].xyb[2] = Coords::XY(maxu, maxv);
		quads.vert[quads.count].xyb[2] = Coords::XY(maxx, maxy);

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

	    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
	    GLES2_TextureData *tdata = (GLES2_TextureData *)texInfo->driverdata;
	    GLES2_ImageSource sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
	    SDL_BlendMode blendMode;
	    GLfloat vertices[8];
	    GLfloat texCoords[8];
	    GLES2_ProgramCacheEntry *program;
	    Uint8 r, g, b, a;

    /* Activate an appropriate shader and set the projection matrix */
    blendMode = texInfo->blendMode;
    if (renderer->target) {
        /* Check if we need to do color mapping between the source and render target textures */
        if (renderer->target->format != texInfo->format) {
            switch (texInfo->format)
            {
            case SDL_PIXELFORMAT_ABGR8888:
                switch (renderer->target->format)
                {
                    case SDL_PIXELFORMAT_ARGB8888:
                    case SDL_PIXELFORMAT_RGB888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                        break;
                    case SDL_PIXELFORMAT_BGR888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                        break;
                }
                break;
            case SDL_PIXELFORMAT_ARGB8888:
                switch (renderer->target->format)
                {
                    case SDL_PIXELFORMAT_ABGR8888:
                    case SDL_PIXELFORMAT_BGR888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                        break;
                    case SDL_PIXELFORMAT_RGB888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                        break;
                }
                break;
            case SDL_PIXELFORMAT_BGR888:
                switch (renderer->target->format)
                {
                    case SDL_PIXELFORMAT_ABGR8888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                        break;
                    case SDL_PIXELFORMAT_ARGB8888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_RGB;
                        break;
                    case SDL_PIXELFORMAT_RGB888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                        break;
                }
                break;
            case SDL_PIXELFORMAT_RGB888:
                switch (renderer->target->format)
                {
                    case SDL_PIXELFORMAT_ABGR8888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                        break;
                    case SDL_PIXELFORMAT_ARGB8888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                        break;
                    case SDL_PIXELFORMAT_BGR888:
                        sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                        break;
                }
                break;
            }
        }
        else sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;   /* Texture formats match, use the non color mapping shader (even if the formats are not ABGR) */
    }
    else {
        switch (texInfo->format)
        {
            case SDL_PIXELFORMAT_ABGR8888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                break;
            case SDL_PIXELFORMAT_ARGB8888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                break;
            case SDL_PIXELFORMAT_BGR888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                break;
            case SDL_PIXELFORMAT_RGB888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_RGB;
                break;
            default:
                return -1;
        }
    }

    if (GLES2_SelectProgram(renderer, sourceType, blendMode) < 0) {
        return -1;
    }

    /* Select the target texture */
    data->glBindTexture(tdata->texture_type, tdata->texture);

    /* Configure color modulation */
    g = texInfo->g;
    a = texInfo->a;

    if (renderer->target &&
        (renderer->target->format == SDL_PIXELFORMAT_ARGB8888 ||
         renderer->target->format == SDL_PIXELFORMAT_RGB888)) {
        r = texInfo->b;
        b = texInfo->r;
    } else {
        r = texInfo->r;
        b = texInfo->b;
    }

    program = data->current_program;

    if (!CompareColors(program->modulation_r, program->modulation_g, program->modulation_b, program->modulation_a, r, g, b, a)) {
        data->glUniform4f(program->uniform_locations[GLES2_UNIFORM_MODULATION], r * inv255f, g * inv255f, b * inv255f, a * inv255f);
        program->modulation_r = r;
        program->modulation_g = g;
        program->modulation_b = b;
        program->modulation_a = a;
    }

    /* Configure texture blending */
    GLES2_SetBlendMode(data, blendMode);

    GLES2_SetTexCoords(data, SDL_TRUE);

		//for (int i = 0; i < quads.count; ++i)
		//{
			data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, quads.vert/* + i*/);
			data->glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, quads.uv/* + i*/);
			data->glDrawArrays(GL_TRIANGLES, 0, Coords::SIZE * quads.count);
		//}

		return false;
	}

} hack_gles2_impl;
Hack* hack_gles2 = &hack_gles2_impl;




