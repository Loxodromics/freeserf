/*
 * event_loop-dummy.cc - Dummy event loop implementation for headless mode
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

#include "src/event_loop-dummy.h"
#include "src/log.h"
#include "src/video.h"

EventLoopDummy::EventLoopDummy() : running(false) {
  Log::Info["event-loop-dummy"] << "Initializing dummy event loop for headless mode";
  last_update = std::chrono::high_resolution_clock::now();
}

EventLoopDummy::~EventLoopDummy() {
}

void EventLoopDummy::run() {
  running = true;
  Log::Info["event-loop-dummy"] << "Starting headless game loop (handlers: " << event_handlers.size() << ")";
  
  auto target_frame_time = std::chrono::milliseconds(20);  // 50Hz like SDL version

  while (running) {
    auto frame_start = std::chrono::high_resolution_clock::now();
    
    // Send update event to all handlers (this drives the game simulation)
    notify_update();
    
    // Send draw event with null frame (headless - no actual drawing)  
    notify_draw(nullptr);
    
    // Sleep to maintain consistent timing
    auto frame_end = std::chrono::high_resolution_clock::now();
    auto frame_duration = frame_end - frame_start;
    
    if (frame_duration < target_frame_time) {
      std::this_thread::sleep_for(target_frame_time - frame_duration);
    }
  }
  
  Log::Info["event-loop-dummy"] << "Headless game loop stopped";
}

void EventLoopDummy::quit() {
  running = false;
}

void EventLoopDummy::deferred_call(DeferredCall call, void *data) {
  // Execute immediately in headless mode (no event queue needed)
  call(data);
}

// Timer implementation
TimerDummy::TimerDummy(unsigned int _id, unsigned int _interval, Timer::Handler *_handler)
  : Timer(_id, _interval, _handler), is_running(false) {
}

TimerDummy::~TimerDummy() {
  stop();
}

void TimerDummy::run() {
  is_running = true;
  // In headless mode, we'll use a simple approach - fire timer immediately
  // This is sufficient for current FreeSerf timer usage (blinking UI elements)
  if (handler) {
    handler->on_timer_fired(id);
  }
}

void TimerDummy::stop() {
  is_running = false;
}

// Timer::create is handled by the main event_loop-sdl.cc file
// with conditional logic based on g_headless_mode