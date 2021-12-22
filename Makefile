CFG = `pkg-config --cflags --libs gtk4`


all:
	make -C canvas
	make -C widgets
	make doodles


doodles: main doodles_gui_controller
	gcc $(CFG) out/*.o -o doodles -lm -ldl


main: main.c
	gcc -c $(CFG) main.c -o out/main.o


doodles_gui_controller: doodles_gui_controller
	gcc -c $(CFG) doodles_gui_controller.c -o out/doodles_gui_controller.o