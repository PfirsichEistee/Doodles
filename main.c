#include <gtk/gtk.h>
#include "doodles_gui_controller.h"


/* PROTOTYPES */

static void
application_activate(	GtkApplication*	app,
						gpointer		user_data);
static gboolean
application_end(	GtkApplication*	app,
					gpointer		user_data);


/* VARIABLES */


/* FUNCTIONS */

int main(	int argc,
			char** arg)
{
	GtkApplication* app = gtk_application_new("com.pfirsicheistee.doodles", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(application_activate), NULL);
	
	int status = g_application_run(G_APPLICATION(app), argc, arg);
	
	g_object_unref(app);
	
	return status;
}


static void
bs_test(GtkButton* self)
{
	printf("ACTIVATE\n");
}


static void
application_activate(	GtkApplication*	app,
						gpointer		user_data)
{
	GtkWidget* main_window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(main_window), "Doodles");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
	g_signal_connect(main_window, "close-request", G_CALLBACK(application_end), NULL);
	
	
	DoodlesGuiController* gui_controller = doodles_gui_controller_new();
	GtkWidget* controller_widget = doodles_gui_controller_get_widget(gui_controller);
	gtk_window_set_child(GTK_WINDOW(main_window), controller_widget);
	
	
	gtk_widget_show(main_window);
}


static gboolean
application_end(	GtkApplication*	app,
					gpointer		user_data)
{
	return FALSE;
}