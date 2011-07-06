#! /usr/bin/python

import sys

if len(sys.argv) != 2:
    print "usage: %s MTU" % sys.argv[0]
    exit(1)

print "#ifndef MTU_H"
print "#define MTU_H"
print "# define MTU %s" % sys.argv[1]
print "#endif"

exit(0)
