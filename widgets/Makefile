CFG = `pkg-config --cflags --libs libadwaita-1`


all:
	make doodles_popup_button


doodles_popup_button: doodles_popup_button.c doodles_popup_button.h
	gcc -c $(CFG) doodles_popup_button.c -o ../out/doodles_popup_button.o