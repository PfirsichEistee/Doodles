#ifndef DOODLES_CANVAS_H
#define DOODLES_CANVAS_H


// Includes
#include<adwaita.h>
#include "../misc/datastructs.h"


// Macros
#define DOODLES_TYPE_CANVAS				( doodles_canvas_get_type() )
#define DOODLES_CANVAS(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_CANVAS, DoodlesCanvas) )
#define DOODLES_CANVAS_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_CANVAS, DoodlesCanvasClass) )
#define DOODLES_IS_CANVAS(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_CANVAS) )
#define DOODLES_IS_CANVAS_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_CANVAS) )
#define DOODLES_CANVAS_GET_CLASS(obj)	( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_CANVAS, DoodlesCanvasClass) )

#define PIXEL_PER_CM 32


// Typedefs
typedef struct _DoodlesCanvas		DoodlesCanvas;
typedef struct _DoodlesCanvasClass	DoodlesCanvasClass;


// Prototypes
GType doodles_canvas_get_type();
DoodlesCanvas* doodles_canvas_new(	gdouble pWidth,
									gdouble pHeight);
void doodles_canvas_set_child(	DoodlesCanvas*	self,
								DoodlesCanvas*	child);
DoodlesCanvasClass* doodles_canvas_get_class();
void doodles_canvas_set_draw(	DoodlesCanvas* self,
								gboolean	(*draw)(	GtkWidget*		self,
														GtkSnapshot*	snap,
														gpointer		user_data),
								gpointer	user_data);
gdouble doodles_canvas_get_pixel_per_cm();
gdouble doodles_canvas_get_width(DoodlesCanvas* self);
gdouble doodles_canvas_get_height(DoodlesCanvas* self);
void doodles_canvas_draw_circle(	cairo_t* cairo,
									gdouble x, gdouble y,
									gdouble radius,
									gboolean fill,
									gdouble r, gdouble g, gdouble b, gdouble a);
void doodles_canvas_draw_line(	cairo_t* cairo,
								gdouble x1, gdouble y1,
								gdouble x2, gdouble y2);
void
doodles_canvas_add_line(	DoodlesCanvas*	self,
							STR_POINT**		point_list,
							guint			point_list_size,
							gdouble			size,
							gdouble r, gdouble g, gdouble b, gdouble a);



#endif