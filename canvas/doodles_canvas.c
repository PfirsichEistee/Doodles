#include "doodles_canvas.h"


// STRUCTS //

struct _DoodlesCanvasClass
{
	GtkWidgetClass			parent;
	
	gdouble					zoom;
};

struct _DoodlesCanvas
{
	GtkWidget				parent;
	
	DoodlesPage*			page;
	
	gdouble					width;
	gdouble					height;
	gint					background;
	DoodlesCanvas*			child_canvas;
	
	// Methods (NULL if should be ignored) (return TRUE if skip default function)
	gboolean				(*draw)(	GtkWidget*		self,
										GtkSnapshot*	snap,
										gpointer		user_data);
	gpointer				draw_user_data;
	
	// Data
	STR_LIST*				data_lines;
	STR_WIDGET_CONTAINER*	data_widgets;
};



// PROTOTYPES //

static void
dispose(GObject* object);

static GtkSizeRequestMode
get_request_mode(GtkWidget* self);

static void
measure (	GtkWidget*		self,
			GtkOrientation	orientation,
			gint			for_size,
			gint*			minimum,
			gint*			natural,
			gint*			minimum_baseline,
			gint*			natural_baseline);

static void
snapshot(	GtkWidget*		self_widget,
			GtkSnapshot*	snap);

static void
doodles_canvas_draw_background	(	cairo_t*	cairo,
									gdouble w, gdouble h,
									int			bg_type);



// VARIABLES //

static DoodlesCanvasClass* class_pointer;
static gdouble ZOOM = 1.0;



// FUNCTIONS //

// Public functions

void
doodles_canvas_set_child(	DoodlesCanvas*	self,
							DoodlesCanvas*	child)
{
	self->child_canvas = child;
	
	gtk_widget_set_parent(GTK_WIDGET(child), GTK_WIDGET(self));
	
	gtk_widget_queue_allocate(GTK_WIDGET(self));
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

DoodlesCanvasClass*
doodles_canvas_get_class()
{
	return class_pointer;
}

void
doodles_canvas_set_draw(	DoodlesCanvas* self,
							gboolean	(*draw)(	GtkWidget*		self,
													GtkSnapshot*	snap,
													gpointer		user_data),
							gpointer	user_data)
{
	self->draw = draw;
	self->draw_user_data = user_data;
}

gdouble
doodles_canvas_get_pixel_per_cm()
{
	return PIXEL_PER_CM * ZOOM;
}

void
doodles_canvas_set_zoom(gdouble pZoom)
{
	ZOOM = pZoom;
}

DoodlesPage*
doodles_canvas_get_page(DoodlesCanvas* self)
{
	return self->page;
}

gdouble
doodles_canvas_get_width(DoodlesCanvas* self)
{
	return self->width;
}
gdouble
doodles_canvas_get_height(DoodlesCanvas* self)
{
	return self->height;
}

void
doodles_canvas_set_background(	DoodlesCanvas*	self,
								int				bg_type)
{
	self->background = bg_type;
}

STR_LIST*
doodles_canvas_get_data_lines(DoodlesCanvas* self)
{
	return self->data_lines;
}

void
doodles_canvas_line_calc_bounds(STR_LINE* line)
{
	STR_POINT** point_list = line->points;
	gint point_list_size = line->points_length;
	
	STR_POINT* pnt = point_list[0];
	gdouble min_x = pnt->x;
	gdouble min_y = pnt->y;
	gdouble max_x = pnt->x;
	gdouble max_y = pnt->y;
	
	for (gint i = 1; i < point_list_size; i++)
	{
		pnt = point_list[i];
		
		if (pnt->x < min_x)
			min_x = pnt->x;
		else if (pnt->x > max_x)
			max_x = pnt->x;
		
		if (pnt->y < min_y)
			min_y = pnt->y;
		else if (pnt->y > max_y)
			max_y = pnt->y;
	}
	
	// Update values
	line->x = min_x;
	line->y = min_y;
	line->w = max_x - min_x;
	line->h = max_y - min_y;
}


// Data functions

void
doodles_canvas_add_line(	DoodlesCanvas*	self,
							STR_POINT**		point_list, // == STR_POINT*-Array -> STR_POINT*[]
							guint			point_list_size,
							gdouble			size,
							gdouble r, gdouble g, gdouble b, gdouble a)
{
	// Create new list entry
	
	gboolean is_new_list = FALSE;
	STR_LIST* list = self->data_lines;
	
	if (list == NULL)
	{
		// List is empty
		is_new_list = TRUE;
		
		list = malloc(sizeof(STR_LIST));
		if (list == NULL)
			return; // Failed!
		
		list->next = NULL;
		list->data = NULL;
		
		self->data_lines = list;
	}
	else
	{
		// Attach new entry
		STR_LIST* new = malloc(sizeof(STR_LIST));
		if (new == NULL)
			return; // Failed!
		new->data = NULL;
		new->next = NULL;
		
		while (list->next != NULL)
			list = list->next;
		list->next = new;
		list = new;
	}
	
	// .."list" is now the current/new entry
	// check is_new_list in case you have to abort this whole thing
	
	
	// Create new line
	STR_LINE* line = malloc(sizeof(STR_LINE));
	if (line == NULL)
	{
		// Creating new line failed, abort
		if (is_new_list)
		{
			self->data_lines = NULL;
			free(list);
		}
		else
		{
			STR_LIST* ph = self->data_lines;
			while (ph->next != list)
				ph = ph->next;
			
			ph->next = NULL;
			free(list);
		}
		return;
	}
	
	// Set values
	line->size = size;
	
	line->r = r;
	line->g = g;
	line->b = b;
	line->a = a;
	
	line->points = point_list;
	line->points_length = point_list_size;
	
	doodles_canvas_line_calc_bounds(line);
	
	
	// Attach line to list
	list->data = line;
}

void
doodles_canvas_remove_line(	DoodlesCanvas*	self,
							STR_LINE*		line,
							gboolean		free_points)
{
	STR_LIST* list = self->data_lines;
	
	// Free points
	if (free_points)
	{
		for (gint i = 0; i < line->points_length; i++)
		{
			free(line->points[i]);
		}
		free(line->points);
	}
	
	
	// Free list & lines & update self->data_lines
	if (list->data == line)
	{
		// First entry is line
		
		if (list->next == NULL)
		{
			// Line is the only element
			
			self->data_lines = NULL;
			free(line);
			free(list);
		}
		else
		{
			// There are more elements in the list
			
			self->data_lines = list->next;
			free(line);
			free(list);
		}
	}
	else
	{
		// Look for line
		while ((list->next)->data != line)
			list = list->next;
		
		// Update connection
		STR_LIST* ph = list->next;
		list->next = ph->next;
		
		free(line);
		free(ph);
	}
}

void
doodles_canvas_add_widget(	DoodlesCanvas*	self,
							GtkWidget*		widget,
							gdouble x, gdouble y,
							gdouble w, gdouble h)
{
	// Create container
	STR_WIDGET_CONTAINER* cont = malloc(sizeof(STR_WIDGET_CONTAINER));
	if (cont == NULL)
		return;
	
	cont->widget = widget;
	cont->x = x;
	cont->y = y;
	cont->w = w;
	cont->h = h;
	cont->next = NULL;
	
	// Add to list
	if (self->data_widgets == NULL)
	{
		// ..first entry
		self->data_widgets = cont;
	}
	else
	{
		// ..append new entry
		STR_WIDGET_CONTAINER* ph = self->data_widgets;
		while (ph->next != NULL)
			ph = ph->next;
		
		ph->next = cont;
	}
	
	gtk_widget_set_parent(widget, GTK_WIDGET(self));
	
	gtk_widget_queue_allocate(GTK_WIDGET(self));
	gtk_widget_queue_draw(GTK_WIDGET(self));
}

void
doodles_canvas_widget_realloc(	DoodlesCanvas*			self,
								STR_WIDGET_CONTAINER*	cont)
{
	gdouble ppc = doodles_canvas_get_pixel_per_cm();
	
	const GtkAllocation alloc = { cont->x * ppc, cont->y * ppc, cont->w * ppc, cont->h * ppc };
	gtk_widget_size_allocate(	cont->widget,
								&alloc,
								-1);
}

STR_WIDGET_CONTAINER*
doodles_canvas_get_widget_list(DoodlesCanvas* self)
{
	return self->data_widgets;
}


// Drawing functions
void
doodles_canvas_draw_circle(	cairo_t* cairo,
							gdouble x, gdouble y,
							gdouble radius,
							gboolean fill,
							gdouble r, gdouble g, gdouble b, gdouble a)
{
	cairo_arc(	cairo,
				x * doodles_canvas_get_pixel_per_cm(),
				y * doodles_canvas_get_pixel_per_cm(),
				radius * doodles_canvas_get_pixel_per_cm(),
				0,
				M_PI * 2);
	
	cairo_set_source_rgba(cairo, r, g, b, a);
	
	if (fill)
		cairo_fill(cairo);
	else
		cairo_stroke(cairo);
}

void
doodles_canvas_draw_line(	cairo_t* cairo,
							gdouble x1, gdouble y1,
							gdouble x2, gdouble y2)
{
	x1 *= doodles_canvas_get_pixel_per_cm();
	y1 *= doodles_canvas_get_pixel_per_cm();
	x2 *= doodles_canvas_get_pixel_per_cm();
	y2 *= doodles_canvas_get_pixel_per_cm();
	
	cairo_move_to(cairo, x1, y1);
	cairo_line_to(cairo, x2, y2);
}

static void
doodles_canvas_draw_background	(	cairo_t*	cairo,
									gdouble w, gdouble h,
									int			bg_type)
{
	if (bg_type == BG_NONE)
		return;
	
	gdouble ppc = doodles_canvas_get_pixel_per_cm();
	
	cairo_rectangle(cairo, 0, 0, w * ppc, h * ppc);
	cairo_set_source_rgb(cairo, 1, 1, 1);
	cairo_fill(cairo);
	
	gdouble plus_x = ((w - floor(w)) * ppc) / 2;
	gdouble plus_y = ((h - floor(h)) * ppc) / 2;
	
	switch (bg_type)
	{
		case (BG_CHECKERED):
			for (int x = 0; x < ceil(w); x++)
			{
				cairo_move_to(cairo, ppc * x + plus_x, 0);
				cairo_line_to(cairo, ppc * x + plus_x, h * ppc);
			}
			for (int y = 0; y < ceil(h); y++)
			{
				cairo_move_to(cairo, 0, ppc * y + plus_y);
				cairo_line_to(cairo, w * ppc, ppc * y + plus_y);
			}
			break;
	}
	
	cairo_set_source_rgb(cairo, 0.7, 0.7, 0.7);
	cairo_set_line_width(cairo, 0.05 * ppc);
	cairo_set_line_cap(cairo, CAIRO_LINE_CAP_BUTT);
	cairo_stroke(cairo);
}



// Constructor
DoodlesCanvas*
doodles_canvas_new(	gdouble			pWidth,
					gdouble			pHeight,
					DoodlesPage*	pPage)
{
	GObject* new = g_object_new(DOODLES_TYPE_CANVAS, NULL);
	DoodlesCanvas* self = DOODLES_CANVAS(new);
	
	self->page = pPage;
	self->width = pWidth;
	self->height = pHeight;
	
	return DOODLES_CANVAS(new);
}



// WIDGET FUNCTIONS //

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
	DoodlesCanvas* canvas_self = DOODLES_CANVAS(self);
	DoodlesCanvasClass* canvas_klass = DOODLES_CANVAS_GET_CLASS(canvas_self);
	GtkWidgetClass* widget_klass = GTK_WIDGET_CLASS(canvas_klass);
	
	gdouble w = canvas_self->width * doodles_canvas_get_pixel_per_cm();
	gdouble h = canvas_self->height * doodles_canvas_get_pixel_per_cm();
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		*minimum = w;
		*natural = w;
	}
	else
	{
		*minimum = h;
		*natural = h;
	}
	
	const GtkAllocation alloc = { 0, 0, w, h };
	gtk_widget_size_allocate(	self,
								&alloc,
								-1);
	
	// Measure child if exists
	if (DOODLES_CANVAS(self)->child_canvas != NULL)
	{
		GtkWidget* child_widget = GTK_WIDGET(DOODLES_CANVAS(self)->child_canvas);
		widget_klass->measure(	child_widget,
								orientation,
								for_size,
								minimum,
								natural,
								minimum_baseline,
								natural_baseline);
		// ^^^ doesnt do much. Child will only execute this same method and allocate
	}
	
	// Measure child-widgets if exist
	if (canvas_self->data_widgets != NULL)
	{
		STR_WIDGET_CONTAINER* cont = canvas_self->data_widgets;
		//gdouble ppc = doodles_canvas_get_pixel_per_cm();
		
		while (cont != NULL)
		{
			doodles_canvas_widget_realloc(canvas_self, cont);
			
			cont = cont->next;
		}
	}
	
	gtk_widget_queue_draw(self);
}

static void
snapshot(	GtkWidget*		self_widget,
			GtkSnapshot*	snap)
{
	DoodlesCanvas* self = DOODLES_CANVAS(self_widget);
	
	// Draw child if exists
	if (self->child_canvas != NULL)
		gtk_widget_snapshot_child(	self_widget,
									GTK_WIDGET(self->child_canvas),
									snap);
	
	// (Optional) call snapshot handler
	if (self->draw != NULL)
	{
		gboolean skip = self->draw(	self_widget,
									snap,
									self->draw_user_data);
		
		if (skip)
			return;
	}
	
	
	// DRAW //
	gdouble w = doodles_canvas_get_width(self);
	gdouble h = doodles_canvas_get_height(self);
	gdouble m = doodles_canvas_get_pixel_per_cm();
	GdkRGBA red;
	gdk_rgba_parse(&red, "rgba(0.5, 0.5, 0, 0.1)");
	//gtk_snapshot_append_color (snap, &red, &GRAPHENE_RECT_INIT(0, 0, w * m, h * m));
	gdk_rgba_parse(&red, "green");
	//gtk_snapshot_append_color (snap, &red, &GRAPHENE_RECT_INIT(0, 0, w * m, h * m)); // DEBUG
	
	
	
	cairo_t* cairo = gtk_snapshot_append_cairo(	snap,
												&GRAPHENE_RECT_INIT(0, 0, w*m, h*m));
	
	// Background
	doodles_canvas_draw_background	(	cairo,
										self->width, self->height,
										self->background);
	
	if (self->child_canvas == NULL)
	{
		GdkRGBA shadowclr;
		gdk_rgba_parse(&shadowclr, "black");
		gtk_snapshot_append_outset_shadow(	snap,
											&GSK_ROUNDED_RECT_INIT(0, 0, w * m, h * m),
											&shadowclr,
											0, 0,
											1.0f,
											5.0f);
	}
	
	
	// Lines
	if (self->data_lines != NULL)
	{
		STR_LIST* list = self->data_lines;
		do
		{
			STR_LINE* line = (STR_LINE*)list->data;
			STR_POINT** point_list = line->points;
			
			cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
			cairo_set_line_width(cairo, line->size * doodles_canvas_get_pixel_per_cm());
			
			gdouble pnt_x = point_list[0]->x;
			gdouble pnt_y = point_list[0]->y;
			
			for (int i = 0; i < line->points_length; i++)
			{
				doodles_canvas_draw_line(	cairo,
											pnt_x, pnt_y,
											point_list[i]->x, point_list[i]->y);
				
				pnt_x = point_list[i]->x;
				pnt_y = point_list[i]->y;
			}
			
			cairo_set_source_rgba(cairo, line->r, line->g, line->b, line->a);
			cairo_stroke(cairo);
			
			
			list = list->next;
		} while (list != NULL);
	}
	
	// Widgets
	if (self->data_widgets != NULL)
	{
		STR_WIDGET_CONTAINER* cont = self->data_widgets;
		
		while (cont != NULL)
		{
			gtk_widget_snapshot_child(	GTK_WIDGET(self),
										cont->widget,
										snap);
			
			cont = cont->next;
		}
	}
	// DEBUG widgets
	/*if (self->data_widgets != NULL)
	{
		STR_WIDGET_CONTAINER* cont = self->data_widgets;
		gdouble ppc = doodles_canvas_get_pixel_per_cm();
		GdkRGBA dbgclr;
		gdk_rgba_parse(&dbgclr, "rgba(0.5, 0.5, 0.5, 0.1)");
		
		while (cont != NULL)
		{
			gtk_snapshot_append_color(snap, &dbgclr, &GRAPHENE_RECT_INIT(cont->x * ppc, cont->y * ppc, cont->w * ppc, cont->h * ppc));
			
			cont = cont->next;
		}
	}*/
	
	cairo_destroy(cairo);
}



// MAIN FUNCTIONS //

static void
dispose(GObject* object)
{
	if (DOODLES_CANVAS(object)->child_canvas != NULL)
	{
		gtk_widget_unparent(GTK_WIDGET(DOODLES_CANVAS(object)->child_canvas));
	}
}

static void
doodles_canvas_class_init(	gpointer	klass,
							gpointer	klass_data)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
	DoodlesCanvasClass* canvas_class = DOODLES_CANVAS_CLASS(klass);
	class_pointer = canvas_class;
	
	object_class->dispose = dispose;
	widget_class->snapshot = snapshot;
	widget_class->get_request_mode = get_request_mode;
	widget_class->measure = measure;
	
	canvas_class->zoom = 1.0;
}

static void
doodles_canvas_instance_init(	GTypeInstance*	instance,
								gpointer		klass)
{
	DoodlesCanvas* self = DOODLES_CANVAS(instance);
	
	self->page = NULL;
	self->background = BG_NONE;
	self->child_canvas = NULL,
	self->draw = NULL;
	self->draw_user_data = NULL;
	self->data_lines = NULL;
	self->data_widgets = NULL;
}

GType
doodles_canvas_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(struct _DoodlesCanvasClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_canvas_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(struct _DoodlesCanvas),
			.instance_init	= doodles_canvas_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	GTK_TYPE_WIDGET,
										"DoodlesCanvas",
										&info,
										0);
	}
	
	return type;
}