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
  // Disable SDL3's compatibility layer to avoid conflicts (must be before includes)
  #define SDL_NO_COMPAT 1
  #include <SDL3/SDL.h>
  #include <SDL3/SDL_main.h>
  
  // Undefine problematic SDL3 oldnames.h macros that conflict with our compatibility layer
  #ifdef SDL_USEREVENT
    #undef SDL_USEREVENT
  #endif
  #ifdef SDL_KEYDOWN  
    #undef SDL_KEYDOWN
  #endif
  #ifdef SDL_MOUSEBUTTONUP
    #undef SDL_MOUSEBUTTONUP
  #endif
  #ifdef SDL_MOUSEBUTTONDOWN
    #undef SDL_MOUSEBUTTONDOWN
  #endif
  #ifdef SDL_MOUSEMOTION
    #undef SDL_MOUSEMOTION
  #endif
  #ifdef SDL_MOUSEWHEEL
    #undef SDL_MOUSEWHEEL
  #endif
  #ifdef SDL_QUIT
    #undef SDL_QUIT
  #endif
  #ifdef SDL_WINDOWEVENT_CLOSE
    #undef SDL_WINDOWEVENT_CLOSE
  #endif
  #ifdef SDL_WINDOWEVENT_SIZE_CHANGED
    #undef SDL_WINDOWEVENT_SIZE_CHANGED
  #endif
  #ifdef KMOD_CTRL
    #undef KMOD_CTRL
  #endif
  #ifdef KMOD_SHIFT
    #undef KMOD_SHIFT
  #endif
  #ifdef KMOD_ALT
    #undef KMOD_ALT
  #endif
  #ifdef SDL_BUTTON
    #undef SDL_BUTTON
  #endif
  #ifdef SDL_RenderCopy
    #undef SDL_RenderCopy
  #endif
  #ifdef SDL_RenderFillRect
    #undef SDL_RenderFillRect
  #endif
  #ifdef SDL_GetRendererOutputSize
    #undef SDL_GetRendererOutputSize
  #endif
  #ifdef SDL_RenderSetLogicalSize
    #undef SDL_RenderSetLogicalSize
  #endif
  #ifdef SDL_ConvertSurfaceFormat
    #undef SDL_ConvertSurfaceFormat
  #endif
  #ifdef SDL_FreeCursor
    #undef SDL_FreeCursor
  #endif
  #ifdef SDL_FreeSurface
    #undef SDL_FreeSurface
  #endif
  #ifdef SDL_RenderDrawLine
    #undef SDL_RenderDrawLine
  #endif
  #ifdef SDL_GameController
    #undef SDL_GameController
  #endif
  #ifdef SDL_GameControllerOpen
    #undef SDL_GameControllerOpen
  #endif
  #ifdef SDL_IsGameController
    #undef SDL_IsGameController
  #endif
  
  // SDL3_mixer and SDL3_image are now built via FetchContent
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
#define SDL_COMPAT_KEY_MOD(event) ((event).key.mod)

// SDL3 compatibility mappings - native SDL3 API equivalents
#define SDL_USEREVENT SDL_EVENT_USER
#define SDL_KEYDOWN SDL_EVENT_KEY_DOWN
#define SDL_MOUSEBUTTONUP SDL_EVENT_MOUSE_BUTTON_UP
#define SDL_MOUSEBUTTONDOWN SDL_EVENT_MOUSE_BUTTON_DOWN
#define SDL_MOUSEMOTION SDL_EVENT_MOUSE_MOTION
#define SDL_MOUSEWHEEL SDL_EVENT_MOUSE_WHEEL
#define SDL_QUIT SDL_EVENT_QUIT
#define SDL_WINDOWEVENT SDL_EVENT_WINDOW_RESIZED
#define SDL_WINDOWEVENT_CLOSE SDL_EVENT_WINDOW_CLOSE_REQUESTED
#define SDL_WINDOWEVENT_SIZE_CHANGED SDL_EVENT_WINDOW_RESIZED

// SDL3 key modifier mappings
#define KMOD_CTRL SDL_KMOD_CTRL
#define KMOD_SHIFT SDL_KMOD_SHIFT
#define KMOD_ALT SDL_KMOD_ALT

// SDL3 mouse button mapping
#define SDL_BUTTON(x) SDL_BUTTON_MASK(x)

// SDL3 initialization constants
#define SDL_INIT_TIMER 0x00000001u
#define SDL_INIT_EVENTS 0x00004000u

// SDL3 timer callback compatibility 
#define SDL_COMPAT_ADD_TIMER(interval, callback, param) SDL_AddTimer(interval, callback, param)

// Forward declare compatibility functions before macros
#ifdef __cplusplus
extern "C" {
#endif
bool SDL_RenderCopy_Compat(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect);
bool SDL_RenderFillRect_Compat(SDL_Renderer *renderer, const SDL_Rect *rect);
#ifdef __cplusplus
}
#endif

// SDL3 coordinate conversion for rendering (SDL3 uses float coordinates)
#define SDL_RenderCopy SDL_RenderCopy_Compat
#define SDL_RenderFillRect SDL_RenderFillRect_Compat

// SDL3 function renames
#define SDL_GetRendererOutputSize SDL_GetCurrentRenderOutputSize
#define SDL_RenderSetLogicalSize SDL_SetRenderLogicalPresentation_Compat
#define SDL_ConvertSurfaceFormat SDL_ConvertSurface
#define SDL_FreeCursor SDL_DestroyCursor
#define SDL_FreeSurface SDL_DestroySurface
#define SDL_CreateRGBSurface SDL_CreateSurface_Compat
#define SDL_CreateRGBSurfaceFrom SDL_CreateSurfaceFrom_Compat
#define SDL_VideoQuit() SDL_QuitSubSystem(SDL_INIT_VIDEO)
#define SDL_GL_GetDrawableSize SDL_GetWindowSizeInPixels
#define SDL_RenderDrawLine SDL_RenderLine

// SDL3 audio initialization compatibility
#define SDL_COMPAT_AUDIO_INIT(driver) SDL_InitSubSystem(SDL_INIT_AUDIO)
#define SDL_COMPAT_AUDIO_QUIT() SDL_QuitSubSystem(SDL_INIT_AUDIO)

// SDL3 Mix_OpenAudio compatibility wrapper
inline bool SDL_COMPAT_MIX_OPEN_AUDIO(int freq, SDL_AudioFormat format, int channels, int chunksize) {
  SDL_AudioSpec spec = {format, channels, freq};
  return Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
}

// SDL3 RWops -> IOStream compatibility
#undef SDL_RWops
#define SDL_RWops SDL_IOStream
#undef SDL_RWFromMem
#define SDL_RWFromMem SDL_IOFromMem
#undef SDL_LoadWAV_RW
#define SDL_LoadWAV_RW SDL_LoadWAV_IO
#define Mix_LoadWAV_RW Mix_LoadWAV_IO
#define Mix_LoadMUS_RW Mix_LoadMUS_IO

// SDL3 Mix_GetError compatibility (uses SDL_GetError)
#define Mix_GetError SDL_GetError

// SDL3 version compatibility
typedef struct SDL_version { 
  Uint8 major; 
  Uint8 minor; 
  Uint8 patch; 
} SDL_version;

inline void SDL_GetVersion(SDL_version *ver) {
  int version = SDL_GetVersion();
  ver->major = (version >> 16) & 0xFF;
  ver->minor = (version >> 8) & 0xFF;
  ver->patch = version & 0xFF;
}

inline const SDL_version* Mix_Linked_Version() { 
  static SDL_version version = {3, 0, 0}; // SDL3_mixer version
  return &version;
}

// SDL3 LoadWAV compatibility wrapper
inline Mix_Chunk* SDL_LoadWAV_IO_Compat(SDL_IOStream *src, int freesrc) {
  SDL_AudioSpec spec;
  Uint8 *audio_buf;
  Uint32 audio_len;
  if (!SDL_LoadWAV_IO(src, freesrc, &spec, &audio_buf, &audio_len)) {
    return nullptr;
  }
  
  Mix_Chunk *chunk = (Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk));
  if (!chunk) {
    SDL_free(audio_buf);
    return nullptr;
  }
  
  chunk->allocated = 1;
  chunk->abuf = audio_buf;
  chunk->alen = audio_len;
  chunk->volume = 128;
  
  return chunk;
}

#undef SDL_LoadWAV_RW
#define SDL_LoadWAV_RW SDL_LoadWAV_IO_Compat

// SDL3 window/renderer creation changes  
#define SDL_CreateRenderer(window, index, flags) SDL_CreateRenderer(window, NULL)
#define SDL_WINDOW_FULLSCREEN_DESKTOP SDL_WINDOW_FULLSCREEN

// SDL3 texture creation compatibility - pixel format parameter handling
#define SDL_CreateTexture(renderer, format, access, w, h) SDL_CreateTexture(renderer, (SDL_PixelFormat)(format), access, w, h)
#define SDL_TEXTUREACCESS_TARGET SDL_TEXTUREACCESS_TARGET

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
#define SDL_COMPAT_KEY_MOD(event) ((event).key.keysym.mod)

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

// SDL2 timer callback compatibility
#define SDL_COMPAT_ADD_TIMER(interval, callback, param) SDL_AddTimer(interval, callback, param)

// SDL2 function names are unchanged
// (macros already defined to themselves above)

#endif

// Common utility macros for both versions
#define SDL_LOG_ERROR(msg) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_WARN(msg) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, msg)
#define SDL_LOG_INFO(msg) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, msg)

// SDL3 compatibility functions for surface creation and pixel formats
#ifdef USE_SDL3
inline int SDL_SetRenderLogicalPresentation_Compat(SDL_Renderer* renderer, int w, int h) {
  // SDL3 function signature: SDL_SetRenderLogicalPresentation(renderer, w, h, mode)
  if (SDL_SetRenderLogicalPresentation(renderer, w, h, 
                                       SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
    return 0;
  } else {
    // If letterbox fails, try stretch mode instead
    if (SDL_SetRenderLogicalPresentation(renderer, w, h, 
                                         SDL_LOGICAL_PRESENTATION_STRETCH)) {
      return 0;
    }
    return -1;
  }
}

inline SDL_Surface* SDL_CreateSurface_Compat(Uint32 flags, int width, int height, int depth,
                                              Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
  // In SDL3, use default RGBA8888 format for compatibility
  return SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);
}

inline SDL_Surface* SDL_CreateSurfaceFrom_Compat(void* pixels, int width, int height, int depth, int pitch,
                                                  Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
  // In SDL3, use default RGBA8888 format for compatibility  
  return SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA8888, pixels, pitch);
}
#endif

#endif  // SRC_SDL_COMPAT_H_