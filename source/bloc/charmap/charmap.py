#! /usr/bin/python

import sys
from PIL import Image
import string

im = Image.open("charmap.png")
size = im.size

assert size[0] == 1024
assert size[1] == 8

limits = [ord(' '), ord('~')]
char_range = range(limits[0], limits[1] + 1)
chars = [chr(number) for number in char_range]
assoc = {}

for k in char_range:
    char = chr(k)
    assoc[char] = []
    for i in range(size[1]):
        for j in range(size[1]):
            assoc[char].append(im.getpixel((k * size[1] + j, i)) == 0)

print "/////////////////////////////////////////////////////////"
print "// This file is generated, please do not edit by hand. //"
print "/////////////////////////////////////////////////////////"
print
print '#include "charmap.h"'
print "static char charmap_start = '%c';" % chars[0]
print "static uint8_t charmap_[%d][64] = " % len(chars)
print "  {"
for key in chars:
    print "  // '%c' = %d, idx = %d  //" \
        % (key, ord(key), ord(key) - ord(chars[0]))
    print "    {"
    print "      ",

    i = 0
    for value in assoc[key]:
        if i != 0 and i % 8 == 0:
            print
            print "      ",
        if value:
            sys.stdout.write("1")
        else:
            sys.stdout.write("0")
        print ",",
        i = i + 1

    print
    print "    },"
    print

print "  };"

print """
uint8_t*
charmap(char c)
{
  return charmap_[c - charmap_start];
}
"""
