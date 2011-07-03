/*
 * lib_image.h
 *
 * ( descripts the module here )
 *
 * Copyright 2011 - Benjamin Bonny    <benjamin.bonny@gmail.com>,
 *                  Cédric Le Ninivin <cedriclen@gmail.com>,
 *                  Guillaume Normand <guillaume.normand.gn@gmail.com>
 *
 * All rights reserved.
 * MB Led
 * Telecom ParisTech - ELECINF344/ELECINF381
 *
 * This file is part of MB Led Project.
 *
 * MB Led Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MB Led Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MB Led Project.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef LIB_IMAGE_H
#define LIB_IMAGE_H

#include "image.h"

void image_direct(image_t img, interface_t north);
void image_set_pixel(image_t img, pixel_t val, int i, int j);
pixel_t image_get_pixel(image_t img, int i, int j);
void image_copy(image_t dst, image_t src);
void image_rotate90_ccw(image_t img);
void image_rotate90_cw(image_t img);
void image_set_pixel(image_t img, pixel_t val, int i, int j);
void image_rotate180(image_t img);
void image_reset(image_t img);
void image_rotate_cw(image_t img, int rot);
void image_rotate_ccw(image_t img, int rot);

#endif
