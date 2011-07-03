#ifndef IMAGE_H
# define IMAGE_H

typedef unsigned short pixel_t;
typedef pixel_t* image_t;
#define R(rgb) (17 * ((rgb >> 8) & 0xf))
#define G(rgb) (17 * ((rgb >> 4) & 0xf))
#define B(rgb) (17 * ((rgb >> 0) & 0xf))
#define PIXEL(Image, I, J) (Image[(I) * 8 + (J)])

#endif
