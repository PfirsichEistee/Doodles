#include "doodles_popup_button.h"


// Structs
struct _DoodlesPopupButtonClass
{
	GtkWidgetClass parent;
	
	// ...
};

struct _DoodlesPopupButton
{
	GtkWidget	parent;
	
	GtkWidget*	popover;
	
	gboolean	selected;
	gboolean	hovering;
	gboolean	toggleable;
	
	void		(*draw_func)(DoodlesPopupButton*, GtkSnapshot*, gdouble, gdouble);
};


// Prototypes
static void snapshot(	GtkWidget*		self_widget,
						GtkSnapshot*	snap);

static GtkSizeRequestMode get_request_mode(GtkWidget* self);
static void measure (	GtkWidget*		self,
						GtkOrientation	orientation,
						gint			for_size,
						gint*			minimum,
						gint*			natural,
						gint*			minimum_baseline,
						gint*			natural_baseline);
static void on_motion_enter(	GtkEventControllerMotion*	controller,
								gdouble						x,
								gdouble						y,
								gpointer					user_data);
static void on_motion_exit(	GtkEventControllerMotion*	controller,
							gdouble						x,
							gdouble						y,
							gpointer					user_data);
static gboolean on_legacy_event(	GtkEventControllerLegacy*	controller,
									GdkEvent*					event,
									gpointer					user_data);


// Variables
static guint signal_id[1];


// Main Functions
static void
dispose(GObject* object)
{
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(object);
	
	g_clear_pointer(&self->popover, gtk_widget_unparent);
}


static void
doodles_popup_button_class_init(	gpointer	klass,
									gpointer	klass_data)
{
	G_OBJECT_CLASS(klass)->dispose = dispose;
	
	GtkWidgetClass* self_class = (GtkWidgetClass*)klass;
	self_class->snapshot = snapshot;
	self_class->get_request_mode = get_request_mode;
	self_class->measure = measure;
	
	
	// This signal will be emitted BEFORE setting "selected = true"
	signal_id[0] = g_signal_new(	"toggle",
									DOODLES_TYPE_POPUP_BUTTON,
									G_SIGNAL_RUN_FIRST,
									0,
									NULL, NULL, // Accumulator
									NULL,
									G_TYPE_NONE,
									0); // "self" will be passed
}


static void
doodles_popup_button_instance_init(	GTypeInstance*	instance,
									gpointer		klass)
{
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(instance);
	
	self->selected = FALSE;
	self->hovering = FALSE;
	self->toggleable = TRUE;
	self->draw_func = NULL;
	
	// Event controllers
	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	g_signal_connect(	motion_controller,
						"enter",
						G_CALLBACK(on_motion_enter),
						NULL);
	g_signal_connect(	motion_controller,
						"leave",
						G_CALLBACK(on_motion_exit),
						NULL);
	
	GtkEventController* legacy_controller = gtk_event_controller_legacy_new();
	g_signal_connect(	legacy_controller,
						"event",
						G_CALLBACK(on_legacy_event),
						NULL);
	
	gtk_widget_add_controller(GTK_WIDGET(self), motion_controller);
	gtk_widget_add_controller(GTK_WIDGET(self), legacy_controller);
	
	// Popover
	GtkWindow* window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)));
	
	self->popover = gtk_popover_new();
	gtk_widget_set_parent(self->popover, GTK_WIDGET(self));
}


GType
doodles_popup_button_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(DoodlesPopupButtonClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_popup_button_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(DoodlesPopupButton),
			.instance_init	= doodles_popup_button_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	GTK_TYPE_WIDGET,
										"DoodlesPopupButton",
										&info,
										0);
	}
	
	return type;
}


// Virtual functions
static GtkSizeRequestMode
get_request_mode(GtkWidget* self)
{
	return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
measure(	GtkWidget*		self,
			GtkOrientation	orientation,
			gint			for_size,
			gint*			minimum,
			gint*			natural,
			gint*			minimum_baseline,
			gint*			natural_baseline)
{
	*minimum = 0;
	*natural = 0;
	
	if (for_size == -1)
	{
		*minimum = 16;
		*natural = 16;
		return;
	}
	
	
	*minimum = for_size;
	*natural = for_size;
}

static void
snapshot(	GtkWidget*		self_widget,
			GtkSnapshot*	snap)
{
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(self_widget);
	
	gdouble w = gtk_widget_get_width(self_widget);
	gdouble h = gtk_widget_get_height(self_widget);
	
	
	// Draw
	GdkRGBA color;
	gdk_rgba_parse(&color, "#44444488");
	
	for (guint i = 1; i <= (self->hovering + self->selected); i++)
	if (self->selected)
		gtk_snapshot_append_color(	snap,
									&color,
									&GRAPHENE_RECT_INIT(0, 0, w, h));
	
	
	if (self->draw_func != NULL)
	{
		gtk_snapshot_save(snap);
		
		gtk_snapshot_translate(	snap,
								&GRAPHENE_POINT_INIT(0.2 * w, 0.2 * h));
		self->draw_func(self, snap, 0.6 * w, 0.6 * h);
		
		gtk_snapshot_restore(snap);
	}
	
	
	gdk_rgba_parse(&color, "#BBBBBB44");
	
	if (self->hovering)
		gtk_snapshot_append_color(	snap,
									&color,
									&GRAPHENE_RECT_INIT(0, 0, w, h));
}


// Constructor
GtkWidget*
doodles_popup_button_new(GtkWidget* popover_child)
{
	GObject* new = g_object_new(DOODLES_TYPE_POPUP_BUTTON, NULL);
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(new);
	
	gtk_popover_set_child(GTK_POPOVER(self->popover), popover_child);
	
	return GTK_WIDGET(new);
}


// Events
static void
on_motion_enter(	GtkEventControllerMotion*	controller,
					gdouble						x,
					gdouble						y,
					gpointer					user_data)
{
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
	
	self->hovering = TRUE;
	
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void
on_motion_exit(	GtkEventControllerMotion*	controller,
				gdouble						x,
				gdouble						y,
				gpointer					user_data)
{
	DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
	
	self->hovering = FALSE;
	
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

static gboolean
on_legacy_event(	GtkEventControllerLegacy*	controller,
					GdkEvent*					event,
					gpointer					user_data)
{
	if (gdk_event_get_event_type(event) == GDK_BUTTON_PRESS && gdk_button_event_get_button(event) == 1)
	{
		DoodlesPopupButton* self = DOODLES_POPUP_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
		
		if (self->selected || !self->toggleable)
		{
			// Already opened?
			if (gtk_widget_get_visible(self->popover))
			{
				// Close popup popup
				gtk_popover_popdown(GTK_POPOVER(self->popover));
				return FALSE;
			}
			
			// Open popup popup
			gtk_popover_popup(GTK_POPOVER(self->popover));
		}
		
		
		// Call signal
		/*g_signal_emit_by_name(	G_OBJECT(self),
								"toggle",
								self);*/
		g_signal_emit(	self,
						signal_id[0],
						0);
		
		
		// Set selected
		if (self->toggleable)
			self->selected = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(self));
		
		return TRUE;
	}
	
	return FALSE;
}


// Else

void
doodles_popup_button_set_state(	DoodlesPopupButton*	self,
								gboolean			state)
{
	self->selected = state;
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

gboolean
doodles_popup_button_get_state(DoodlesPopupButton* self)
{
	return self->selected;
}

GtkWidget*
doodles_popup_button_get_popover_child(DoodlesPopupButton* self)
{
	return gtk_popover_get_child(GTK_POPOVER(self->popover));
}

void
doodles_popup_button_set_draw_func(	DoodlesPopupButton*	self,
									void				(*draw_func)(DoodlesPopupButton*, GtkSnapshot*, gdouble, gdouble))
{
	self->draw_func = draw_func;
}

void
doodles_popup_button_set_open(	DoodlesPopupButton*	self,
								gboolean			state)
{
	if (state)
		gtk_popover_popup(GTK_POPOVER(self->popover));
	else
		gtk_popover_popdown(GTK_POPOVER(self->popover));
}

void
doodles_popup_button_set_toggle(	DoodlesPopupButton*	self,
									gboolean			state)
{
	self->toggleable = state;
}













