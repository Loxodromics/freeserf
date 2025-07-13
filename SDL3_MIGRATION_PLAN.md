# SDL3 Migration Plan for FreeSerf

## Phase 1: Preparation & Environment Setup

### 1.1 Dependency Updates
- [x] Update CMake configuration to find SDL3, SDL3_image, SDL3_mixer packages
- [x] Update Conan configuration (conanfile.py) to use SDL3 packages
- [x] Create compatibility layer to enable gradual migration

### 1.2 Header Migration
- [x] Create sdl_compat.h compatibility header
- [x] Update all SDL header includes to use compatibility layer
- [x] Update video-sdl.h, event_loop-sdl.h, audio-sdlmixer.h
- [x] Update all source files to use compatibility header

## Phase 2: Core SDL3 Function Migrations

### 2.1 Return Value Changes
- [ ] Convert all SDL functions from int/negative error codes to bool returns
- [ ] Update error handling patterns throughout codebase
- [ ] Focus on: VideoSDL, EventLoopSDL, AudioSDL classes

### 2.2 Symbol Renaming (using automated scripts)
- [ ] Run SDL's `rename_symbols.py` on src/ directory
- [ ] Run `rename_headers.py` on src/ directory  
- [ ] Run `rename_macros.py` on src/ directory
- [ ] Manual review and fixes for script limitations

## Phase 3: Platform Abstraction Layer Updates

### 3.1 VideoSDL Class Modifications
- [ ] Update `SDL_CreateRenderer()` calls (name parameter instead of index)
- [ ] Migrate rendering functions: `SDL_RenderCopy()` → `SDL_RenderTexture()`
- [ ] Update window creation to use `SDL_CreateWindow()` simplified API
- [ ] Add `SDL_FlushRenderer()` calls before any direct graphics API usage
- [ ] Handle new floating-point color ranges for `SDL_Vertex`

### 3.2 EventLoopSDL Class Modifications  
- [ ] Update event handling for flattened event structure
- [ ] Migrate `event.key.keysym.sym` → `event.key.key`
- [ ] Handle `SDL_PRESSED/SDL_RELEASED` → bool `down` field changes
- [ ] Update window/display events (no longer nested under SDL_WINDOWEVENT)
- [ ] Handle floating-point mouse coordinates

### 3.3 AudioSDL Class Overhaul
- [ ] **Major rewrite required** - SDL3 audio is fundamentally different
- [ ] Replace callback-based approach with SDL_AudioStream
- [ ] Update from `SDL_OpenAudioDevice()` to `SDL_OpenAudioDeviceStream()`
- [ ] Migrate `SDL_PauseAudioDevice()` → `SDL_ResumeAudioDevice()` 
- [ ] Remove deprecated audio conversion (SDL_AudioCVT) → `SDL_ConvertAudioSamples()`

## Phase 4: Build System & Dependencies

### 4.1 CMake Updates
- [ ] Update target linking: `SDL2::SDL2` → `SDL3::SDL3`
- [ ] Remove SDLmain library (replaced by SDL_main.h)
- [ ] Update test library: `SDL3test` → `SDL3_test`
- [ ] Add conditional SDL3 vs SDL2 support during transition

### 4.2 Conan Package Updates
- [ ] Update to SDL3, SDL3_mixer, SDL3_image packages
- [ ] Verify compatibility with existing build configurations
- [ ] Test both traditional and Conan builds

## Phase 5: Testing & Validation

### 5.1 Functionality Testing
- [ ] Comprehensive testing of video/rendering pipeline
- [ ] Audio playback and effects verification
- [ ] Event handling and input validation
- [ ] Window management and fullscreen modes

### 5.2 Performance Validation
- [ ] Benchmark against SDL2 version
- [ ] Verify automatic batching improvements in SDL3 renderer
- [ ] Test with different graphics drivers

## Phase 6: Code Quality & Cleanup

### 6.1 Style & Standards
- [ ] Run `ninja check_style` to ensure Google C++ Style compliance
- [ ] Update any deprecated patterns identified during migration
- [ ] Remove SDL2 compatibility code after successful migration

### 6.2 Documentation Updates
- [ ] Update CLAUDE.md with SDL3 build instructions
- [ ] Document any breaking changes or new requirements
- [ ] Update dependency requirements

## Risk Mitigation

- **Gradual Migration**: Keep SDL2 build option during transition
- **Automated Tools**: Leverage SDL's migration scripts where possible
- **Platform Abstraction**: Use existing clean architecture to isolate changes
- **Comprehensive Testing**: Focus on audio system (biggest changes)
- **Fallback Plan**: Maintain SDL2 branch until SDL3 migration proven stable

## Estimated Timeline

- **Phase 1-2**: 1-2 days (setup + basic function migrations)
- **Phase 3**: 3-4 days (platform layer updates, especially audio rewrite)
- **Phase 4**: 1 day (build system updates)
- **Phase 5-6**: 2-3 days (testing, validation, cleanup)

**Total: ~7-10 days** for complete migration with thorough testing

## Migration Progress Log

*This section will be updated as migration progresses*

### Completed Tasks
- [x] Created migration plan document
- [x] **Phase 1 Complete: Preparation & Environment Setup**
  - [x] Updated CMake configuration with USE_SDL3 option and conditional SDL2/SDL3 support
  - [x] Updated Conan configuration with use_sdl3 option
  - [x] Created sdl_compat.h compatibility header for gradual migration
  - [x] Updated all header and source files to use compatibility layer
- [x] **Phase 2 Complete: Core SDL3 Function Migrations**
  - [x] Converted SDL functions from int/negative error codes to bool returns using compatibility macros
  - [x] Updated error handling patterns throughout codebase (event_loop-sdl.cc, video-sdl.cc, audio-sdlmixer.cc)
  - [x] Added key SDL2->SDL3 symbol renames to compatibility layer
  - [x] Updated event handling to use compatibility macros for key events and mouse coordinates
- [x] **Phase 3.1 Complete: VideoSDL Class Modifications**
  - [x] Enhanced compatibility layer with SDL3-specific rendering functions and macros
  - [x] Updated SDL_CreateRenderer API calls to work with both SDL2 and SDL3
  - [x] Implemented SDL_SetRenderLogicalPresentation compatibility for logical sizing
  - [x] Added texture filtering configuration for pixel art (nearest neighbor filtering)
  - [x] Integrated SDL_FlushRenderer calls for SDL3 mandatory batching support
- [x] **Phase 3.2 Complete: EventLoopSDL Class Modifications**
  - [x] Enhanced compatibility layer with SDL3 event system mappings
  - [x] Updated window event handling for SDL3's flattened event structure
  - [x] Migrated SDL_WINDOWEVENT to direct SDL3 window events
  - [x] Updated mouse coordinate handling for SDL3 float precision in double-click detection
  - [x] Added comprehensive event type mappings (keyboard, mouse, window, quit events)

### Current Status
- Status: Phase 3.2 EventLoopSDL Class Modifications completed successfully
- Next: Continue with Phase 3.3 - AudioSDL Class Overhaul

### Notes & Issues
- None yet

### Key Findings
- FreeSerf has excellent platform abstraction layer making migration more manageable
- Audio system will require the most significant changes due to SDL3's new audio architecture
- Automated migration scripts should handle majority of symbol/function renaming