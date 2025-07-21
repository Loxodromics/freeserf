/*
 * headless-handler.cc - Minimal event handler for headless mode
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

#include "src/headless-handler.h"
#include "src/log.h"
#include "src/game-manager.h"

HeadlessHandler::HeadlessHandler() : frame_count(0) {
  Log::Info["headless-handler"] << "Initializing headless game handler";
}

HeadlessHandler::~HeadlessHandler() {
}

bool HeadlessHandler::handle_event(const Event *event) {
  switch (event->type) {
    case Event::TypeUpdate:
      if (game) {
        game->update();
        frame_count++;
        
        // Log periodic status
        if (frame_count % 500 == 0) {
          Log::Info["headless-handler"] << "Processed " << frame_count << " simulation frames";
        }
      } else {
        GameManager &game_manager = GameManager::get_instance();
        game = game_manager.get_current_game();
        if (game) {
          Log::Info["headless-handler"] << "Game instance acquired from GameManager";
        } else {
          Log::Error["headless-handler"] << "Failed to acquire game instance";
        }
      }
      break;
      
    case Event::TypeDraw:
      // Skip drawing in headless mode (no logging needed)
      break;
      
    default:
      // Ignore other events (mouse, keyboard, etc.) in headless mode
      break;
  }
  
  return true;
}

void HeadlessHandler::set_game(PGame g) {
  game = g;
  Log::Info["headless-handler"] << "Game instance set";
}