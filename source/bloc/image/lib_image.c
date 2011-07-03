/*
 * lib_image.c
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

#include <FreeRTOS.h>
#include <task.h>

#include "olsr/olsr_ifaces.h"
#include "lib_image.h"
#include "image.h"

/*
 *
 *  Direct the image up->north
 *
 *
 */
void image_direct(image_t img, interface_t north)
{
  image_rotate_ccw(img, north);
}

/*
 *
 * Reset or init an image (fill it with zeros)
 *
 */
void image_reset(image_t img)
{
  for (int i = 0; i < 64; i++)
    img[i] = 0;
}


/*
 *
 *  Set the pixel (i, j) of img with value
 *
 */
void image_set_pixel(image_t img, pixel_t val, int i, int j)
{
  const int idx = i * 8 + j;
  if ((idx >= 0) && (idx < 64))
    img[idx] = val;
}

/*
 *
 *  Get the pixel (i, j) of img with value
 *
 */
unsigned short image_get_pixel(image_t img, int i, int j)
{
  const int idx = i * 8 + j;
  if ((idx >= 0) && (idx < 64))
    return img[idx];
  return 0;
}


/*
 *
 * Replace dst with src
 *
 */
void image_copy(image_t dst, image_t src)
{
  for (int i = 0; i < 8 * 8; i++)
    dst[i] = src[i];
}

/*
 *
 * rotate img from 90 degrees clockwise
 *
 */
void image_rotate90_cw(image_t img)
{
  for (int j = 0; j < 8 / 2; ++j)
  {
    for (int i = j; i < 8 - 1 - j; ++i)
    {
      pixel_t temp = image_get_pixel(img, j, i);
      image_set_pixel(img, image_get_pixel(img, 8 - 1 - i, j),
                      j, i);
      image_set_pixel(img, image_get_pixel(img, 8 - 1 - j, 8 - 1 - i),
                      8 - 1 - i, j);
      image_set_pixel(img, image_get_pixel(img, i, 8 - 1 - j),
                      8 - 1 - j, 8 - 1 - i);
      image_set_pixel(img, temp, i, 8 - 1 - j);
    }
  }
}

/*
 *
 * rotate img from 90 degrees counter-clockwise
 *
 */
void image_rotate90_ccw(image_t img)
{
  for (int j = 0; j < 8 / 2; ++j)
  {
    for (int i = j; i < 8 - 1 - j; ++i)
    {
      pixel_t temp = image_get_pixel(img, i, j);
      image_set_pixel(img, image_get_pixel(img, j, 8 - 1-i),
                      i, j);
      image_set_pixel(img, image_get_pixel(img, 8 - 1 - i, 8 - 1 - j),
                      j, 8 - 1 - i);
      image_set_pixel(img, image_get_pixel(img, 8 - 1 - j, i),
                      8 - 1 - i, 8 - 1 - j);
      image_set_pixel(img, temp, 8 - 1 - j, i);
    }
  }
}

/*
 *
 * rotate img from 180 degre on the right
 *
 */
void image_rotate180(image_t img)
{
  image_rotate90_cw(img);
  image_rotate90_cw(img);
}


/*
 *
 * This function rotates an image clockwise
 *
 */
void image_rotate_cw(image_t img, int rot) {
  switch (rot % 4)
  {
    case 1:
      image_rotate90_cw(img);
      break;
    case 2:
      image_rotate180(img);
      break;
    case 3:
      image_rotate90_ccw(img);
      break;
    default:
      break;
  }
}

/*
 *
 * This function rotates an image counter-clockwise
 *
 */
void image_rotate_ccw(image_t img, int rot)
{
  image_rotate_cw(img, 4 - (rot % 4));
}
