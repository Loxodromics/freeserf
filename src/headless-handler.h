/*
 * headless-handler.h - Minimal event handler for headless mode
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

#ifndef SRC_HEADLESS_HANDLER_H_
#define SRC_HEADLESS_HANDLER_H_

#include "src/event_loop.h"
#include "src/game.h"

class HeadlessHandler : public EventLoop::Handler {
 private:
  PGame game;
  int frame_count;

 public:
  HeadlessHandler();
  virtual ~HeadlessHandler();

  virtual bool handle_event(const Event *event);
  
  void set_game(PGame game);
};

#endif  // SRC_HEADLESS_HANDLER_H_