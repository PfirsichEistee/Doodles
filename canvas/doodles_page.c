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
	gdouble					prev_cursor_x;
	gdouble					prev_cursor_y;
	gpointer				tool_data;
	gint					prev_used_tool;
};



// Prototypes
static void dispose(GObject* object);
static gboolean on_draw(	GtkWidget*		widget,
							GtkSnapshot*	snap,
							gpointer		user_data);
static void pos_to_cm(	gdouble*		x,
						gdouble*		y);


// STRUCTS //
struct _tool_text_data
{
	STR_WIDGET_CONTAINER*	container;
	guint					state;
	gdouble					local_x; // local position when started dragging
	gdouble					local_y;
};


// ENUMS //
enum _tool_text_state
{
	TXT_STATE_NONE = 0,
	TXT_STATE_DRAG,
	TXT_STATE_RESIZE_LEFT,
	TXT_STATE_RESIZE_RIGHT
};


// MACROS //
#define POINT_IN_RECT(x, y, rx, ry, rw, rh) ((x) >= (rx) && (x) < ((rx) + (rw)) && (y) >= (ry) && (y) < ((ry) + (rh)))
#define TEXT_RECT_HEIGHT 26
#define TEXT_RECT_SPACE 10



// Main Functions
static void
doodles_page_class_init(	gpointer	klass,
							gpointer	klass_data)
{
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
}


static void
doodles_page_instance_init(	GTypeInstance*	instance,
							gpointer		klass)
{
	DoodlesPage* self = DOODLES_PAGE(instance);
	
	self->cursor_x = 0;
	self->cursor_y = 0;
	self->prev_cursor_x = 0;
	self->prev_cursor_y = 0;
	self->tool_data = NULL;
	self->prev_used_tool = TOOL_NONE;
	
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
	self->layer_work = doodles_canvas_new(pWidth, pHeight, self);
	self->layer_highlighter = doodles_canvas_new(pWidth, pHeight, self);
	self->layer_pen = doodles_canvas_new(pWidth, pHeight, self);
	self->layer_graphics = doodles_canvas_new(pWidth, pHeight, self);
	self->layer_background = doodles_canvas_new(pWidth, pHeight, self);
	doodles_canvas_set_background(	self->layer_background,
									BG_CHECKERED);
	
	doodles_canvas_set_child(	self->layer_work,
								self->layer_highlighter);
	
	doodles_canvas_set_child(	self->layer_highlighter,
								self->layer_pen);
	
	doodles_canvas_set_child(	self->layer_pen,
								self->layer_graphics);
	
	doodles_canvas_set_child(	self->layer_graphics,
								self->layer_background);
	
	
	//GtkWidget* phh = gtk_label_new("# Bruh\nBruh moment\n<small>Small text</small>");
	GtkWidget* phh = gtk_text_view_new();
	doodles_canvas_add_widget(	self->layer_graphics,
								phh,
								1, 1.2,
								3, 2);
	
	
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

static void
tool_clear_data(DoodlesPage* self)
{
	switch (self->prev_used_tool)
	{
		case (TOOL_PEN):
			GSList* l = self->tool_data;
			while (l != NULL)
			{
				free(l->data);
				
				l = l->next;
			}
			g_slist_free(self->tool_data);
			break;
		case (TOOL_TEXT):
			free(self->tool_data);
			break;
	}
	
	self->tool_data = NULL;
}

static void
pen_event(	DoodlesPage*	self,
			gint			gdk_event_button,
			gint			gdk_event_type)
{
	// pen_event is for both pen & highlighter, as they have the same function anyways
	
	switch (gdk_event_type)
	{
		case (GDK_MOTION_NOTIFY):
			if (self->mouse_pressed[0] && self->tool_data != NULL)
			{
				STR_POINT* point = malloc(sizeof(STR_POINT));
				if (point != NULL)
				{
					point->x = self->cursor_x;
					point->y = self->cursor_y;
					
					GSList* list = g_slist_append(self->tool_data, point);
					
					self->tool_data = list;
				}
			}
			
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_PRESS):
			if (gdk_event_button == 1 && self->tool_data == NULL)
			{
				STR_POINT* point = malloc(sizeof(STR_POINT));
				if (point != NULL)
				{
					point->x = self->cursor_x;
					point->y = self->cursor_y;
					
					GSList* list = g_slist_append(NULL, point);
					
					self->tool_data = list;
				}
			}
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
		case (GDK_BUTTON_RELEASE):
			if (gdk_event_button == 1 && self->tool_data != NULL)
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
						
						return;
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
				tool_clear_data(self);
			}
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			break;
	}
}


static gdouble
dist_to_line(	gdouble sx, gdouble sy,
				gdouble ex, gdouble ey,
				gdouble px, gdouble py)
{
	// Point?
	if (sx == ex && sy == ey)
		return sqrt( (sx - px)*(sx - px) + (sy - py)*(sy - py) );
	
	// Prepare distances to borders
	gdouble dis_a = sqrt( (sx - px)*(sx - px) + (sy - py)*(sy - py) );
	gdouble dis_b = sqrt( (ex - px)*(ex - px) + (ey - py)*(ey - py) );
	gdouble dis_short = (dis_a < dis_b ? dis_a : dis_b);
	
	// Horizontal/Vertical line?
	if (sx == ex)
	{
		if (py >= sy && py <= ey || py >= ey && py <= sy)
			return fabs( sx - px );
		else
			return dis_short;
	}
	else if (sy == ey)
	{
		if (px >= sx && px <= ex || px >= ex && px <= sx)
			return fabs( sy - py );
		else
			return dis_short;
	}
	
	// Curve
	gdouble m = (ey - sy) / (ex - sx);
	gdouble b = sy - ( m * sx );
	gdouble pnt_m = -1.0 / m;
	gdouble pnt_b = py - ( pnt_m * px );
	
	// line is orthogonal to pnt-line
	
	// Cut-point
	gdouble cut_x = ( b - pnt_b ) / ( pnt_m - m );
	gdouble cut_y = m * cut_x + b;
	
	// Cut between lines?
	if (cut_x >= sx && cut_x <= ex || cut_x >= ex && cut_x <= sx)
	{
		return sqrt( (cut_x - px)*(cut_x - px) + (cut_y - py)*(cut_y - py) );
	}
	
	// ...Otherwise get shortest distance between the end-points
	return dis_short;
}


static void
erase_line_from_list(	gdouble sx, gdouble sy,
						gdouble ex, gdouble ey,
						gdouble			radius,
						STR_LIST*		list,
						DoodlesCanvas*	canvas)
{
	// list->data is a STR_LINE
	
	while (list != NULL)
	{
		STR_LIST* ph_next = list->next; // list-entry might be removed, so śtore this beforehand
		
		STR_LINE* line = list->data;
		
		// In bouding box?
		if ((sx + radius) >= line->x && (sx - radius) <= (line->x + line->w) || (ex + radius) >= line->x && (ex - radius) <= (line->x + line->w))
		{
			if ((sy + radius) >= line->y && (sy - radius) <= (line->y + line->h) || (ey + radius) >= line->y && (ey - radius) <= (line->y + line->h))
			{
				// Check all points
				gboolean has_cut = FALSE;
				for (gint i = 0; i < line->points_length; i++)
				{
					STR_POINT* pnt = line->points[i];
					gdouble dis = dist_to_line(	sx, sy,
												ex, ey,
												pnt->x, pnt->y);
					
					
					if (dis <= radius)
					{
						pnt->x = -1000;
						pnt->y = -1000;
						has_cut = TRUE;
					}
				}
				
				if (has_cut)
				{
					// Remove points at (-1000|-1000)
					gint cut_start = -1;
					gint cut_end = -1;
					for (gint i = (line->points_length - 1); i >= -1; i--)
					{
						// Get point
						STR_POINT* pnt = NULL;
						if (i != -1)
							pnt = line->points[i];
						
						// Get start&end indices
						if (cut_end == -1 && pnt != NULL && pnt->x != -1000 && pnt->y != -1000)
						{
							cut_end = i;
						}
						else if (cut_end != -1 && (pnt == NULL || pnt->x == -1000 && pnt->y == -1000))
						{
							cut_start = i + 1;
						}
						
						if (cut_end != -1 && cut_start != -1)
						{
							// Create new line from cutout
							gint new_length = cut_end - cut_start + 1;
							STR_POINT** new_points = malloc(sizeof(STR_POINT*) * new_length);
							
							// Copy old values
							for (gint k = cut_start; k <= cut_end; k++)
							{
								new_points[k - cut_start] = line->points[k];
							}
							
							// Insert new line
							doodles_canvas_add_line(	canvas,
														new_points,
														new_length,
														line->size,
														line->r, line->g, line->b, line->a);
							
							// Clear
							cut_end = -1;
							cut_start = -1;
						}
					}
					
					// Remove old line
					for (gint i = 0; i < line->points_length; i++)
					{
						STR_POINT* pnt = line->points[i];
						
						if (pnt->x == -1000 && pnt->y == -1000)
						{
							free(pnt);
						}
					}
					free(line->points);
					
					doodles_canvas_remove_line(	canvas,
												line,
												false);
				}
			}
		}
		
		
		// Next
		list = ph_next;
	}
}


static void
eraser_event(	DoodlesPage*	self,
				gint			gdk_event_button,
				gint			gdk_event_type)
{
	switch (gdk_event_type)
	{
		case (GDK_BUTTON_PRESS):
		case (GDK_BUTTON_RELEASE):
		case (GDK_MOTION_NOTIFY):
			if (self->mouse_pressed[0] == TRUE || gdk_event_type == GDK_BUTTON_RELEASE && gdk_event_button == 1)
			{
				erase_line_from_list(	self->prev_cursor_x, self->prev_cursor_y,
										self->cursor_x, self->cursor_y,
										doodles_gui_controller_get_size(self->gui_controller) / 2,
										doodles_canvas_get_data_lines(self->layer_highlighter),
										self->layer_highlighter);
				
				erase_line_from_list(	self->prev_cursor_x, self->prev_cursor_y,
										self->cursor_x, self->cursor_y,
										doodles_gui_controller_get_size(self->gui_controller) / 2,
										doodles_canvas_get_data_lines(self->layer_pen),
										self->layer_pen);
				
				gtk_widget_queue_draw(GTK_WIDGET(self->layer_pen));
			}
			
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			
			break;
	}
}


static void
text_event(	DoodlesPage*	self,
			gint			gdk_event_button,
			gint			gdk_event_type)
{
	// Alloc tool data if empty
	if (self->tool_data == NULL)
	{
		self->tool_data = malloc(sizeof(struct _tool_text_data));
		struct _tool_text_data* ttd = self->tool_data;
		ttd->container = NULL;
		ttd->state = TXT_STATE_NONE;
	}
	
	switch (gdk_event_type)
	{
		case (GDK_BUTTON_PRESS):
		case (GDK_BUTTON_RELEASE):
		case (GDK_MOTION_NOTIFY):
			struct _tool_text_data* ttd = self->tool_data;
			
			// ..Is dragging?
			if (ttd->state == TXT_STATE_DRAG && gdk_event_type == GDK_MOTION_NOTIFY)
			{
				(ttd->container)->x = self->cursor_x - ttd->local_x;
				(ttd->container)->y = self->cursor_y - ttd->local_y;
				doodles_canvas_widget_realloc(self->layer_graphics, ttd->container);
				goto skip_to_draw;
			}
			
			// ..TXT_STATE_RESIZE_LEFT?
			// TODO
			
			// ..TXT_STATE_RESIZE_RIGHT?
			// TODO
			
			// ..Is still focusing text field??
			if (ttd->container != NULL)
			{
				STR_WIDGET_CONTAINER* c = ttd->container;
				
				gdouble ph_space = TEXT_RECT_SPACE / doodles_canvas_get_pixel_per_cm();
				gdouble ph_height = TEXT_RECT_HEIGHT / doodles_canvas_get_pixel_per_cm();
				if (!POINT_IN_RECT(	self->cursor_x,
									self->cursor_y,
									c->x - ph_space,
									c->y - ph_height - ph_space,
									c->w + ph_space * 2,
									c->h + ph_height + ph_space * 2))
				{
					ttd->container = NULL;
					ttd->state = TXT_STATE_NONE;
				}
			}
			
			// Find new text field to focus on
			if (ttd->container == NULL)
			{
				STR_WIDGET_CONTAINER* cont = doodles_canvas_get_widget_list(self->layer_graphics);
				
				while (cont != NULL)
				{
					if (POINT_IN_RECT(self->cursor_x, self->cursor_y, cont->x, cont->y, cont->w, cont->h))
					{
						ttd->container = cont;
						ttd->state = TXT_STATE_NONE;
						
						break;
					}
					
					cont = cont->next;
				}
			}
			
			// Start drag?
			if (ttd->state == TXT_STATE_NONE && gdk_event_type == GDK_BUTTON_PRESS && ttd->container != NULL)
			{
				STR_WIDGET_CONTAINER* c = ttd->container;
				
				gdouble ph_space = TEXT_RECT_SPACE / doodles_canvas_get_pixel_per_cm();
				gdouble ph_height = TEXT_RECT_HEIGHT / doodles_canvas_get_pixel_per_cm();
				if (POINT_IN_RECT(	self->cursor_x,
									self->cursor_y,
									c->x - ph_space,
									c->y - ph_height - ph_space,
									c->w + ph_space * 2,
									ph_height))
				{
					ttd->state = TXT_STATE_DRAG;
					
					ttd->local_x = self->cursor_x - c->x;
					ttd->local_y = self->cursor_y - c->y;
				}
			}
			
			// End drag?
			if (ttd->state == TXT_STATE_DRAG && gdk_event_type == GDK_BUTTON_RELEASE && ttd->container != NULL)
			{
				ttd->state = TXT_STATE_NONE;
			}
			
			
			// Draw
			skip_to_draw:
			gtk_widget_queue_draw(GTK_WIDGET(self->layer_work));
			
			break;
	}
}


void
doodles_page_receive_event(	DoodlesPage*	self,
							gdouble			mouse_x,
							gdouble			mouse_y,
							gint			gdk_event_button,
							gint			gdk_event_type)
{
	// No tool selected?
	if (doodles_gui_controller_get_tool(self->gui_controller) == 0)
		return;
	
	// From pixel to cm
	pos_to_cm(&mouse_x, &mouse_y);
	
	// Update mouse-variables
	switch (gdk_event_type)
	{
		case (GDK_BUTTON_PRESS):
			self->prev_cursor_x = mouse_x;
			self->prev_cursor_y = mouse_y;
			self->cursor_x = mouse_x;
			self->cursor_y = mouse_y;
			self->mouse_pressed[gdk_event_button - 1] = TRUE;
			break;
		case (GDK_BUTTON_RELEASE):
			self->prev_cursor_x = mouse_x;
			self->prev_cursor_y = mouse_y;
			self->cursor_x = mouse_x;
			self->cursor_y = mouse_y;
			self->mouse_pressed[gdk_event_button - 1] = FALSE;
			break;
		case (GDK_MOTION_NOTIFY):
			self->prev_cursor_x = self->cursor_x;
			self->prev_cursor_y = self->cursor_y;
			self->cursor_x = mouse_x;
			self->cursor_y = mouse_y;
			break;
	}
	
	// Clear old tool data
	if (self->prev_used_tool != doodles_gui_controller_get_tool(self->gui_controller))
	{
		if (self->prev_used_tool != TOOL_NONE)
			tool_clear_data(self);
		
		self->prev_used_tool = doodles_gui_controller_get_tool(self->gui_controller);
	}
	
	// Call tool event
	switch (doodles_gui_controller_get_tool(self->gui_controller))
	{
		case (TOOL_PEN):
		case (TOOL_HIGHLIGHTER):
			pen_event(	self,
						gdk_event_button,
						gdk_event_type);
			break;
		case (TOOL_ERASER):
			eraser_event(	self,
							gdk_event_button,
							gdk_event_type);
			break;
		case (TOOL_TEXT):
			text_event(	self,
						gdk_event_button,
						gdk_event_type);
			break;
	}
}




////////////////
// DRAW TOOLS //
////////////////

static void
on_draw_pen(	DoodlesPage*	self,
				cairo_t*		cairo,
				gdouble w, gdouble h,
				gdouble			ppc) // ppc == pixel per cm
{
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
}

static void
on_draw_text(	DoodlesPage*	self,
				cairo_t*		cairo,
				gdouble w, gdouble h,
				gdouble			ppc)
{
	struct _tool_text_data* ttd = self->tool_data;
	if (ttd != NULL && ttd->container != NULL)
	{
		STR_WIDGET_CONTAINER* cont = ttd->container;
		
		
		// Title bar
		gboolean did_fill = FALSE;
		title_bar_shape:
		cairo_rectangle (	cairo,
							cont->x * doodles_canvas_get_pixel_per_cm() - TEXT_RECT_SPACE,
							cont->y * doodles_canvas_get_pixel_per_cm() - TEXT_RECT_HEIGHT - TEXT_RECT_SPACE,
							cont->w * doodles_canvas_get_pixel_per_cm() + TEXT_RECT_SPACE * 2,
							TEXT_RECT_HEIGHT);
		if (!did_fill)
		{
			did_fill = TRUE;
			cairo_set_source_rgba(cairo, 0.4, 0.4, 0.4, 0.4);
			cairo_fill(cairo);
			goto title_bar_shape;
		}
		
		
		// Content
		cairo_rectangle (	cairo,
							cont->x * doodles_canvas_get_pixel_per_cm() - TEXT_RECT_SPACE,
							cont->y * doodles_canvas_get_pixel_per_cm() - TEXT_RECT_SPACE,
							cont->w * doodles_canvas_get_pixel_per_cm() + TEXT_RECT_SPACE * 2,
							cont->h * doodles_canvas_get_pixel_per_cm() + TEXT_RECT_SPACE * 2);
		
		
		cairo_set_line_width(cairo, 1.0);
		cairo_set_source_rgba(cairo, 0.4, 0.4, 0.4, 1.0);
		cairo_stroke(cairo);
	}
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
	
	
	switch (doodles_gui_controller_get_tool(self->gui_controller))
	{
		case (TOOL_PEN):
		case (TOOL_HIGHLIGHTER):
		case (TOOL_ERASER): // eraser has no tool_data so its alright
			on_draw_pen(	self,
							cairo,
							w, h,
							m);
			break;
		case (TOOL_TEXT):
			on_draw_text(	self,
							cairo,
							w, h,
							m);
			break;
	}
	
	
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