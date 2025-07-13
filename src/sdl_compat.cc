/*
 * sdl_compat.cc - SDL2/SDL3 compatibility layer implementation
 *
 * Copyright (C) 2024  FreeSerf Contributors
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

#include "src/sdl_compat.h"

#ifdef USE_SDL3

// Implementation of SDL3 compatibility functions
bool SDL_RenderCopy_Compat(SDL_Renderer *renderer, SDL_Texture *texture,
                           const SDL_Rect *srcrect, const SDL_Rect *dstrect) {
  SDL_FRect src_frect, dst_frect;
  SDL_FRect *src_ptr = nullptr, *dst_ptr = nullptr;

  if (srcrect) {
    src_frect.x = static_cast<float>(srcrect->x);
    src_frect.y = static_cast<float>(srcrect->y);
    src_frect.w = static_cast<float>(srcrect->w);
    src_frect.h = static_cast<float>(srcrect->h);
    src_ptr = &src_frect;
  }

  if (dstrect) {
    dst_frect.x = static_cast<float>(dstrect->x);
    dst_frect.y = static_cast<float>(dstrect->y);
    dst_frect.w = static_cast<float>(dstrect->w);
    dst_frect.h = static_cast<float>(dstrect->h);
    dst_ptr = &dst_frect;
  }

  // Call native SDL3 function
  return SDL_RenderTexture(renderer, texture, src_ptr, dst_ptr);
}

bool SDL_RenderFillRect_Compat(SDL_Renderer *renderer,
                               const SDL_Rect *rect) {
  if (rect) {
    SDL_FRect frect;
    frect.x = static_cast<float>(rect->x);
    frect.y = static_cast<float>(rect->y);
    frect.w = static_cast<float>(rect->w);
    frect.h = static_cast<float>(rect->h);
    // Temporarily undefine macro to call native SDL3 function
    #undef SDL_RenderFillRect
    bool result = SDL_RenderFillRect(renderer, &frect);
    #define SDL_RenderFillRect SDL_RenderFillRect_Compat
    return result;
  } else {
    #undef SDL_RenderFillRect
    bool result = SDL_RenderFillRect(renderer, nullptr);
    #define SDL_RenderFillRect SDL_RenderFillRect_Compat
    return result;
  }
}

#endif
