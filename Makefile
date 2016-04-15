all: highlight

clean:
	rm -f highlight bmp.o color.o font.o screen.o

bmp.o: bmp.c color.h types.h
	cc -Wall -o3 -o bmp.o -c bmp.c

color.o: color.c color.h types.h
	cc -Wall -o3 -o color.o -c color.c

font.o: font.c types.h monaco_compressed_large.h
	cc -Wall -o3 -o font.o -c font.c

screen.o: screen.c types.h
	cc -Wall -o3 -o screen.o -c screen.c

highlight: main.c color.o bmp.o font.o color.h screen.o types.h
	cc -Wall -o3 -o highlight main.c color.o bmp.o font.o screen.o
