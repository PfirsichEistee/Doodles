#include "doodles_text_field.h"



// Macros
#define TF_MIN_WIDTH(klass) ((klass)->space * 8)



// Structs
typedef struct _DragInfo
{
	gshort		drag_type;
	/*	0	drag title
		1	resize left edge
		2	resize right edge	*/
	gint		local_x;
	gint		local_y;
	gboolean	is_down;
} DragInfo;

struct _DoodlesTextFieldClass
{
	GtkWidgetClass parent;
	
	// Private static attributes
	gint 	title_height;
	gint	space;
};

struct _DoodlesTextField
{
	GtkWidget parent;
	
	// Private attributes
	gfloat			width;
	gfloat			height;
	gboolean		mouse_is_over;
	GtkTextView*	text_view;
	DragInfo		drag_info;
	GtkFixed*		phFixed; // debug; delete later
};


// Prototypes
static void snapshot(	GtkWidget*		self,
						GtkSnapshot*	snap);

static GtkSizeRequestMode get_request_mode(GtkWidget* self);
static void measure (	GtkWidget*		self,
						GtkOrientation	orientation,
						gint			for_size,
						gint*			minimum,
						gint*			natural,
						gint*			minimum_baseline,
						gint*			natural_baseline);

static void on_focus_enter(	GtkEventControllerFocus*	event_controller,
							gpointer					user_data);
static void on_focus_leave(	GtkEventControllerFocus*	event_controller,
							gpointer					user_data);
static gboolean on_legacy_event(	GtkEventControllerLegacy*	event_controller,
									GdkEvent*					event,
									gpointer					user_data);


static void get_content_transform(	DoodlesTextField*	self,
									gint*				px,
									gint*				py,
									gint*				pwidth,
									gint*				pheight);
static GtkAllocation get_content_transform_rectangle(DoodlesTextField* self);



// Main Functions
static void dispose(GObject* object)
{
	DoodlesTextField* tf_object = DOODLES_TEXT_FIELD(object);
	GtkWidget* tv_widget = GTK_WIDGET(tf_object->text_view);
	
	gtk_widget_unparent(tv_widget);
}


static void
doodles_text_field_class_init(	gpointer	klass,
								gpointer	klass_data)
{
	G_OBJECT_CLASS(klass)->dispose = dispose;
	
	GtkWidgetClass* self_class = (GtkWidgetClass*)klass;
	self_class->snapshot = snapshot;
	self_class->get_request_mode = get_request_mode;
	self_class->measure = measure;
	
	DoodlesTextFieldClass* tf_class = DOODLES_TEXT_FIELD_CLASS(klass);
	tf_class->title_height = 20;
	tf_class->space = 10;
}


static void
doodles_text_field_instance_init(	GTypeInstance*	instance,
									gpointer		klass)
{
	DoodlesTextField* self = DOODLES_TEXT_FIELD(instance);
	
	//self->width = 100;
	self->width = TF_MIN_WIDTH(DOODLES_TEXT_FIELD_CLASS(klass));
	self->height = 60;
	self->mouse_is_over = FALSE;
	
	DragInfo ph = {
		.drag_type	= 0,
		.local_x	= 0,
		.local_y	= 0,
		.is_down	= FALSE
	};
	self->drag_info = ph;
	
	// Text view
	self->text_view = GTK_TEXT_VIEW(gtk_text_view_new());
	gtk_widget_set_parent(GTK_WIDGET(self->text_view), GTK_WIDGET(self));
	
	
	// Events
	GtkEventController* motion_controller = gtk_event_controller_motion_new();
	GtkEventController* legacy_controller = gtk_event_controller_legacy_new();
	g_signal_connect(	motion_controller,
						"enter",
						G_CALLBACK(on_focus_enter),
						NULL);
	
	g_signal_connect(	motion_controller,
						"leave",
						G_CALLBACK(on_focus_leave),
						NULL);
	
	g_signal_connect(	legacy_controller,
						"event",
						G_CALLBACK(on_legacy_event),
						NULL);
	
	gtk_widget_add_controller(GTK_WIDGET(instance), motion_controller);
	gtk_widget_add_controller(GTK_WIDGET(instance), legacy_controller);
}


GType
doodles_text_field_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(struct _DoodlesTextFieldClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_text_field_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(struct _DoodlesTextField),
			.instance_init	= doodles_text_field_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	GTK_TYPE_WIDGET,
										"DoodlesTextField",
										&info,
										0);
	}
	
	return type;
}


// Constructor
GtkWidget*
doodles_text_field_new()
{
	GObject* new = g_object_new(DOODLES_TYPE_TEXT_FIELD, NULL);
	
	return GTK_WIDGET(new);
}


// GtkWidget virtual functions

static GtkSizeRequestMode
get_request_mode(GtkWidget* self)
{
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
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
	DoodlesTextField* tf_self = DOODLES_TEXT_FIELD(self);
	DoodlesTextFieldClass* klass = DOODLES_TEXT_FIELD_GET_CLASS(tf_self);
	GtkWidget* text_view = GTK_WIDGET(tf_self->text_view);
	
	gint tf_minimum, tf_natural;
	gint tf_minimum_baseline, tf_natural_baseline;
	gtk_widget_measure(	text_view,
						orientation,
						-1,
						&tf_minimum,
						&tf_natural,
						&tf_minimum_baseline,
						&tf_natural_baseline);
	
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		//tf_self->width = tf_minimum + klass->space * 5;
		
		*minimum = tf_self->width;
		*natural = tf_self->width;
		
		//*minimum_baseline = tf_self->width;
		//*natural_baseline = tf_self->width;
	}
	else
	{
		tf_self->height = tf_natural + klass->space * 2 + klass->title_height;
		
		*minimum = tf_self->height;
		*natural = tf_self->height;
		
		//*minimum_baseline = tf_self->height;
		//*natural_baseline = tf_self->height;
	}
	
	
	// Text view
	const GtkAllocation alloc = get_content_transform_rectangle(DOODLES_TEXT_FIELD(self));
	
	gtk_widget_size_allocate(	GTK_WIDGET(DOODLES_TEXT_FIELD(self)->text_view),
								&alloc,
								-1);
}


static void
snapshot(	GtkWidget*		self,
			GtkSnapshot*	snap)
{
	DoodlesTextFieldClass* tf_class = DOODLES_TEXT_FIELD_GET_CLASS(self);
	float w = gtk_widget_get_width(self);
	float h = gtk_widget_get_height(self);
	
	
	if (!DOODLES_TEXT_FIELD(self)->mouse_is_over &&
			!gtk_widget_has_focus(GTK_WIDGET(DOODLES_TEXT_FIELD(self)->text_view)))
		goto skipdraw;
	
	
	GdkRGBA color_out = { 0.7f, 0.7f, 0.7f, 1.0f };
	GdkRGBA color_tit = { 0.7f, 0.7f, 0.7f, 0.7f };
	
	
	// Draw outline
	GskRoundedRect outline, title_outline;
	gsk_rounded_rect_init_from_rect(	&outline,
										&GRAPHENE_RECT_INIT(0, 0, w, h),
										0);
	gsk_rounded_rect_init_from_rect(	&title_outline,
										&GRAPHENE_RECT_INIT(0, 0, w, tf_class->title_height),
										0);
	
	static const float border_width[] = { 2.0f, 2.0f, 2.0f, 2.0f };
	GdkRGBA outline_color[] = { color_out, color_out, color_out, color_out };
	
	gtk_snapshot_append_border(	snap,
								&outline,
								border_width,
								outline_color);
	
	gtk_snapshot_append_border(	snap,
								&title_outline,
								border_width,
								outline_color);
	
	// Draw title
	gtk_snapshot_append_color(	snap,
								&color_tit,
								&GRAPHENE_RECT_INIT(2, 2, w - 4, tf_class->title_height - 4));
	
	
	skipdraw:
	
	
	// Draw text view
	gtk_widget_snapshot_child(self, GTK_WIDGET(DOODLES_TEXT_FIELD(self)->text_view), snap);
	
	//printf("Draw %lf %lf \n", w, h);
}



// Events

static void
on_focus_enter(	GtkEventControllerFocus*	event_controller,
				gpointer					user_data)
{
	GtkWidget* self = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(event_controller));
	
	DOODLES_TEXT_FIELD(self)->mouse_is_over = TRUE;
	
	gtk_widget_queue_draw(self);
}

static void
on_focus_leave(	GtkEventControllerFocus*	event_controller,
				gpointer					user_data)
{
	GtkWidget* self = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(event_controller));
	
	DOODLES_TEXT_FIELD(self)->mouse_is_over = FALSE;
	
	gtk_widget_queue_draw(self);
}

static gboolean
on_legacy_event(	GtkEventControllerLegacy*	event_controller,
					GdkEvent*					event,
					gpointer					user_data)
{
	switch (gdk_event_get_event_type(event))
	{
		case (GDK_BUTTON_PRESS):
		case (GDK_BUTTON_RELEASE):
			if (gdk_button_event_get_button(event) == 1)
			{
				DoodlesTextField* self = DOODLES_TEXT_FIELD(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(event_controller)));
				GtkFixed* fixed_parent = GTK_FIXED(gtk_widget_get_parent(GTK_WIDGET(self)));
				
				gtk_widget_grab_focus(GTK_WIDGET(self->text_view));
				
				// Get local position
				gdouble event_x, event_y;
				gdk_event_get_position(event, &event_x, &event_y);
				
				gdouble tf_x, tf_y;
				gtk_fixed_get_child_position(fixed_parent, GTK_WIDGET(self), &tf_x, &tf_y);
				
				self->drag_info.local_x = (int)(event_x - tf_x);
				self->drag_info.local_y = (int)(event_y - tf_y);
				
				// Set is_down
				self->drag_info.is_down = TRUE;
				if (gdk_event_get_event_type(event) == GDK_BUTTON_RELEASE)
				{
					self->drag_info.is_down = FALSE;
				}
				else
				{
					// Get drag type
					DoodlesTextFieldClass* klass = DOODLES_TEXT_FIELD_GET_CLASS(self);
					
					if (self->drag_info.local_y <= klass->title_height)
					{
						self->drag_info.drag_type = 0;
					}
					else if (self->drag_info.local_x <= klass->space)
					{
						self->drag_info.drag_type = 1;
						
						self->drag_info.local_x = (int)tf_x + self->width;
					}
					else if (self->drag_info.local_x >= (self->width - klass->space))
					{
						self->drag_info.drag_type = 2;
					}
					else
					{
						self->drag_info.is_down = FALSE;
					}
				
					// Raise widget inside fixed
					//gtk_fixed_remove(fixed_parent, GTK_WIDGET(self));
					//gtk_fixed_put(fixed_parent, GTK_WIDGET(self), tf_x, tf_y);
					gtk_widget_grab_focus(GTK_WIDGET(self->text_view));
				}
				
				printf("Local pos: %d %d \n", self->drag_info.local_x, self->drag_info.local_y);
			}
			
			break;
		case (GDK_MOTION_NOTIFY):
			DoodlesTextField* self = DOODLES_TEXT_FIELD(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(event_controller)));
			
			if (self->drag_info.is_down)
			{
				GtkFixed* fixed_parent = GTK_FIXED(gtk_widget_get_parent(GTK_WIDGET(self)));
				
				gdouble event_x, event_y;
				gdk_event_get_position(event, &event_x, &event_y);
				
				gdouble tf_x, tf_y;
				gtk_fixed_get_child_position(fixed_parent, GTK_WIDGET(self), &tf_x, &tf_y);
				
				if (self->drag_info.drag_type == 0)
				{
					gtk_fixed_move(	fixed_parent,
									GTK_WIDGET(self),
									(int)event_x - self->drag_info.local_x,
									(int)event_y - self->drag_info.local_y);
				}
				else if (self->drag_info.drag_type == 1)
				{
					gint w = self->drag_info.local_x - event_x;
					if (w < TF_MIN_WIDTH(DOODLES_TEXT_FIELD_GET_CLASS(self)))
						w = TF_MIN_WIDTH(DOODLES_TEXT_FIELD_GET_CLASS(self));
					
					gtk_fixed_move(	fixed_parent,
									GTK_WIDGET(self),
									self->drag_info.local_x - w,
									tf_y);
					
					self->width = w;
					gtk_widget_queue_resize(GTK_WIDGET(self));
				}
				else if (self->drag_info.drag_type == 2)
				{
					gint w = event_x - tf_x;
					if (w < TF_MIN_WIDTH(DOODLES_TEXT_FIELD_GET_CLASS(self)))
						w = TF_MIN_WIDTH(DOODLES_TEXT_FIELD_GET_CLASS(self));
					
					self->width = w;
					gtk_widget_queue_resize(GTK_WIDGET(self));
					
					break;
				}
			}
			
			break;
	}
	
	return TRUE;
}


// Other functions

static void
get_content_transform(	DoodlesTextField*	self,
						gint*				px,
						gint*				py,
						gint*				pwidth,
						gint*				pheight)
{
	DoodlesTextFieldClass* klass = DOODLES_TEXT_FIELD_GET_CLASS(self);
	
	if (px != NULL)
		*px = klass->space;
	if (py != NULL)
		*py = klass->title_height + klass->space;
	if (pwidth != NULL)
		*pwidth = self->width - klass->space * 2;
	if (pheight != NULL)
		*pheight = self->height - klass->title_height - klass->space * 2;
}

static GtkAllocation
get_content_transform_rectangle(DoodlesTextField* self)
{
	gint x, y, w, h;
	get_content_transform(	self,
							&x,
							&y,
							&w,
							&h);
	
	GtkAllocation alloc = { x, y, w, h };
	
	return alloc;
}