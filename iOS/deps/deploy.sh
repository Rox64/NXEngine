#!/bin/sh

echo "Downloading dependencies\n"

hg clone http://hg.libsdl.org/SDL
hg clone http://hg.libsdl.org/SDL_ttf
git clone git://github.com/cdave1/freetype2-ios.git libfreetype
