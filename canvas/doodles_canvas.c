#include "doodles_canvas.h"


// Structs
struct _DoodlesCanvasClass
{
	GtkWidgetClass		parent;
	
	gdouble				zoom;
};

struct _DoodlesCanvas
{
	GtkWidget			parent;
	
	gdouble				width;
	gdouble				height;
	DoodlesCanvas*		child_canvas;
	
	// Methods (NULL if should be ignored) (return TRUE if skip default function)
	gboolean			(*draw)(	GtkWidget*		self,
									GtkSnapshot*	snap,
									gpointer		user_data);
	gpointer			draw_user_data;
	
	// Data
	STR_LIST*			data_lines;
};

static DoodlesCanvasClass* class_pointer;


// Prototypes
static void dispose(GObject* object);
static GtkSizeRequestMode get_request_mode(GtkWidget* self);
static void measure (	GtkWidget*		self,
						GtkOrientation	orientation,
						gint			for_size,
						gint*			minimum,
						gint*			natural,
						gint*			minimum_baseline,
						gint*			natural_baseline);
static void snapshot(	GtkWidget*		self_widget,
						GtkSnapshot*	snap);



// Main Functions
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
	
	self->data_lines = NULL;
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


// Constructor
DoodlesCanvas*
doodles_canvas_new(	gdouble pWidth,
					gdouble pHeight)
{
	GObject* new = g_object_new(DOODLES_TYPE_CANVAS, NULL);
	DoodlesCanvas* self = DOODLES_CANVAS(new);
	
	self->width = pWidth;
	self->height = pHeight;
	
	return DOODLES_CANVAS(new);
}


// Virtual Methods

static void
dispose(GObject* object)
{
	if (DOODLES_CANVAS(object)->child_canvas != NULL)
	{
		gtk_widget_unparent(GTK_WIDGET(DOODLES_CANVAS(object)->child_canvas));
	}
}

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
	
	gdouble w = canvas_self->width * PIXEL_PER_CM * canvas_klass->zoom;
	gdouble h = canvas_self->height * PIXEL_PER_CM * canvas_klass->zoom;
	
	
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
		gtk_widget_measure(	child_widget,
							orientation,
							for_size,
							minimum,
							natural,
							minimum_baseline,
							natural_baseline);
		// ^^^ doesnt do much. Child will only execute this same method and allocate
	}
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
	
	// Get size
	gdouble w = gtk_widget_get_width(self_widget);
	gdouble h = gtk_widget_get_height(self_widget);
	
	// Draw background
	GdkRGBA red;
	gdk_rgba_parse(&red, "#CCCCCC66");
	
	gtk_snapshot_append_color(	snap,
								&red,
								&GRAPHENE_RECT_INIT(0, 0, w, h));
	
	
	
	// DRAW CONTENT //
	w = doodles_canvas_get_width(self);
	h = doodles_canvas_get_height(self);
	gdouble m = doodles_canvas_get_pixel_per_cm();
	cairo_t* cairo = gtk_snapshot_append_cairo(	snap,
												&GRAPHENE_RECT_INIT(0, 0, w*m, h*m));
	
	if (self->data_lines != NULL)
	{
		STR_LIST* list = self->data_lines;
		do
		{
			/*if (list->data == NULL)
			{
				printf("Drawing line with data == NULL ?!\n");
				break; // This should NOT happen!
			}*/
			
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
											point_list[i]->x, point_list[i]->y,
											line->r, line->g, line->b, line->a);
				
				pnt_x = point_list[i]->x;
				pnt_y = point_list[i]->y;
			}
			
			
			
			list = list->next;
		} while (list != NULL);
	}
	
	cairo_destroy(cairo);
}


// Other functions

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
	return PIXEL_PER_CM * doodles_canvas_get_class()->zoom;
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


// DATA

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
	
	// Get bounding box
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
	
	// Set values
	line->x = min_x;
	line->y = min_y;
	line->w = max_x - min_x;
	line->h = max_y - min_y;
	
	line->size = size;
	
	line->r = r;
	line->g = g;
	line->b = b;
	line->a = a;
	
	line->points = point_list;
	line->points_length = point_list_size;
	
	
	// Attach line to list
	list->data = line;
}


// DRAWING

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
							gdouble x2, gdouble y2,
							gdouble r, gdouble g, gdouble b, gdouble a)
{
	x1 *= doodles_canvas_get_pixel_per_cm();
	y1 *= doodles_canvas_get_pixel_per_cm();
	x2 *= doodles_canvas_get_pixel_per_cm();
	y2 *= doodles_canvas_get_pixel_per_cm();
	
	cairo_move_to(cairo, x1, y1);
	cairo_line_to(cairo, x2, y2);
	
	cairo_set_source_rgba(cairo, r, g, b, a);
	cairo_stroke(cairo);
}