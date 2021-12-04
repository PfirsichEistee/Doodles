CFG = `pkg-config --cflags --libs libadwaita-1`


all:
	make doodles_canvas
	make doodles_page


doodles_canvas: doodles_canvas.c doodles_canvas.h
	gcc -c $(CFG) doodles_canvas.c -o ../out/doodles_canvas.o


doodles_page: doodles_page.c doodles_page.h
	gcc -c $(CFG) doodles_page.c -o ../out/doodles_page.o