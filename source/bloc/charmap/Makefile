all: charmap.c

charmap.c: charmap.py charmap.png
	@if [ -f $@ ]; then chmod +w $@; fi
	./$< > $@
	@chmod a-w $@
