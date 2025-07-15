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

#ifdef WIN32
# include "src/sdl_compat.h"
#endif  // WIN32

int
main(int argc, char *argv[]) {
  std::string data_dir;
  std::string save_file;

  unsigned int screen_width = 0;
  unsigned int screen_height = 0;
  bool fullscreen = false;
  bool debug_color_channels = false;
  bool swap_rb_channels = false;
  bool channel_rgba = false;
  bool channel_bgra = false;
  bool channel_argb = false;
  bool channel_abgr = false;

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
  command_line.add_option('r', "Set display resolution (e.g. 800x600)")
                .add_parameter("RES",
                              [&screen_width, &screen_height](std::istream& s) {
                  s >> screen_width;
                  char c; s >> c;
                  s >> screen_height;
                  return true;
                });
  command_line.add_option('c', "Enable color channel debugging",
                          [&debug_color_channels](){ debug_color_channels = true; });
  command_line.add_option('s', "Swap R/B color channels for debugging",
                          [&swap_rb_channels](){ swap_rb_channels = true; });
  command_line.add_option('1', "Force RGBA color channel order",
                          [&channel_rgba](){ channel_rgba = true; });
  command_line.add_option('2', "Force BGRA color channel order",
                          [&channel_bgra](){ channel_bgra = true; });
  command_line.add_option('3', "Force ARGB color channel order",
                          [&channel_argb](){ channel_argb = true; });
  command_line.add_option('4', "Force ABGR color channel order",
                          [&channel_abgr](){ channel_abgr = true; });
  command_line.set_comment("Please report bugs to <" PACKAGE_BUGREPORT ">");
  if (!command_line.process(argc, argv)) {
    return EXIT_FAILURE;
  }

  Log::Info["main"] << "freeserf " << FREESERF_VERSION;

  // Enable color channel debugging if requested
  if (debug_color_channels) {
    Log::Info["main"] << "Color channel debugging enabled";
    set_debug_color_channels(true);
  }
  if (swap_rb_channels) {
    Log::Info["main"] << "R/B channel swapping enabled";
    set_swap_rb_channels(true);
  }
  
  // Set color channel permutation if requested
  if (channel_rgba) {
    Log::Info["main"] << "Force RGBA color channel order";
    set_channel_permutation_rgba();
  }
  if (channel_bgra) {
    Log::Info["main"] << "Force BGRA color channel order";
    set_channel_permutation_bgra();
  }
  if (channel_argb) {
    Log::Info["main"] << "Force ARGB color channel order";
    set_channel_permutation_argb();
  }
  if (channel_abgr) {
    Log::Info["main"] << "Force ABGR color channel order";
    set_channel_permutation_abgr();
  }

  Data &data = Data::get_instance();
  if (!data.load(data_dir)) {
    Log::Error["main"] << "Could not load game data.";
    return EXIT_FAILURE;
  }

  Log::Info["main"] << "Initialize graphics...";

  Graphics &gfx = Graphics::get_instance();

  /* TODO move to right place */
  Audio &audio = Audio::get_instance();
  Audio::PPlayer player = audio.get_music_player();
  if (player) {
    Audio::PTrack t = player->play_track(Audio::TypeMidiTrack0);
  }

  GameManager &game_manager = GameManager::get_instance();

  /* Either load a save game if specified or
     start a new game. */
  if (!save_file.empty()) {
    if (!game_manager.load_game(save_file)) {
      return EXIT_FAILURE;
    }
  } else {
    if (!game_manager.start_random_game()) {
      return EXIT_FAILURE;
    }
  }

  /* Initialize interface */
  Interface interface;
  if ((screen_width == 0) || (screen_height == 0)) {
    gfx.get_resolution(&screen_width, &screen_height);
  }
  interface.set_size(screen_width, screen_height);
  interface.set_displayed(true);

  if (save_file.empty()) {
    interface.open_game_init();
  }

  /* Init game loop */
  EventLoop &event_loop = EventLoop::get_instance();
  event_loop.add_handler(&interface);

  /* Start game loop */
  event_loop.run();

  event_loop.del_handler(&interface);

  Log::Info["main"] << "Cleaning up...";

  return EXIT_SUCCESS;
}
