#! /usr/bin/python

import random

f = open('glip.svg', 'r')
content = f.read()
f.close()

f = open('glip1.svg', 'w')

for i in range(64 * 9):
    color = [random.randint(120, 200) for i in range(3)]
    color[random.randint(0, 2)] = 0
    color = ["%x" % c for c in color]
    for i in range(3):
        if len(color[i]) == 1:
            color[i] = "0" + color[i]
    color = color[0] + color[1] + color[2]
    content = content.replace("fill:#005bff", "fill:#" + color, 1)

f.write(content)
f.close()
