
SDL_PROC(void, glEnable, (GLenum cap))
SDL_PROC(void, glBindTexture, (GLenum, GLuint))

SDL_PROC(void, glEnableClientState, (GLenum array))
SDL_PROC(void, glDisableClientState, (GLenum array))
SDL_PROC(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count))

SDL_PROC(void, glTexCoordPointer,
         (GLint size, GLenum type, GLsizei stride,
          const GLvoid * pointer))
SDL_PROC(void, glVertexPointer,
         (GLint size, GLenum type, GLsizei stride,
          const GLvoid * pointer))
