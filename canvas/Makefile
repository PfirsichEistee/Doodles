ADWAITA = `pkg-config --cflags --libs libadwaita-1`


all: doodles_canvas.o doodles_page.o doodles_color_button.o
	


doodles_canvas.o: doodles_canvas.c doodles_canvas.h
	gcc -c $(ADWAITA) doodles_canvas.c


doodles_page.o: doodles_page.c doodles_page.h
	gcc -c $(ADWAITA) doodles_page.c


doodles_color_button.o: doodles_color_button.c doodles_color_button.h
	gcc -c $(ADWAITA) doodles_color_button.c