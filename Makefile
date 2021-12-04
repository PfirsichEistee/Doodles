CFG = `pkg-config --cflags --libs libadwaita-1`


all:
	make -C widgets
	make -C canvas
	make doodles


doodles: main doodles_gui_controller
	gcc $(CFG) out/*.o -o doodles -lm -ldl


main: main.c
	gcc -c $(CFG) main.c -o out/main.o


doodles_gui_controller: doodles_gui_controller.c doodles_gui_controller.h
	gcc -c $(CFG) doodles_gui_controller.c -o out/doodles_gui_controller.o


clean:
	rm -r out/*.o
