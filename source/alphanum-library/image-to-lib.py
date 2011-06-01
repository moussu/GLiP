#! /usr/bin/python

from PIL import Image
import string

im = Image.open("8x8font.png")
size = im.size

assert size[0] == 1024
assert size[1] == 8

limits = [ord('!'), ord('~')]
char_range = range(limits[0], limits[1] + 1)
chars = [chr(number) for number in char_range]
assoc = {}

for k in char_range:
    char = chr(k)
    assoc[char] = []
    for i in range(size[1]):
        for j in range(size[1]):
            print (k * size[1] + j, i)
            assoc[char].append(im.getpixel((k * size[1] + j, i)) == 0)

for key in assoc:
    i = 0
    print key + ":"
    for value in assoc[key]:
        if value:
            print "O",
        else:
            print " ",
        if i % 8 == 0:
            print
        i = i + 1
    print
    print
    print
    print

