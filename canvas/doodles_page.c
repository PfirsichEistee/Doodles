#include "doodles_page.h"


// Structs
struct _DoodlesPageClass
{
	GObjectClass		parent;
};

struct _DoodlesPage
{
	GObject					parent;
	
	DoodlesGuiController*	gui_controller;
	
	// Layers
	DoodlesCanvas*			layer_work;
	DoodlesCanvas*			layer_highlighter;
	DoodlesCanvas*			layer_pen;
	DoodlesCanvas*			layer_graphics;
	DoodlesCanvas*			layer_background;
	
	// Cursor
	guchar					mouse_pressed[3];
	gdouble					cursor_x;
	gdouble					cursor_y;
	gpointer				tool_data;
};



// Prototypes
static void dispose(GObject* object);
static gboolean on_legacy_event(	GtkEventControllerLegacy*	event_controller,
									GdkEvent*					event,
									gpointer					user_data);
static gboolean on_draw(	GtkWidget*		widget,
							GtkSnapshot*	snap,
							gpointer		user_data);
static void pos_to_cm(	gdouble*		x,
						gdouble*		y);


// Main Functions
static void
doodles_page_class_init(	gpointer	klass,
							gpointer	klass_data)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	//DoodlesPageClass* self_class = DOODLES_PAGE_CLASS(object_class);
}


static void
doodles_page_instance_init(	GTypeInstance*	instance,
							gpointer		klass)
{
	DoodlesPage* self = DOODLES_PAGE(instance);
	
	self->cursor_x = 0;
	self->cursor_y = 0;
	
	for (guchar i = 0; i < 3; i++)
		self->mouse_pressed[i] = FALSE;
}


GType
doodles_page_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(struct _DoodlesPageClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_page_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(struct _DoodlesPage),
			.instance_init	= doodles_page_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	G_TYPE_OBJECT,
										"DoodlesPage",
										&info,
										0);
	}
	
	return type;
}


// Constructor
DoodlesPage*
doodles_page_new(	DoodlesGuiController*	controller,
					gdouble					pWidth,
					gdouble					pHeight)
{
	GObject* new = g_object_new(DOODLES_TYPE_PAGE, NULL);
	DoodlesPage* self = DOODLES_PAGE(new);
	
	self->gui_controller = controller;
	
	// LAYERS
	self->layer_work = doodles_canvas_new(pWidth, pHeight);
	self->layer_highlighter = doodles_canvas_new(pWidth, pHeight);
	self->layer_pen = doodles_canvas_new(pWidth, pHeight);
	self->layer_graphics = doodles_canvas_new(pWidth, pHeight);
	self->layer_background = doodles_canvas_new(pWidth, pHeight);
	
	doodles_canvas_set_child(	self->layer_work,
								self->layer_highlighter);
	
	doodles_canvas_set_child(	self->layer_highlighter,
								self->layer_pen);
	
	doodles_canvas_set_child(	self->layer_pen,
								self->layer_graphics);
	
	doodles_canvas_set_child(	self->layer_graphics,
								self->layer_background);
	
	
	// Event Handler
	GtkEventController* legacy_controller = gtk_event_controller_legacy_new();
	g_signal_connect(	legacy_controller,
						"event",
						G_CALLBACK(on_legacy_event),
						self);
	gtk_widget_add_controller(GTK_WIDGET(self->layer_work), legacy_controller);
	
	
	// Drawing Handler
	DoodlesCanvas* c = self->layer_work;
	doodles_canvas_set_draw(	self->layer_work,
								&on_draw,
								self);
	
	
	// Done
	
	return self;
}


// Virtual Methods

static void
dispose(GObject* object)
{
	// ...
}


// Events


/////////////////
// TOOL EVENTS //
/////////////////

static gboolean
pen_event(	DoodlesPage*	self,
			GdkEvent*		event,
			gdouble crs_x, gdouble crs_y)
{
	// pen_event is for both pen & highlighter, as they have the same function anyways

	switch (gdk_event_get_event_type(event))
	{
		case (GDK_ENTER_NOTIFY):
			break;
		case (GDK_LEAVE_NOTIFY):
			break;
		case (GDK_MOTION_NOTIFY):
			if (!self->mouse_pressed[0] && (crs_x >= doodles_canvas_get_width(self->layer_work) || crs_y >= doodles_canvas_get_height(self->layer_work)))
				return FALSE;
			
			self->cursor_x = crs_x;
			self->cursor_y = crs_y;
			
			
			if (self->mouse_pressed[0] && self->tool_data != NULL)
			{
				STR_POINT* point = malloc(sizeof(STR_POINT));
				if (point != NULL)
				{
					point->x = crs_x;
					point->y = crs_y;
					
					GSList* list = g_slist_append(self->tool_data, point);
					
					self->tool_data = list;
				}
			}
			
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_PRESS):
			self->mouse_pressed[gdk_button_event_get_button(event) - 1] = TRUE;
			
			
			if (gdk_button_event_get_button(event) == 1 && self->tool_data == NULL)
			{
				STR_POINT* point = malloc(sizeof(STR_POINT));
				if (point != NULL)
				{
					point->x = crs_x;
					point->y = crs_y;
					
					GSList* list = g_slist_append(NULL, point);
					
					self->tool_data = list;
				}
			}
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_RELEASE):
			self->mouse_pressed[gdk_button_event_get_button(event) - 1] = FALSE;
			
			if (gdk_button_event_get_button(event) == 1 && self->tool_data != NULL)
			{
				// Create line array
				STR_POINT** point_list = malloc(sizeof(STR_POINT*) * g_slist_length(self->tool_data));
				guint cnt = 0;
				GSList* i = self->tool_data;
				while (i != NULL)
				{
					point_list[cnt] = malloc(sizeof(STR_POINT));
					if (point_list[cnt] == NULL)
					{
						// Failed, abort
						cnt = 0;
						for (GSList* k = self->tool_data; k = k->next; k != i)
						{
							free(point_list[cnt]);
							
							cnt++;
						}
						
						return FALSE;
					}
					
					point_list[cnt]->x = ((STR_POINT*)i->data)->x;
					point_list[cnt]->y = ((STR_POINT*)i->data)->y;
					
					cnt++;
					
					i = i->next;
				}
				
				
				// Save new line
				GdkRGBA color;
				doodles_gui_controller_get_color(self->gui_controller, &color);
				
				switch (doodles_gui_controller_get_tool(self->gui_controller))
				{
					case (TOOL_PEN):
						// Save as pen data
						doodles_canvas_add_line(	self->layer_pen,
													point_list,
													cnt,
													doodles_gui_controller_get_size(self->gui_controller),
													color.red, color.green, color.blue, 1.0);
						
						gtk_widget_queue_draw(GTK_WIDGET(self->layer_pen));
						break;
					case (TOOL_HIGHLIGHTER):
						// Save as highlighter data
						doodles_canvas_add_line(	self->layer_highlighter,
													point_list,
													cnt,
													doodles_gui_controller_get_size(self->gui_controller),
													color.red, color.green, color.blue, 0.25);
						
						gtk_widget_queue_draw(GTK_WIDGET(self->layer_pen));
						break;
				}
				
				
				// Free data
				GSList* l = self->tool_data;
				while (l != NULL)
				{
					free(l->data);
					
					l = l->next;
				}
				g_slist_free(self->tool_data);
				self->tool_data = NULL;
			}
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
	}
	
	return FALSE;
}


static gboolean
eraser_event(	DoodlesPage*	self,
				GdkEvent*		event,
				gdouble crs_x, gdouble crs_y)
{
	switch (gdk_event_get_event_type(event))
	{
		case (GDK_ENTER_NOTIFY):
			break;
		case (GDK_LEAVE_NOTIFY):
			break;
		case (GDK_MOTION_NOTIFY):
			if (!self->mouse_pressed[0] && (crs_x >= doodles_canvas_get_width(self->layer_work) || crs_y >= doodles_canvas_get_height(self->layer_work)))
				return FALSE;
			
			self->cursor_x = crs_x;
			self->cursor_y = crs_y;
			
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_PRESS):
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_RELEASE):
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
	}
	
	return FALSE;
}


static gboolean
on_legacy_event(	GtkEventControllerLegacy*	event_controller,
					GdkEvent*					event,
					gpointer					user_data)
{
	DoodlesPage* self = DOODLES_PAGE(user_data);
	
	if (doodles_gui_controller_get_tool(self->gui_controller) == 0)
		return FALSE;
	
	
	gdouble evt_x, evt_y;
	gdouble crs_x, crs_y;
	gdk_event_get_position(event, &evt_x, &evt_y);
	
	GtkWidget* root = GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(self->layer_work)));
	
	gtk_widget_translate_coordinates(	root,
										GTK_WIDGET(self->layer_work),
										evt_x, evt_y,
										&crs_x, &crs_y);
	
	pos_to_cm(&crs_x, &crs_y);
	
	
	switch (doodles_gui_controller_get_tool(self->gui_controller))
	{
		case (TOOL_PEN):
		case (TOOL_HIGHLIGHTER):
			return pen_event(	self,
								event,
								crs_x, crs_y);
		case (TOOL_ERASER):
			return eraser_event(	self,
									event,
									crs_x, crs_y);
	}
	
	
	return FALSE;
}











static gboolean
on_draw(	GtkWidget*		widget,
			GtkSnapshot*	snap,
			gpointer		user_data)
{
	DoodlesPage* self = DOODLES_PAGE(user_data);
	gdouble w = doodles_canvas_get_width(self->layer_work);
	gdouble h = doodles_canvas_get_height(self->layer_work);
	gdouble m = doodles_canvas_get_pixel_per_cm();
	
	cairo_t* cairo = gtk_snapshot_append_cairo(	snap,
												&GRAPHENE_RECT_INIT(0, 0, w*m, h*m));
	
	
	
	// Draw content
	if (self->tool_data != NULL)
	{
		GdkRGBA color;
		doodles_gui_controller_get_color(self->gui_controller, &color);
		gdouble size = doodles_gui_controller_get_size(self->gui_controller);
		
		cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
		cairo_set_line_width(cairo, size * doodles_canvas_get_pixel_per_cm());
		
		GSList* list = self->tool_data;
		STR_POINT* point = list->data;
		gdouble lx = point->x;
		gdouble ly = point->y;
		
		while (list != NULL)
		{
			point = list->data;
			doodles_canvas_draw_line(	cairo,
										lx, ly,
										point->x, point->y);
			
			lx = point->x;
			ly = point->y;
			list = list->next;
		}
		
		cairo_set_source_rgba(cairo, color.red, color.green, color.blue, color.alpha);
		cairo_stroke(cairo);
	}
	
	// Draw tool
	cairo_set_line_width(cairo, 1);
	doodles_canvas_draw_circle(	cairo,
								self->cursor_x, self->cursor_y,
								doodles_gui_controller_get_size(self->gui_controller) / 2,
								FALSE,
								0.7, 0.7, 0.7, 1.0);
	
	
	cairo_destroy(cairo);
	
	return TRUE;
}


// Other methods
GtkWidget*
doodles_page_get_widget(DoodlesPage* self)
{
	return GTK_WIDGET(self->layer_work);
}


static void
pos_to_cm(	gdouble*		x,
			gdouble*		y)
{
	*x /= doodles_canvas_get_pixel_per_cm();
	*y /= doodles_canvas_get_pixel_per_cm();
}