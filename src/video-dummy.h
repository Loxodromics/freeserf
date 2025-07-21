/*
 * video-dummy.h - Dummy video implementation for headless mode
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

#ifndef SRC_VIDEO_DUMMY_H_
#define SRC_VIDEO_DUMMY_H_

#include "src/video.h"

class VideoDummy : public Video {
 protected:
  unsigned int width;
  unsigned int height;
  bool fullscreen;

 public:
  VideoDummy();
  virtual ~VideoDummy();

  virtual void set_resolution(unsigned int width, unsigned int height,
                              bool fullscreen);
  virtual void get_resolution(unsigned int *width, unsigned int *height);
  virtual void set_fullscreen(bool enable);
  virtual bool is_fullscreen();

  virtual Frame *get_screen_frame();
  virtual Frame *create_frame(unsigned int width, unsigned int height);
  virtual void destroy_frame(Frame *frame);

  virtual Image *create_image(void *data, unsigned int width,
                              unsigned int height);
  virtual void destroy_image(Image *image);

  virtual void warp_mouse(int x, int y);

  virtual void draw_image(const Image *image, int x, int y,
                          int y_offset, Frame *dest);
  virtual void draw_frame(int dx, int dy, Frame *dest, int sx, int sy,
                          Frame *src, int w, int h);
  virtual void draw_rect(int x, int y, unsigned int width, unsigned int height,
                         const Video::Color color, Frame *dest);
  virtual void fill_rect(int x, int y, unsigned int width, unsigned int height,
                         const Video::Color color, Frame *dest);
  virtual void draw_line(int x, int y, int x1, int y1,
                         const Video::Color color, Frame *dest);

  virtual void swap_buffers();

  virtual void set_cursor(void *data, unsigned int width,
                          unsigned int height);

  virtual float get_zoom_factor();
  virtual bool set_zoom_factor(float factor);
  virtual void get_screen_factor(float *fx, float *fy);
};

#endif  // SRC_VIDEO_DUMMY_H_