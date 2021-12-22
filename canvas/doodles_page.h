#ifndef DOODLES_PAGE_H
#define DOODLES_PAGE_H


// Macros
#define DOODLES_TYPE_PAGE				( doodles_page_get_type() )
#define DOODLES_PAGE(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_PAGE, DoodlesPage) )
#define DOODLES_PAGE_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_PAGE, DoodlesPageClass) )
#define DOODLES_IS_PAGE(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_PAGE) )
#define DOODLES_IS_PAGE_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_PAGE) )
#define DOODLES_PAGE_GET_CLASS(obj)		( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_PAGE, DoodlesPageClass) )


// Typedefs
typedef struct _DoodlesPage				DoodlesPage;
typedef struct _DoodlesPageClass		DoodlesPageClass;
typedef struct _DoodlesGuiController	DoodlesGuiController;


// Includes
#include<gtk/gtk.h>
#include "doodles_canvas.h"
#include "../doodles_gui_controller.h"
#include "../misc/datastructs.h"


// Prototypes
GType
doodles_page_get_type();

DoodlesPage*
doodles_page_new(	DoodlesGuiController*	controller,
								gdouble					pWidth,
								gdouble					pHeight);

GtkWidget*
doodles_page_get_widget(DoodlesPage* self);

void
doodles_page_config_set_color(gdouble r, gdouble g, gdouble b);

void
doodles_page_config_set_size(gdouble pSize);

void
doodles_page_receive_event(	DoodlesPage*	self,
							gdouble			mouse_x,
							gdouble			mouse_y,
							gint			gdk_event_button,
							gint			gdk_event_type);


#endif