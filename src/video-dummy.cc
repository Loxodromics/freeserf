/*
 * video-dummy.cc - Dummy video implementation for headless mode
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

#include "src/video-dummy.h"
#include "src/log.h"

// Define the nested classes for dummy video
class Video::Frame {
 public:
  unsigned int width, height;
  Frame(unsigned int w = 800, unsigned int h = 600) : width(w), height(h) {}
  virtual ~Frame() {}
};

class Video::Image {
 public:
  unsigned int w, h;
  Image(unsigned int width = 0, unsigned int height = 0) : w(width), h(height) {}
  virtual ~Image() {}
};

VideoDummy::VideoDummy() : width(800), height(600), fullscreen(false) {
  Log::Info["video-dummy"] << "Initializing dummy video for headless mode";
}

VideoDummy::~VideoDummy() {
}

void VideoDummy::set_resolution(unsigned int w, unsigned int h, bool fs) {
  width = w;
  height = h;
  fullscreen = fs;
}

void VideoDummy::get_resolution(unsigned int *w, unsigned int *h) {
  if (w) *w = width;
  if (h) *h = height;
}

void VideoDummy::set_fullscreen(bool enable) {
  fullscreen = enable;
}

bool VideoDummy::is_fullscreen() {
  return fullscreen;
}

Video::Frame *VideoDummy::get_screen_frame() {
  static Video::Frame screen(width, height);
  return &screen;
}

Video::Frame *VideoDummy::create_frame(unsigned int w, unsigned int h) {
  return new Video::Frame(w, h);
}

void VideoDummy::destroy_frame(Video::Frame *frame) {
  delete frame;
}

Video::Image *VideoDummy::create_image(void *data, unsigned int w, unsigned int h) {
  return new Video::Image(w, h);
}

void VideoDummy::destroy_image(Video::Image *image) {
  delete image;
}

void VideoDummy::warp_mouse(int x, int y) {
  // No-op in headless mode
}

void VideoDummy::draw_image(const Video::Image *image, int x, int y,
                            int y_offset, Video::Frame *dest) {
  // No-op in headless mode
}

void VideoDummy::draw_frame(int dx, int dy, Video::Frame *dest, int sx, int sy,
                            Video::Frame *src, int w, int h) {
  // No-op in headless mode
}

void VideoDummy::draw_rect(int x, int y, unsigned int w, unsigned int h,
                           const Video::Color color, Video::Frame *dest) {
  // No-op in headless mode
}

void VideoDummy::fill_rect(int x, int y, unsigned int w, unsigned int h,
                           const Video::Color color, Video::Frame *dest) {
  // No-op in headless mode
}

void VideoDummy::draw_line(int x, int y, int x1, int y1,
                           const Video::Color color, Video::Frame *dest) {
  // No-op in headless mode
}

void VideoDummy::swap_buffers() {
  // No-op in headless mode
}

void VideoDummy::set_cursor(void *data, unsigned int w, unsigned int h) {
  // No-op in headless mode
}

float VideoDummy::get_zoom_factor() {
  return 1.0f;
}

bool VideoDummy::set_zoom_factor(float factor) {
  return true;
}

void VideoDummy::get_screen_factor(float *fx, float *fy) {
  if (fx) *fx = 1.0f;
  if (fy) *fy = 1.0f;
}