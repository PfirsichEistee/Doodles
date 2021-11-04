#ifndef DOODLES_COLOR_BUTTON_H
#define DOODLES_COLOR_BUTTON_H


// Includes
#include<adwaita.h>


// Macros
#define DOODLES_TYPE_COLOR_BUTTON				( doodles_color_button_get_type() )
#define DOODLES_COLOR_BUTTON(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_COLOR_BUTTON, DoodlesColorButton) )
#define DOODLES_COLOR_BUTTON_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_COLOR_BUTTON, DoodlesColorButtonClass) )
#define DOODLES_IS_COLOR_BUTTON(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_COLOR_BUTTON) )
#define DOODLES_IS_COLOR_BUTTON_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_COLOR_BUTTON) )
#define DOODLES_COLOR_BUTTON_GET_CLASS(obj)		( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_COLOR_BUTTON, DoodlesColorButtonClass) )


// Typedefs
typedef struct _DoodlesColorButton			DoodlesColorButton;
typedef struct _DoodlesColorButtonClass		DoodlesColorButtonClass;


// Prototypes
GType doodles_color_button_get_type();
GtkWidget* doodles_color_button_new(gdouble r, gdouble g, gdouble b);
gboolean doodles_color_button_get_state(DoodlesColorButton* self);
void doodles_color_button_set_state(	DoodlesColorButton*	self,
										gboolean			state);
void doodles_color_button_get_color(	DoodlesColorButton*	self,
										GdkRGBA*			color);


#endif