#ifndef DOODLES_GUI_CONTROLLER_H
#define DOODLES_GUI_CONTROLLER_H


// Includes
#include<adwaita.h>
#include "canvas/doodles_page.h"
#include "canvas/doodles_canvas.h"
#include "canvas/doodles_color_button.h"


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


// Prototypes
GType doodles_gui_controller_get_type();
DoodlesGuiController* doodles_gui_controller_new();
GtkWidget* doodles_gui_controller_get_widget(DoodlesGuiController* self);
void doodles_gui_controller_destroy(DoodlesGuiController* self); // TODO delete later
void doodles_gui_controller_get_color(	DoodlesGuiController*	self,
										GdkRGBA*				color);
gdouble doodles_gui_controller_get_size(DoodlesGuiController* self);



#endif