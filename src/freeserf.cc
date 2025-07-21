/*
 * freeserf.cc - Main program source.
 *
 * Copyright (C) 2013-2018  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/freeserf.h"

#include <string>
#include <iostream>

#include "src/log.h"
#include "src/version.h"
#include "src/data.h"
#include "src/audio.h"
#include "src/gfx.h"
#include "src/interface.h"
#include "src/game-manager.h"
#include "src/command_line.h"
#include "src/video-sdl.h"
#include "src/ai/ai-logger.h"
#include "src/ai/agent-integration.h"
#include "src/mission.h"
#include "src/headless.h"
#include "src/headless-handler.h"

#ifdef WIN32
# include "src/sdl_compat.h"
#endif  // WIN32

// Global headless mode flag
bool g_headless_mode = false;

// Helper function to start a game with pre-configured AI players
bool start_game_with_ai_players(int ai_player_count, bool ai_debug_mode) {
  // Create a new game info with random seed
  PGameInfo game_info(new GameInfo(Random()));
  
  // Define colors for up to 4 AI players
  Player::Color ai_colors[] = {
    {0x00, 0xe3, 0xe3},  // Cyan
    {0xcf, 0x63, 0x63},  // Red  
    {0x63, 0xcf, 0x63},  // Green
    {0xcf, 0xcf, 0x63}   // Yellow
  };
  
  // Add AI players with reasonable defaults
  for (int i = 0; i < ai_player_count && i < 4; ++i) {
    size_t character = 1 + (i % 12);  // Use characters 1-12 (avoiding 0 which is ERROR)
    unsigned int intelligence = 40;
    unsigned int supplies = 40; 
    unsigned int reproduction = 40;
    
    game_info->add_player(character, ai_colors[i], intelligence, supplies, reproduction);
  }
  
  // Start the game
  GameManager &game_manager = GameManager::get_instance();
  if (!game_manager.start_game(game_info)) {
    return false;
  }
  
  // Configure AI system
  AILogger::set_debug_enabled(ai_debug_mode);
  AgentIntegration::setup_ai_players(ai_player_count);
  AILogger::log_game_started(ai_player_count);
  Log::Info["main"] << "AI system initialized with " << ai_player_count 
                    << " AI players, debug=" << (ai_debug_mode ? "ON" : "OFF");
  
  return true;
}

int
main(int argc, char *argv[]) {
  std::string data_dir;
  std::string save_file;

  unsigned int screen_width = 0;
  unsigned int screen_height = 0;
  bool fullscreen = false;
  bool mute_audio = false;
  
  // AI configuration
  bool ai_debug_mode = false;
  int ai_player_count = 0;
  bool headless_mode = false;

  CommandLine command_line;
  command_line.add_option('d', "Set Debug output level")
                .add_parameter("NUM", [](std::istream& s) {
                  int d;
                  s >> d;
                  if (d >= 0 && d < Log::LevelMax) {
                    Log::set_level(static_cast<Log::Level>(d));
                  }
                  return true;
                });
  command_line.add_option('f', "Run in Fullscreen mode",
                          [&fullscreen](){ fullscreen = true; });
  command_line.add_option('g', "Use specified data directory")
                .add_parameter("DATA-PATH", [&data_dir](std::istream& s) {
                  s >> data_dir;
                  return true;
                });
  command_line.add_option('h', "Show this help text", [&command_line](){
                  command_line.show_help();
                  exit(EXIT_SUCCESS);
                });
  command_line.add_option('l', "Load saved game")
                .add_parameter("FILE", [&save_file](std::istream& s) {
                  std::getline(s, save_file);
                  return true;
                });
  command_line.add_option('m', "Disable all audio (mute)",
                          [&mute_audio](){ mute_audio = true; });
  command_line.add_option('r', "Set display resolution (e.g. 800x600)")
                .add_parameter("RES",
                              [&screen_width, &screen_height](std::istream& s) {
                  s >> screen_width;
                  char c; s >> c;
                  s >> screen_height;
                  return true;
                });
  
  // AI options
  command_line.add_option('a', "Enable AI debug logging",
                          [&ai_debug_mode](){ ai_debug_mode = true; });
  command_line.add_option('p', "Set number of AI players (1-4)")
                .add_parameter("NUM", [&ai_player_count](std::istream& s) {
                  s >> ai_player_count;
                  if (ai_player_count < 0) ai_player_count = 0;
                  if (ai_player_count > 4) ai_player_count = 4;
                  return true;
                });
  command_line.add_option('H', "Run in headless mode (no graphics)",
                          [&headless_mode](){ headless_mode = true; });
  
  command_line.set_comment("Please report bugs to <" PACKAGE_BUGREPORT ">");
  if (!command_line.process(argc, argv)) {
    return EXIT_FAILURE;
  }

  // Set global headless mode flag
  g_headless_mode = headless_mode;
  
  if (headless_mode) {
    Log::Info["main"] << "freeserf " << FREESERF_VERSION << " (headless mode)";
    Log::Info["main"] << "Graphics and audio disabled for simulation-only execution";
  } else {
    Log::Info["main"] << "freeserf " << FREESERF_VERSION;
  }


  Data &data = Data::get_instance();
  if (!data.load(data_dir)) {
    Log::Error["main"] << "Could not load game data.";
    return EXIT_FAILURE;
  }

  if (!headless_mode) {
    Log::Info["main"] << "Initialize graphics...";
  }

  // Graphics initialization - skipped in headless mode but we still need the instance
  // for some code paths that expect it (will use dummy video implementation)
  Graphics &gfx = Graphics::get_instance();

  /* Initialize audio (skip in headless mode) */
  if (!headless_mode) {
    Audio &audio = Audio::get_instance();
    if (!mute_audio) {
      Audio::PPlayer player = audio.get_music_player();
      if (player) {
        Audio::PTrack t = player->play_track(Audio::TypeMidiTrack0);
      }
    } else {
      // Disable audio players when muted
      Audio::PPlayer sound_player = audio.get_sound_player();
      Audio::PPlayer music_player = audio.get_music_player();
      if (sound_player) {
        sound_player->enable(false);
      }
      if (music_player) {
        music_player->enable(false);
      }
      Log::Info["main"] << "Audio disabled (mute mode)";
    }
  } else {
    Log::Info["main"] << "Audio disabled (headless mode)";
  }

  /* Either load a save game if specified or
     start a new game. */
  if (!save_file.empty()) {
    GameManager &game_manager = GameManager::get_instance();
    if (!game_manager.load_game(save_file)) {
      return EXIT_FAILURE;
    }
    
    // Configure AI system for loaded games if requested
    if (ai_debug_mode || ai_player_count > 0) {
      AILogger::set_debug_enabled(ai_debug_mode);
      
      if (ai_player_count > 0) {
        AgentIntegration::setup_ai_players(ai_player_count);
        AILogger::log_game_started(ai_player_count);
        Log::Info["main"] << "AI system initialized with " << ai_player_count 
                          << " AI players, debug=" << (ai_debug_mode ? "ON" : "OFF");
      }
    }
  } else if (ai_player_count > 0) {
    // Start a new game with pre-configured AI players
    if (!start_game_with_ai_players(ai_player_count, ai_debug_mode)) {
      return EXIT_FAILURE;
    }
  } else {
    // Start a regular random game
    GameManager &game_manager = GameManager::get_instance();
    if (!game_manager.start_random_game()) {
      return EXIT_FAILURE;
    }
  }

  /* Initialize interface (skip in headless mode) */
  if (!headless_mode) {
    Interface interface;
    if ((screen_width == 0) || (screen_height == 0)) {
      gfx.get_resolution(&screen_width, &screen_height);
    }
    interface.set_size(screen_width, screen_height);
    interface.set_displayed(true);

    if (save_file.empty() && ai_player_count == 0) {
      interface.open_game_init();
    }

    /* Init game loop */
    EventLoop &event_loop = EventLoop::get_instance();
    event_loop.add_handler(&interface);

    /* Start game loop */
    event_loop.run();

    event_loop.del_handler(&interface);
  } else {
    /* Headless mode - run game loop with minimal handler */
    Log::Info["main"] << "Starting headless simulation...";
    
    HeadlessHandler headless_handler;
    
    EventLoop &event_loop = EventLoop::get_instance();
    event_loop.add_handler(&headless_handler);
    
    /* Start game loop */
    event_loop.run();
    
    event_loop.del_handler(&headless_handler);
  }

  Log::Info["main"] << "Cleaning up...";

  return EXIT_SUCCESS;
}
