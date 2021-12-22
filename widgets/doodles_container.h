#ifndef DOODLES_CONTAINER_H
#define DOODLES_CONTAINER_H


// Includes
#include <gtk/gtk.h>
#include "../canvas/doodles_page.h"
#include "../canvas/doodles_canvas.h"


// Macros
#define DOODLES_TYPE_CONTAINER				( doodles_container_get_type() )
#define DOODLES_CONTAINER(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_CONTAINER, DoodlesContainer) )
#define DOODLES_CONTAINER_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_CONTAINER, DoodlesContainerClass) )
#define DOODLES_IS_CONTAINER(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_CONTAINER) )
#define DOODLES_IS_CONTAINER_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_CONTAINER) )
#define DOODLES_CONTAINER_GET_CLASS(obj)	( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_CONTAINER, DoodlesContainerClass) )


// Typedefs
typedef struct _DoodlesContainer			DoodlesContainer;
typedef struct _DoodlesContainerClass		DoodlesContainerClass;


// Prototypes
GType
doodles_container_get_type();

GtkWidget*
doodles_container_new();

void
doodles_container_insert(	DoodlesContainer*	self,
							GtkWidget*			child);

void
doodles_container_set_scrolled_window(	DoodlesContainer*	self,
										GtkScrolledWindow*	p_scroll_window);


#endif