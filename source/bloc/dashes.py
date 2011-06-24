#! /usr/bin/python

import sys

if len(sys.argv) != 2:
    print("usage: %s N" % sys.argv[0])
    exit(1)

n = int(sys.argv[1])

print('#ifndef DASHES_H')
print('# define DASHES_H')

print('# define DASHES(N) DASHES_##N')
print('# define DASHES_0')
for i in range(1, n + 1):
    print('# define DASHES_%d "%s"' % (i, "-" * i))

print('#endif')
