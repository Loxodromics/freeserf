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

// SDL3 audio function changes
#define SDL_OpenAudioDevice SDL_OpenAudioDeviceStream
#define SDL_PauseAudioDevice SDL_ResumeAudioDevice
#define SDL_LoadWAV_RW SDL_LoadWAV_IO
#define SDL_RWFromMem SDL_IOFromMem

// SDL3 audio initialization compatibility
#define SDL_COMPAT_AUDIO_INIT(driver) SDL_InitSubSystem(SDL_INIT_AUDIO)
#define SDL_COMPAT_AUDIO_QUIT() SDL_QuitSubSystem(SDL_INIT_AUDIO)

// SDL3 Mix_OpenAudio compatibility wrapper
#define SDL_COMPAT_MIX_OPEN_AUDIO(freq, format, channels, chunksize) \
  Mix_OpenAudio(freq, format, channels, chunksize)

// SDL3 window/renderer creation changes
#define SDL_CreateRenderer(window, index, flags) SDL_CreateRenderer(window, NULL)

// SDL3 rendering function renames
#define SDL_GetRendererOutputSize SDL_GetCurrentRenderOutputSize
#define SDL_RenderSetLogicalSize SDL_SetRenderLogicalPresentation_Compat

// SDL3 texture filtering compatibility
#define SDL_COMPAT_SET_TEXTURE_FILTERING(texture) SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)

// SDL3 renderer flushing (required before direct graphics API calls)
#define SDL_COMPAT_FLUSH_RENDERER(renderer) SDL_FlushRenderer(renderer)

// SDL3 mouse coordinate changes (now float)
#define SDL_COMPAT_MOUSE_X(event) ((int)(event).button.x)
#define SDL_COMPAT_MOUSE_Y(event) ((int)(event).button.y)

// SDL3 event system changes
#define SDL_WINDOWEVENT SDL_EVENT_WINDOW_RESIZED
#define SDL_WINDOWEVENT_CLOSE SDL_EVENT_WINDOW_CLOSE_REQUESTED
#define SDL_WINDOWEVENT_SIZE_CHANGED SDL_EVENT_WINDOW_RESIZED
#define SDL_QUIT SDL_EVENT_QUIT

// SDL3 input event changes
#define SDL_KEYDOWN SDL_EVENT_KEY_DOWN
#define SDL_MOUSEBUTTONUP SDL_EVENT_MOUSE_BUTTON_UP
#define SDL_MOUSEBUTTONDOWN SDL_EVENT_MOUSE_BUTTON_DOWN
#define SDL_MOUSEMOTION SDL_EVENT_MOUSE_MOTION
#define SDL_MOUSEWHEEL SDL_EVENT_MOUSE_WHEEL

// SDL3 event structure compatibility macros
#define SDL_COMPAT_WINDOW_EVENT_TYPE(event) (event.type)
#define SDL_COMPAT_WINDOW_DATA1(event) (event.window.data1)
#define SDL_COMPAT_WINDOW_DATA2(event) (event.window.data2)

// SDL3 gamepad renames (if used)
#define SDL_GameController SDL_Gamepad
#define SDL_GameControllerOpen SDL_OpenGamepad
#define SDL_IsGameController SDL_IsGamepad

#else  // SDL2

// SDL2 uses int return values (0 for success, negative for error)
#define SDL_CHECK_SUCCESS(call) ((call) == 0)
#define SDL_CHECK_ERROR(call) ((call) < 0)

// SDL2 audio initialization compatibility  
#define SDL_COMPAT_AUDIO_INIT(driver) SDL_AudioInit(driver)
#define SDL_COMPAT_AUDIO_QUIT() SDL_AudioQuit()

// SDL2 Mix_OpenAudio compatibility wrapper
#define SDL_COMPAT_MIX_OPEN_AUDIO(freq, format, channels, chunksize) \
  Mix_OpenAudio(freq, format, channels, chunksize)

// SDL2 event structure compatibility
#define SDL_COMPAT_KEY(event) ((event).key.keysym.sym)
#define SDL_COMPAT_BUTTON_STATE(event) ((event).button.state == SDL_PRESSED)

// SDL2 mouse coordinates are already int
#define SDL_COMPAT_MOUSE_X(event) ((event).button.x)
#define SDL_COMPAT_MOUSE_Y(event) ((event).button.y)

// SDL2 compatibility for new SDL3 functions
#define SDL_RenderSetLogicalSize SDL_RenderSetLogicalSize
#define SDL_COMPAT_SET_TEXTURE_FILTERING(texture) ((void)0)
#define SDL_COMPAT_FLUSH_RENDERER(renderer) ((void)0)

// SDL2 event structure compatibility macros
#define SDL_COMPAT_WINDOW_EVENT_TYPE(event) (event.window.event)
#define SDL_COMPAT_WINDOW_DATA1(event) (event.window.data1)
#define SDL_COMPAT_WINDOW_DATA2(event) (event.window.data2)

// SDL2 function names are unchanged
// (macros already defined to themselves above)

#endif

// Common utility macros for both versions
#define SDL_LOG_ERROR(msg) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_WARN(msg) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_INFO(msg) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, msg)

// SDL3 compatibility function for logical presentation
#ifdef USE_SDL3
inline int SDL_SetRenderLogicalPresentation_Compat(SDL_Renderer* renderer, int w, int h) {
  return SDL_SetRenderLogicalPresentation(renderer, w, h, 
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX, 
                                          SDL_SCALEMODE_LINEAR) ? 0 : -1;
}
#endif

#endif  // SRC_SDL_COMPAT_H_