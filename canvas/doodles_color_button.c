#include "doodles_color_button.h"


// Structs
struct _DoodlesColorButtonClass
{
	GtkWidgetClass parent;
	
	// ...
};

struct _DoodlesColorButton
{
	GtkWidget	parent;
	
	GtkWidget*	popover;
	
	gboolean	selected;
	gboolean	hovering;
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
static void on_color_activated (	GtkColorChooser*	color_chooser,
									GdkRGBA*			color,
									gpointer			user_data);


// Variables
static guint signal_id[1];


// Main Functions
static void
dispose(GObject* object)
{
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(object);
	
	g_clear_pointer(&self->popover, gtk_widget_unparent);
}


static void
doodles_color_button_class_init(	gpointer	klass,
									gpointer	klass_data)
{
	G_OBJECT_CLASS(klass)->dispose = dispose;
	
	GtkWidgetClass* self_class = (GtkWidgetClass*)klass;
	self_class->snapshot = snapshot;
	self_class->get_request_mode = get_request_mode;
	self_class->measure = measure;
	
	
	// This signal will be emitted BEFORE setting "selected = true"
	signal_id[0] = g_signal_new(	"toggle",
									DOODLES_TYPE_COLOR_BUTTON,
									G_SIGNAL_RUN_FIRST,
									0,
									NULL, NULL, // Accumulator
									NULL,
									G_TYPE_NONE,
									0); // "self" will be passed
}


static void
doodles_color_button_instance_init(	GTypeInstance*	instance,
									gpointer		klass)
{
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(instance);
	
	self->selected = FALSE;
	self->hovering = FALSE;
	
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
	//gtk_popover_set_autohide(GTK_POPOVER(self->popover), FALSE);
	
	GtkWidget* color_chooser = gtk_color_chooser_widget_new();
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(color_chooser), FALSE);
	gtk_popover_set_child(GTK_POPOVER(self->popover), color_chooser);
	
	g_signal_connect(	color_chooser,
						"color-activated",
						G_CALLBACK(on_color_activated),
						self);
}


GType
doodles_color_button_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(DoodlesColorButtonClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_color_button_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(DoodlesColorButton),
			.instance_init	= doodles_color_button_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	GTK_TYPE_WIDGET,
										"DoodlesColorButton",
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
measure (	GtkWidget*		self,
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
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(self_widget);
	
	gdouble w = gtk_widget_get_width(self_widget);
	gdouble h = gtk_widget_get_height(self_widget);
	
	GtkWidget* color_chooser = gtk_popover_get_child(GTK_POPOVER(self->popover));
	
	
	
	
	
	GdkRGBA grey, black;
	gdk_rgba_parse(&grey, "#BBBBBB44");
	gdk_rgba_parse(&black, "#44444488");
	
	for (guint i = 1; i <= (self->hovering + self->selected); i++)
	if (self->selected)
		gtk_snapshot_append_color(	snap,
									&black,
									&GRAPHENE_RECT_INIT(0, 0, w, h));
	
	
	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_chooser), &color);
	
	gtk_snapshot_append_color(	snap,
								&color,
								&GRAPHENE_RECT_INIT(0.2 * w, 0.2 * h, 0.6 * w, 0.6 * h));
	
	
	if (self->hovering)
		gtk_snapshot_append_color(	snap,
									&grey,
									&GRAPHENE_RECT_INIT(0, 0, w, h));
}


// Constructor
GtkWidget*
doodles_color_button_new(	gdouble		r,
							gdouble		g,
							gdouble		b)
{
	GObject* new = g_object_new(DOODLES_TYPE_COLOR_BUTTON, NULL);
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(new);
	
	GtkWidget* color_chooser = gtk_popover_get_child(GTK_POPOVER(self->popover));
	
	GdkRGBA* color = malloc(sizeof(GdkRGBA));
	color->red = r;
	color->green = g;
	color->blue = b;
	color->alpha = 1.0;
	
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_chooser), color);
	
	return GTK_WIDGET(new);
}


// Events
static void
on_motion_enter(	GtkEventControllerMotion*	controller,
					gdouble						x,
					gdouble						y,
					gpointer					user_data)
{
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
	
	self->hovering = TRUE;
	
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void
on_motion_exit(	GtkEventControllerMotion*	controller,
				gdouble						x,
				gdouble						y,
				gpointer					user_data)
{
	DoodlesColorButton* self = DOODLES_COLOR_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
	
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
		DoodlesColorButton* self = DOODLES_COLOR_BUTTON(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller)));
		
		if (self->selected)
		{
			// Already opened?
			if (gtk_widget_get_visible(self->popover))
			{
				// Close color popup
				gtk_popover_popdown(GTK_POPOVER(self->popover));
				return FALSE;
			}
			
			// Open color popup
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
		self->selected = TRUE;
		gtk_widget_queue_draw(GTK_WIDGET(self));
		
		return TRUE;
	}
	
	return FALSE;
}

static void
on_color_activated (	GtkColorChooser*	color_chooser,
						GdkRGBA*			color,
						gpointer			user_data)
{
	DoodlesColorButton* self = (DoodlesColorButton*)user_data;
	
	gtk_popover_popdown(GTK_POPOVER(self->popover));
	
	gtk_widget_queue_draw(GTK_WIDGET(self));
}


// Else

void
doodles_color_button_set_state(	DoodlesColorButton*	self,
								gboolean			state)
{
	self->selected = state;
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

gboolean
doodles_color_button_get_state(DoodlesColorButton* self)
{
	return self->selected;
}

void
doodles_color_button_get_color(	DoodlesColorButton*	self,
								GdkRGBA*			color)
{
	GtkWidget* color_chooser = gtk_popover_get_child(GTK_POPOVER(self->popover));
	
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_chooser), color);
}