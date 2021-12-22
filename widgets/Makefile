CFG = `pkg-config --cflags --libs gtk4`


all:
	make doodles_container
	make doodles_popup_button


doodles_popup_button: doodles_popup_button.c doodles_popup_button.h
	gcc -c $(CFG) doodles_popup_button.c -o ../out/doodles_popup_button.o


doodles_container: doodles_container.c doodles_container.h
	gcc -c $(CFG) doodles_container.c -o ../out/doodles_container.o