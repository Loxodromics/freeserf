/*
 * event_loop-dummy.h - Dummy event loop implementation for headless mode
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

#ifndef SRC_EVENT_LOOP_DUMMY_H_
#define SRC_EVENT_LOOP_DUMMY_H_

#include <chrono>
#include <thread>

#include "src/event_loop.h"

class EventLoopDummy : public EventLoop {
 private:
  bool running;
  std::chrono::high_resolution_clock::time_point last_update;

 public:
  EventLoopDummy();
  virtual ~EventLoopDummy();

  virtual void run();
  virtual void quit();
  virtual void deferred_call(DeferredCall call, void *data);
};

class TimerDummy : public Timer {
 private:
  bool is_running;

 public:
  TimerDummy(unsigned int _id, unsigned int _interval, Timer::Handler *_handler);
  virtual ~TimerDummy();

  virtual void run();
  virtual void stop();
};

#endif  // SRC_EVENT_LOOP_DUMMY_H_