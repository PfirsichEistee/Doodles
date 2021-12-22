#ifndef DOODLES_POPUP_BUTTON_H
#define DOODLES_POPUP_BUTTON_H


// INCLUDES //
#include<gtk/gtk.h>


// MACROS //
#define DOODLES_TYPE_POPUP_BUTTON				( doodles_popup_button_get_type() )
#define DOODLES_POPUP_BUTTON(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_POPUP_BUTTON, DoodlesPopupButton) )
#define DOODLES_POPUP_BUTTON_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_POPUP_BUTTON, DoodlesPopupButtonClass) )
#define DOODLES_IS_POPUP_BUTTON(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_POPUP_BUTTON) )
#define DOODLES_IS_POPUP_BUTTON_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_POPUP_BUTTON) )
#define DOODLES_POPUP_BUTTON_GET_CLASS(obj)		( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_POPUP_BUTTON, DoodlesPopupButtonClass) )


// TYPEDEFS //
typedef struct _DoodlesPopupButton			DoodlesPopupButton;
typedef struct _DoodlesPopupButtonClass		DoodlesPopupButtonClass;


// PROTOTYPES //
GType
doodles_popup_button_get_type();

// Creates new popup button
GtkWidget*
doodles_popup_button_new(GtkWidget* popover_child);

// Returns if button is pressed/selected
gboolean
doodles_popup_button_get_state(DoodlesPopupButton* self);

// Sets button pressed-state
void
doodles_popup_button_set_state(	DoodlesPopupButton*	self,
								gboolean			state);

// Gets the child of the popover (-> the widget that was passed at object creation)
GtkWidget*
doodles_popup_button_get_popover_child(DoodlesPopupButton* self);

// Drawing callback for the button-icon
// void draw_func(self, snapshot, width, height);
void
doodles_popup_button_set_draw_func(	DoodlesPopupButton*	self,
									void				(*draw_func)(DoodlesPopupButton*, GtkSnapshot*, gdouble, gdouble));

// Opens or closes popup
void
doodles_popup_button_set_open(	DoodlesPopupButton*	self,
								gboolean			state);

// Is it a toggle-button?
void
doodles_popup_button_set_toggle(	DoodlesPopupButton*	self,
									gboolean			state);



/* SIGNALS:
"toggled": void toggled(DoodlesPopupButton* self);
*/



#endif
