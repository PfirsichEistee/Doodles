ADWAITA = `pkg-config --cflags --libs libadwaita-1`


all:
	make -C misc
	make -C canvas
	make doodles


doodles: main.o doodles_gui_controller.o canvas/doodles_canvas.o canvas/doodles_page.o canvas/doodles_color_button.o
	gcc $(ADWAITA) main.o doodles_gui_controller.o canvas/doodles_canvas.o canvas/doodles_page.o canvas/doodles_color_button.o -o doodles


main.o: main.c
	gcc -c $(ADWAITA) main.c


doodles_gui_controller.o: doodles_gui_controller.c doodles_gui_controller.h
	gcc -c $(ADWAITA) doodles_gui_controller.c


clean:
	rm -r *.o
