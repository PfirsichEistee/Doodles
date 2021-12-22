CFG = `pkg-config --cflags --libs gtk4`


all:
	make doodles_page
	make doodles_canvas


doodles_canvas: doodles_canvas.c doodles_canvas.h
	gcc -c $(CFG) doodles_canvas.c -o ../out/doodles_canvas.o


doodles_page: doodles_page.c doodles_page.h
	gcc -c $(CFG) doodles_page.c -o ../out/doodles_page.o