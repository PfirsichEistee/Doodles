#include "doodles_container.h"


// STRUCTS //

struct _DoodlesContainerClass
{
	GtkWidgetClass		parent;
	
	gint				space;
};

struct _DoodlesContainer
{
	GtkWidget			parent;
	
	GtkScrolledWindow*	scroll_window;
	gdouble				scale;
};


// PROTOTYPES //
// ...


// FUNCTIONS //

// Public
void
doodles_container_insert(	DoodlesContainer*	self,
							GtkWidget*			child)
{
	gtk_widget_set_parent(child, GTK_WIDGET(self));
	
	gtk_widget_queue_allocate(GTK_WIDGET(self));
}

void
doodles_container_set_scrolled_window(	DoodlesContainer*	self,
										GtkScrolledWindow*	p_scroll_window)
{
	self->scroll_window = p_scroll_window;
}


// Constructor
GtkWidget*
doodles_container_new()
{
	GObject* new = g_object_new(DOODLES_TYPE_CONTAINER, NULL);
	
	return GTK_WIDGET(new);
}

// Widget functions
static GtkSizeRequestMode
get_request_mode(GtkWidget* self)
{
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
measure (	GtkWidget*		self_widget,
			GtkOrientation	orientation,
			gint			for_size,
			gint*			minimum,
			gint*			natural,
			gint*			minimum_baseline,
			gint*			natural_baseline)
{
	DoodlesContainer* self = DOODLES_CONTAINER(self_widget);
	DoodlesContainerClass* self_class = DOODLES_CONTAINER_GET_CLASS(self);
	
	gint content_width = self_class->space * 4;
	gint content_height = self_class->space;
	
	
	// Measure content width
	GtkWidget* child = gtk_widget_get_first_child(self_widget);
	while (child != NULL)
	{
		gint w_width, dummy;
		
		gtk_widget_measure(	child,
							GTK_ORIENTATION_HORIZONTAL,
							-1,
							&w_width,
							&dummy,
							&dummy,
							&dummy);
		
		if (content_width < (w_width + self_class->space * 4))
			content_width = w_width + self_class->space * 4;
		
		child = gtk_widget_get_next_sibling(child);
	}
	
	// Measure content height
	child = gtk_widget_get_first_child(self_widget);
	while (child != NULL)
	{
		gint w_width, w_height, dummy;
		
		gtk_widget_measure(	child,
							GTK_ORIENTATION_HORIZONTAL,
							-1,
							&w_width,
							&dummy,
							&dummy,
							&dummy);
		
		gtk_widget_measure(	child,
							GTK_ORIENTATION_VERTICAL,
							-1,
							&w_height,
							&dummy,
							&dummy,
							&dummy);
		
		const GtkAllocation alloc = { (content_width - w_width) / 2, content_height, w_width, w_height };
		gtk_widget_size_allocate(	child,
									&alloc,
									-1);
		
		content_height += w_height + self_class->space;
		
		child = gtk_widget_get_next_sibling(child);
	}
	
	// Multiply by scale
	content_width = (gint)(content_width * self->scale);
	content_height = (gint)(content_height * self->scale);
	
	// Set size
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		*minimum = content_width;
		*natural = content_width;
	}
	else
	{
		*minimum = content_height;
		*natural = content_height;
	}
}

static void
snapshot(	GtkWidget*		self_widget,
			GtkSnapshot*	snap)
{
	DoodlesContainer* self = DOODLES_CONTAINER(self_widget);
	
	// Get min size
	GtkRequisition* req = gtk_requisition_new();
	gtk_widget_get_preferred_size(self_widget, req, NULL);
	gint w = req->width;
	gint h = req->height;
	gtk_requisition_free(req);
	
	// Draw
	GdkRGBA red;
	gdk_rgba_parse(&red, "red");
	gtk_snapshot_append_color (snap, &red, &GRAPHENE_RECT_INIT(0, 0, w, h)); // DEBUG
	gdk_rgba_parse(&red, "blue");
	
	// Draw children
	GtkWidget* child = gtk_widget_get_first_child(self_widget);
	GtkSnapshot* child_snap = gtk_snapshot_new();
	gtk_snapshot_scale(child_snap, self->scale, self->scale);
	while (child != NULL)
	{
		gtk_widget_snapshot_child(	self_widget,
									child,
									child_snap);
		
		child = gtk_widget_get_next_sibling(child);
	}
	
	
	gtk_snapshot_append_node(	snap,
								gtk_snapshot_free_to_node(child_snap));
}


// Events
static gboolean
on_legacy_event(	GtkEventControllerLegacy*	controller,
					GdkEvent*					event,
					gpointer					user_data)
{
	DoodlesContainer* self = (DoodlesContainer*)user_data;
	DoodlesContainerClass* self_class = DOODLES_CONTAINER_GET_CLASS(self);
	
	static gdouble mouse_pos_x = 0, mouse_pos_y = 0;
	
	gint event_type = gdk_event_get_event_type(event);
	switch (event_type)
	{
		case (GDK_MOTION_NOTIFY):
			// Update cursor values
			//gdouble prev_mouse_x = mouse_pos_x;
			//gdouble prev_mouse_y = mouse_pos_y;
			
			gdk_event_get_position(event, &mouse_pos_x, &mouse_pos_y);
			gtk_widget_translate_coordinates(	GTK_WIDGET(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)))),
												GTK_WIDGET(self),
												mouse_pos_x, mouse_pos_y,
												&mouse_pos_x, &mouse_pos_y);
			
			gtk_widget_queue_draw(GTK_WIDGET(self));
			
			break;
		case (GDK_BUTTON_PRESS):
			// Update cursor values
			gdk_event_get_position(event, &mouse_pos_x, &mouse_pos_y);
			gtk_widget_translate_coordinates(	GTK_WIDGET(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)))),
												GTK_WIDGET(self),
												mouse_pos_x, mouse_pos_y,
												&mouse_pos_x, &mouse_pos_y);
			
			break;
		case (GDK_BUTTON_RELEASE):
			// Update cursor values
			gdk_event_get_position(event, &mouse_pos_x, &mouse_pos_y);
			gtk_widget_translate_coordinates(	GTK_WIDGET(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(self)))),
												GTK_WIDGET(self),
												mouse_pos_x, mouse_pos_y,
												&mouse_pos_x, &mouse_pos_y);
			
			break;
		case (GDK_SCROLL):
			if (gdk_event_get_modifier_state(event) & GDK_CONTROL_MASK)
			{
				// Get new scale
				gdouble old_scale = self->scale;
				self->scale += 0.3 * (gdk_scroll_event_get_direction(event) == GDK_SCROLL_UP ? 1 : -1) * CLAMP((1.0 / 3.0) * self->scale, 0.1, 1);
				self->scale = CLAMP(self->scale, 0.2, 3);
				
				// Get previous & new mouse positions (in "unscaled" pixels)
				gdouble old_x = mouse_pos_x / old_scale;
				gdouble old_y = mouse_pos_y / old_scale;
				
				gdouble new_x = mouse_pos_x / self->scale;
				gdouble new_y = mouse_pos_y / self->scale;
				
				
				gdouble delta_x = (new_x - old_x);
				gdouble delta_y = (new_y - old_y);
				
				
				// Move scrolled window
				gdouble ph = (gtk_widget_get_width(GTK_WIDGET(self)) / old_scale) * self->scale;
				gtk_adjustment_set_upper(gtk_scrolled_window_get_hadjustment(self->scroll_window), ph);
				ph = (gtk_widget_get_height(GTK_WIDGET(self)) / old_scale) * self->scale;
				gtk_adjustment_set_upper(gtk_scrolled_window_get_vadjustment(self->scroll_window), ph);
				gdouble srl_h = gtk_adjustment_get_value(gtk_scrolled_window_get_hadjustment(self->scroll_window));
				gdouble srl_v = gtk_adjustment_get_value(gtk_scrolled_window_get_vadjustment(self->scroll_window));
				
				gtk_adjustment_set_value(gtk_scrolled_window_get_hadjustment(self->scroll_window), srl_h - (delta_x * self->scale));
				gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(self->scroll_window), srl_v - (delta_y * self->scale));
				
				
				// Resize widget
				gtk_widget_queue_resize(GTK_WIDGET(self));
				
				return TRUE;
			}
			
			return FALSE;
	}
	
	
	// Should event type be passed down?
	switch (event_type)
	{
		case (GDK_MOTION_NOTIFY):
		case (GDK_BUTTON_PRESS):
		case (GDK_BUTTON_RELEASE):
			break;
		default:
			goto skip_to_end;
	}
	
	
	// Send event to child (Only DoodlesPage's !)
	GtkWidget* child = NULL;
	gint ph_space = 0;
	for (gint i = 0; i < 2; i++)
	{
		child = gtk_widget_get_first_child(GTK_WIDGET(self));
		
		while (child != NULL)
		{
			GtkAllocation alloc;
			gtk_widget_get_allocation(	child,
										&alloc);
			alloc.x *= self->scale;
			alloc.y *= self->scale;
			alloc.width *= self->scale;
			alloc.height *= self->scale;
			
			
			// Mouse inside widget allocation?
			if (mouse_pos_x >= (alloc.x - ph_space) && mouse_pos_x < (alloc.x + alloc.width + ph_space) &&
					mouse_pos_y >= (alloc.y - ph_space) && mouse_pos_y < (alloc.y + alloc.height + ph_space))
			{
				// Get local mouse pos
				gdouble ph_mouse_x = mouse_pos_x - alloc.x;
				gdouble ph_mouse_y = mouse_pos_y - alloc.y;
				ph_mouse_x /= self->scale;
				ph_mouse_y /= self->scale;
				
				
				// Pass event if canvas
				if (DOODLES_IS_CANVAS(child))
				{
					DoodlesPage* page = doodles_canvas_get_page(DOODLES_CANVAS(child));
					
					if (page != NULL)
					{
						gint ph_btn = (event_type != GDK_MOTION_NOTIFY ? gdk_button_event_get_button(event) : -1);
						doodles_page_receive_event(	page,
													ph_mouse_x,
													ph_mouse_y,
													ph_btn,
													event_type);
					}
				}
				
				goto skip_to_end;
			}
			
			child = gtk_widget_get_next_sibling(child);
		}
		
		// If not found: Try it again with space
		ph_space = self_class->space * self->scale;
	}
	
	skip_to_end:
	return TRUE;
}


// MAIN FUNCTIONS //
static void
dispose(GObject* object)
{
	// ...
}

static void
doodles_container_class_init(	gpointer	klass,
								gpointer	klass_data)
{
	DoodlesContainerClass* self_class = DOODLES_CONTAINER_CLASS(klass);
	
	G_OBJECT_CLASS(klass)->dispose = dispose;
	
	GTK_WIDGET_CLASS(klass)->measure = measure;
	GTK_WIDGET_CLASS(klass)->snapshot = snapshot;
	
	self_class->space = 15;
}

static void
doodles_container_instance_init(	GTypeInstance*	instance,
									gpointer		klass)
{
	DoodlesContainer* self = DOODLES_CONTAINER(instance);
	
	self->scale = 1.5;
	gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_CENTER);
	gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);
	
	GtkEventController* controller = gtk_event_controller_legacy_new();
	g_signal_connect(	controller,
						"event",
						G_CALLBACK(on_legacy_event),
						self);
	
	gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(GTK_WIDGET(self), controller);
}

GType
doodles_container_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(struct _DoodlesContainerClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_container_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(struct _DoodlesContainer),
			.instance_init	= doodles_container_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	GTK_TYPE_WIDGET,
										"DoodlesContainer",
										&info,
										0);
	}
	
	return type;
}