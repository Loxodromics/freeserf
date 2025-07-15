/*
 * sprite-file-sdl.cc - Sprite loaded from file implementation
 *
 * Copyright (C) 2017  Wicked_Digger <wicked_digger@mail.ru>
 *
 * This file is part of freeserf.
 *
 * freeserf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * freeserf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freeserf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/sprite-file.h"

#include "src/sdl_compat.h"
#include "src/log.h"

SpriteFile::SpriteFile() {
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
}

bool
SpriteFile::load(const std::string &path) {
  Log::Debug["sprite"] << "SpriteFile::load: Loading " << path;
  
  SDL_Surface *image = IMG_Load(path.c_str());
  if (image == nullptr) {
    Log::Debug["sprite"] << "SpriteFile::load: Failed to load " << path;
    return false;
  }
  
  Log::Debug["sprite"] << "SpriteFile::load: Original dimensions: " << image->w << "x" << image->h;
#ifdef USE_SDL3
  Log::Debug["sprite"] << "SpriteFile::load: Original format: " << SDL_GetPixelFormatName(image->format);
#else
  Log::Debug["sprite"] << "SpriteFile::load: Original format: " << SDL_GetPixelFormatName(image->format->format);
#endif
  
  // Sample first few pixels before conversion
  if (image->pixels) {
    SDL_LockSurface(image);
    uint32_t *pixels = (uint32_t *)image->pixels;
    Log::Debug["sprite"] << "SpriteFile::load: Original first 4 pixels:";
    for (int i = 0; i < 4 && i < (image->w * image->h); i++) {
      Log::Debug["sprite"] << "  Pixel " << i << ": 0x" << std::hex << pixels[i] << std::dec;
    }
    SDL_UnlockSurface(image);
  }
  
  // Use consistent RGBA8888 pixel format for both SDL2 and SDL3
  Log::Debug["sprite"] << "SpriteFile::load: Converting to SDL_PIXELFORMAT_RGBA8888";
  SDL_Surface *surf = SDL_ConvertSurfaceFormat(image,
                                               SDL_PIXELFORMAT_RGBA8888,
                                               0);
  SDL_FreeSurface(image);
  
  if (surf == nullptr) {
    Log::Debug["sprite"] << "SpriteFile::load: Failed to convert surface format";
    return false;
  }
  
#ifdef USE_SDL3
  Log::Debug["sprite"] << "SpriteFile::load: Converted format: " << SDL_GetPixelFormatName(surf->format);
#else
  Log::Debug["sprite"] << "SpriteFile::load: Converted format: " << SDL_GetPixelFormatName(surf->format->format);
#endif
  
  SDL_LockSurface(surf);
  
  // Sample first few pixels after conversion
  uint32_t *converted_pixels = (uint32_t *)surf->pixels;
  Log::Debug["sprite"] << "SpriteFile::load: Converted first 4 pixels:";
  for (int i = 0; i < 4 && i < (surf->w * surf->h); i++) {
    uint32_t pixel = converted_pixels[i];
    // Extract RGBA components assuming RGBA8888 format
    uint8_t r = (pixel >> 24) & 0xFF;
    uint8_t g = (pixel >> 16) & 0xFF;
    uint8_t b = (pixel >> 8) & 0xFF;
    uint8_t a = pixel & 0xFF;
    Log::Debug["sprite"] << "  Pixel " << i << ": 0x" << std::hex << pixel << std::dec 
                        << " R=" << (int)r << " G=" << (int)g << " B=" << (int)b << " A=" << (int)a;
  }
  
  width = surf->w;
  height = surf->h;
  size_t size = width * height * 4;
  data = reinterpret_cast<uint8_t*>(malloc(size));
  bool res = (surf->pixels != nullptr) && (data != nullptr);
  if (res) {
    memcpy(data, surf->pixels, size);
    Log::Debug["sprite"] << "SpriteFile::load: Successfully loaded sprite " << width << "x" << height;
  } else {
    Log::Debug["sprite"] << "SpriteFile::load: Failed to copy pixel data";
  }
  SDL_UnlockSurface(surf);
  SDL_FreeSurface(surf);
  return res;
}
