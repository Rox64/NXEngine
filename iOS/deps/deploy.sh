#!/bin/sh

echo "Downloading dependencies\n"

hg clone -r release-2.0.1 http://hg.libsdl.org/SDL
hg clone -r release-2.0.12 http://hg.libsdl.org/SDL_ttf

cd SDL; patch -p1 < ../SDL_gles2funcs.diff; cd ..
cd SDL_ttf; patch -p1 < ../sdl_ttf.diff; cd ..
cd SDL_ttf; patch -p1 < ../sdl2_ttf_apinames.diff; cd ..

