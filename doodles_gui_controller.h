#ifndef DOODLES_GUI_CONTROLLER_H
#define DOODLES_GUI_CONTROLLER_H


// Includes
#include<gtk/gtk.h>
#include "canvas/doodles_page.h"
#include "widgets/doodles_container.h"
#include "widgets/doodles_popup_button.h"


// Macros
#define DOODLES_TYPE_GUI_CONTROLLER				( doodles_gui_controller_get_type() )
#define DOODLES_GUI_CONTROLLER(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_GUI_CONTROLLER, DoodlesGuiController) )
#define DOODLES_GUI_CONTROLLER_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_GUI_CONTROLLER, DoodlesGuiControllerClass) )
#define DOODLES_IS_GUI_CONTROLLER(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_GUI_CONTROLLER) )
#define DOODLES_IS_GUI_CONTROLLER_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_GUI_CONTROLLER) )
#define DOODLES_GUI_CONTROLLER_GET_CLASS(obj)	( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_GUI_CONTROLLER, DoodlesGuiControllerClass) )


// Typedefs
typedef struct _DoodlesGuiController		DoodlesGuiController;
typedef struct _DoodlesGuiControllerClass	DoodlesGuiControllerClass;


// Enums
enum tool {
	TOOL_NONE = 0,
	TOOL_PEN,
	TOOL_HIGHLIGHTER,
	TOOL_ERASER,
	TOOL_TEXT,
	TOOL_GRAPH,
	TOOL_SELECTION
};


// Prototypes
GType doodles_gui_controller_get_type();
DoodlesGuiController* doodles_gui_controller_new();
GtkWidget* doodles_gui_controller_get_widget(DoodlesGuiController* self);
void doodles_gui_controller_destroy(DoodlesGuiController* self); // TODO delete later
void doodles_gui_controller_get_color(	DoodlesGuiController*	self,
										GdkRGBA*				color);
gint doodles_gui_controller_get_tool(DoodlesGuiController* self);
gdouble doodles_gui_controller_get_size(DoodlesGuiController* self);

void
doodles_gui_controller_focus_page(	DoodlesGuiController*	self,
									DoodlesPage*			page);



#endif
