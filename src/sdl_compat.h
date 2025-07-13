/*
 * sdl_compat.h - SDL2/SDL3 compatibility layer for FreeSerf
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

#ifndef SRC_SDL_COMPAT_H_
#define SRC_SDL_COMPAT_H_

// Include appropriate SDL headers based on build configuration
#ifdef USE_SDL3
  #include <SDL3/SDL.h>
  #include <SDL3/SDL_main.h>
  #ifdef ENABLE_SDL_MIXER
    #include <SDL3_mixer/SDL_mixer.h>
  #endif
  #ifdef ENABLE_SDL_IMAGE
    #include <SDL3_image/SDL_image.h>
  #endif
#else
  #include <SDL.h>
  #ifdef ENABLE_SDL_MIXER
    #include <SDL_mixer.h>
  #endif
  #ifdef ENABLE_SDL_IMAGE
    #include <SDL_image.h>
  #endif
#endif

// Compatibility macros and inline functions for gradual migration
#ifdef USE_SDL3

// SDL3 uses bool return values instead of int error codes
#define SDL_CHECK_SUCCESS(call) (call)
#define SDL_CHECK_ERROR(call) (!(call))

// SDL3 event structure changes
#define SDL_COMPAT_KEY(event) ((event).key.key)
#define SDL_COMPAT_BUTTON_STATE(event) ((event).button.down)

// SDL3 rendering function name changes
#define SDL_RenderCopy SDL_RenderTexture
#define SDL_RenderCopyEx SDL_RenderTextureRotated

#else  // SDL2

// SDL2 uses int return values (0 for success, negative for error)
#define SDL_CHECK_SUCCESS(call) ((call) == 0)
#define SDL_CHECK_ERROR(call) ((call) < 0)

// SDL2 event structure compatibility
#define SDL_COMPAT_KEY(event) ((event).key.keysym.sym)
#define SDL_COMPAT_BUTTON_STATE(event) ((event).button.state == SDL_PRESSED)

// SDL2 function names are unchanged
// (macros already defined to themselves above)

#endif

// Common utility macros for both versions
#define SDL_LOG_ERROR(msg) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_WARN(msg) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_INFO(msg) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, msg)

#endif  // SRC_SDL_COMPAT_H_