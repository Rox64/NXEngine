#!/bin/sh

echo "Downloading dependencies\n"

hg clone -r release-2.0.1 http://hg.libsdl.org/SDL
hg clone -r 239 http://hg.libsdl.org/SDL_ttf
git clone git://github.com/cdave1/freetype2-ios.git libfreetype

cd SDL; patch -p1 < ../SDL_gles2funcs.diff; cd ..
cd SDL_ttf; patch -p1 < ../sdl_ttf.diff; cd ..
cd libfreetype; patch -p1 < ../libfreetype.diff; cd ..
