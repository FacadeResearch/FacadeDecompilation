#pragma once
#include "TextureEntry.h"

struct TextureFilmstrip {
    TextureEntry textures[20];
    char useMipmaps;
    int width;
    int height;
    int originalWidth;
    int originalHeight;
    char isLoaded;
    int numTextures;
};