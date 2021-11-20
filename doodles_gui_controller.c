#include "doodles_gui_controller.h"


// Structs
typedef struct _ToolContainer
{
	GtkWidget*	button;
	GtkWidget*	box;
	GList*		box_children;
} ToolContainer;

struct _DoodlesGuiControllerClass
{
	GObjectClass parent;
	
	// Private static attributes
};

struct _DoodlesGuiController
{
	GObject			parent;
	
	// Private attributes
	GtkBox*			page_box;
	
	
	guint			current_tool;
	GtkWidget*		action_bar;
	GtkBox*			tool_box; // tool buttons are in here
	ToolContainer*	tool_pen;
	ToolContainer*	tool_highlighter;
	ToolContainer*	tool_eraser;
	ToolContainer*	tool_text;
	ToolContainer*	tool_selection;
};


// Prototypes
static GList* insert_button(	GtkBox*		parent,
								GList*		list,
								gchar*		icon);

static ToolContainer* make_tool_pen();
static void make_tool_pen_box(ToolContainer* container);

static ToolContainer* make_tool_highlighter();
static void make_tool_highlighter_box(ToolContainer* container);

static ToolContainer* make_tool_eraser();
static void make_tool_eraser_box(ToolContainer* container);

static ToolContainer* make_tool_text();
static ToolContainer* make_tool_selection();
static void on_color_button_clicked(	DoodlesPopupButton* self,
										gpointer user_data);
static void on_tool_button_clicked(	GtkButton*	button,
									gpointer	user_data);
static ToolContainer* doodles_gui_controller_get_tool_container(DoodlesGuiController* self);


// Color button
static GtkWidget*
create_color_button(gdouble r, gdouble g, gdouble b);

static void
color_button_draw(	DoodlesPopupButton*	button,
					GtkSnapshot*		snap,
					gdouble				w,
					gdouble				h);

static void
color_button_on_color_activated(	GtkColorChooser*	color_chooser,
									GdkRGBA*			color,
									gpointer			user_data);




// Main Functions
static void
dispose(GObject* object)
{
	// ...
}


static void
doodles_gui_controller_class_init(	gpointer	klass,
									gpointer	klass_data)
{
	GObjectClass* g_class = G_OBJECT_CLASS(klass);
	g_class->dispose = dispose;
}


static void
doodles_gui_controller_instance_init(	GTypeInstance*	instance,
										gpointer		klass)
{
	DoodlesGuiController* self = DOODLES_GUI_CONTROLLER(instance);
	
	// Pages (debug)
	self->page_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 16));
	gtk_widget_set_margin_top(GTK_WIDGET(self->page_box), 15);
	gtk_widget_set_margin_bottom(GTK_WIDGET(self->page_box), 15);
	
	DoodlesPage* page = doodles_page_new(self, 18, 24);
	gtk_box_append(self->page_box, doodles_page_get_widget(page));
	
	
	// Tool Box
	self->tool_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	self->current_tool = TOOL_PEN;
	
	
	// Tools
	self->tool_pen = make_tool_pen();
	gtk_box_append(self->tool_box, (self->tool_pen)->button);
	self->tool_highlighter = make_tool_highlighter();
	gtk_box_append(self->tool_box, (self->tool_highlighter)->button);
	self->tool_eraser = make_tool_eraser();
	gtk_box_append(self->tool_box, (self->tool_eraser)->button);
	
	// ..tool signals
	g_signal_connect((self->tool_pen)->button, "clicked", G_CALLBACK(on_tool_button_clicked), self);
	g_signal_connect((self->tool_highlighter)->button, "clicked", G_CALLBACK(on_tool_button_clicked), self);
	g_signal_connect((self->tool_eraser)->button, "clicked", G_CALLBACK(on_tool_button_clicked), self);
	
	
	// Prepare first tool
	make_tool_pen_box(self->tool_pen);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((self->tool_pen)->button), TRUE);
	
	
	// Action bar
	self->action_bar = gtk_action_bar_new();
	gtk_action_bar_set_center_widget(GTK_ACTION_BAR(self->action_bar), GTK_WIDGET(self->tool_box));
	
	
	gtk_action_bar_pack_end(GTK_ACTION_BAR(self->action_bar), (self->tool_pen)->box);
}


GType
doodles_gui_controller_get_type()
{
	static GType type = 0;
	
	if (type == 0)
	{
		const GTypeInfo info =
		{
			.class_size		= sizeof(struct _DoodlesGuiControllerClass),
			.base_init		= NULL,
			.base_finalize	= NULL,
			.class_init		= doodles_gui_controller_class_init,
			.class_finalize	= NULL,
			.class_data		= NULL,
			.instance_size	= sizeof(struct _DoodlesGuiController),
			.instance_init	= doodles_gui_controller_instance_init,
			.n_preallocs	= 0
		};
		
		type = g_type_register_static(	G_TYPE_OBJECT,
										"DoodlesGuiController",
										&info,
										0);
	}
	
	return type;
}


// Constructor
DoodlesGuiController*
doodles_gui_controller_new()
{
	GObject* new = g_object_new(DOODLES_TYPE_GUI_CONTROLLER, NULL);
	
	return DOODLES_GUI_CONTROLLER(new);
}


// Other functions


// Returns the root widget of the Gui Controller
GtkWidget*
doodles_gui_controller_get_widget(DoodlesGuiController* self)
{
	// FOREGROUND //
	
	GtkWidget* fg_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	// Action Bar
	//GtkWidget* fg_action = gtk_action_bar_new();
	//gtk_action_bar_set_center_widget(GTK_ACTION_BAR(fg_action), GTK_WIDGET(self->tool_box));
	
	gtk_box_append(GTK_BOX(fg_box), self->action_bar);
	//gtk_widget_set_vexpand(fg_box, TRUE);
	
	
	// CONTENT VIEW //
	
	GtkWidget* content_clamp = adw_clamp_new(); // centers the widget (page-list)
	GtkWidget* content_window = gtk_scrolled_window_new(); // scrolling
	gtk_widget_set_vexpand(content_window, TRUE);
	
	// Insert childs
	adw_clamp_set_child(ADW_CLAMP(content_clamp), GTK_WIDGET(self->page_box));
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(content_window), content_clamp);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(content_window), 150);
	
	
	// COMBINE //
	
	gtk_box_append(GTK_BOX(fg_box), content_window);
	gtk_widget_set_valign(fg_box, GTK_ALIGN_FILL);
	
	
	// RETURN //
	
	return fg_box;
}


void
doodles_gui_controller_destroy(DoodlesGuiController* self) // DELETE LATER
{
	/*GList* list = g_list_first(self->tool_buttons);
	while (list != NULL)
	{
		gtk_button_set_child(GTK_BUTTON(list->data), NULL);
		
		list = list->next;
	}
	
	g_list_free(self->tool_buttons);*/
}


static GList*
insert_button(	GtkBox*		parent,
				GList*		list,
				gchar*		icon)
{
	GtkWidget* button = gtk_toggle_button_new();
	
	GtkWidget* img = gtk_image_new_from_file(icon);
	gtk_button_set_child(GTK_BUTTON(button), img);
	
	gtk_box_append(parent, button);
	
	return g_list_append(list, button);
}


static ToolContainer*
make_tool_pen()
{
	// Malloc container
	ToolContainer* container = malloc(sizeof(ToolContainer)); // TODO check for NULL?
	
	// Button
	container->button = gtk_toggle_button_new();
	GtkWidget* img = gtk_image_new_from_file("files/icons/pencil.svg");
	gtk_button_set_child(GTK_BUTTON(container->button), img);
	
	
	return container;
}
static void
make_tool_pen_box(ToolContainer* container)
{
	container->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	
	// Child list
	GtkWidget* clr1 = create_color_button(0.0, 0.0, 0.0);
	GtkWidget* clr2 = create_color_button(1.0, 0.0, 0.0);
	GtkWidget* clr3 = create_color_button(0.2, 0.2, 1.0);
	doodles_popup_button_set_state(DOODLES_POPUP_BUTTON(clr1), TRUE);
	
	
	const char* icons[2];
	icons[0] = "media-record-symbolic";
	icons[1] = NULL;
	GtkWidget* scale = gtk_scale_button_new(0.1, 0.8, 0.1, icons);
	
	container->box_children = g_list_append(NULL, clr1);
	container->box_children = g_list_append(container->box_children, clr2);
	container->box_children = g_list_append(container->box_children, clr3);
	container->box_children = g_list_append(container->box_children, scale);
	
	
	// Insert to parent-widget
	gtk_box_append(GTK_BOX(container->box), clr1);
	gtk_box_append(GTK_BOX(container->box), clr2);
	gtk_box_append(GTK_BOX(container->box), clr3);
	gtk_box_append(GTK_BOX(container->box), scale);
	
	
	// Signals
	g_signal_connect(clr1, "toggle", G_CALLBACK(on_color_button_clicked), container);
	g_signal_connect(clr2, "toggle", G_CALLBACK(on_color_button_clicked), container);
	g_signal_connect(clr3, "toggle", G_CALLBACK(on_color_button_clicked), container);
}

static ToolContainer*
make_tool_highlighter()
{
	// Malloc container
	ToolContainer* container = malloc(sizeof(ToolContainer)); // TODO check for NULL?
	
	// Button
	container->button = gtk_toggle_button_new();
	GtkWidget* img = gtk_image_new_from_file("files/icons/marker.svg");
	gtk_button_set_child(GTK_BUTTON(container->button), img);
	
	
	
	return container;
}
static void
make_tool_highlighter_box(ToolContainer* container)
{
	container->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	
	// Child list
	GtkWidget* clr1 = create_color_button(1.0, 1.0, 0.4);
	GtkWidget* clr2 = create_color_button(0.4, 1.0, 0.4);
	GtkWidget* clr3 = create_color_button(0.4, 0.4, 1.0);
	doodles_popup_button_set_state(DOODLES_POPUP_BUTTON(clr1), TRUE);
	
	
	const char* icons[2];
	icons[0] = "media-record-symbolic";
	icons[1] = NULL;
	GtkWidget* scale = gtk_scale_button_new(0.4, 1.0, 0.1, icons);
	
	container->box_children = g_list_append(NULL, clr1);
	container->box_children = g_list_append(container->box_children, clr2);
	container->box_children = g_list_append(container->box_children, clr3);
	container->box_children = g_list_append(container->box_children, scale);
	
	
	// Insert to parent-widget
	gtk_box_append(GTK_BOX(container->box), clr1);
	gtk_box_append(GTK_BOX(container->box), clr2);
	gtk_box_append(GTK_BOX(container->box), clr3);
	gtk_box_append(GTK_BOX(container->box), scale);
	
	
	// Signals
	g_signal_connect(clr1, "toggle", G_CALLBACK(on_color_button_clicked), container);
	g_signal_connect(clr2, "toggle", G_CALLBACK(on_color_button_clicked), container);
	g_signal_connect(clr3, "toggle", G_CALLBACK(on_color_button_clicked), container);
}

static ToolContainer*
make_tool_eraser()
{
	// Malloc container
	ToolContainer* container = malloc(sizeof(ToolContainer)); // TODO check for NULL?
	
	// Button
	container->button = gtk_toggle_button_new();
	GtkWidget* img = gtk_image_new_from_file("files/icons/eraser.svg");
	gtk_button_set_child(GTK_BUTTON(container->button), img);
	
	
	return container;
}
static void
make_tool_eraser_box(ToolContainer* container)
{
	container->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	
	// Child list
	const char* icons[2];
	icons[0] = "media-record-symbolic";
	icons[1] = NULL;
	GtkWidget* scale = gtk_scale_button_new(0.4, 1.0, 0.1, icons);
	
	container->box_children = g_list_append(NULL, scale);
	
	
	// Insert to parent-widget
	gtk_box_append(GTK_BOX(container->box), scale);
	
	
	// Signals
	// ...
}

static ToolContainer*
make_tool_text()
{
	return NULL;
}

static ToolContainer*
make_tool_selection()
{
	return NULL;
}


static void
on_color_button_clicked(	DoodlesPopupButton* self,
							gpointer user_data)
{
	ToolContainer* container = user_data;
	
	// Indices 0-2 are color-buttons
	GList* list = g_list_first(container->box_children);
	
	for (guint i = 0; i < 3; i++)
	{
		doodles_popup_button_set_state(	DOODLES_POPUP_BUTTON(list->data),
										FALSE);
		
		list = list->next;
	}
}


static void
on_tool_button_clicked(	GtkButton*	button,
						gpointer	user_data)
{
	DoodlesGuiController* self = DOODLES_GUI_CONTROLLER(user_data);
	
	// Disable all buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((self->tool_pen)->button), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((self->tool_highlighter)->button), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((self->tool_eraser)->button), FALSE);
	
	// Enable active button
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	
	// Change current tool
	ToolContainer* prev_container = doodles_gui_controller_get_tool_container(self);
	self->current_tool = TOOL_NONE;
	
	if (GTK_WIDGET(button) == (self->tool_pen)->button)
		self->current_tool = TOOL_PEN;
	else if (GTK_WIDGET(button) == (self->tool_highlighter)->button)
		self->current_tool = TOOL_HIGHLIGHTER;
	else if (GTK_WIDGET(button) == (self->tool_eraser)->button)
		self->current_tool = TOOL_ERASER;
	
	// Change tool config
	if (prev_container != NULL) {
		g_clear_pointer(&prev_container->box, gtk_widget_unparent);
		
		prev_container->box = NULL;
		g_list_free(prev_container->box_children);
		prev_container->box_children = NULL;
	}
	
	ToolContainer* container = doodles_gui_controller_get_tool_container(self);
	
	
	if (container != NULL)
	{
		switch (self->current_tool)
		{
			case (TOOL_PEN):
				make_tool_pen_box(container);
				break;
			case (TOOL_HIGHLIGHTER):
				make_tool_highlighter_box(container);
				break;
			case (TOOL_ERASER):
				make_tool_eraser_box(container);
				break;
		}
		
		gtk_action_bar_pack_end(GTK_ACTION_BAR(self->action_bar), container->box);
	}
}


// Configs
void
doodles_gui_controller_get_color(	DoodlesGuiController*	self,
									GdkRGBA*				color)
{
	switch (self->current_tool)
	{
		case (TOOL_PEN):
			GList* list = g_list_first(self->tool_pen->box_children);
			for (guint i = 0; i < 3; i++)
			{
				if (doodles_popup_button_get_state(DOODLES_POPUP_BUTTON(list->data)))
				{
					GtkWidget* color_chooser = doodles_popup_button_get_popover_child(DOODLES_POPUP_BUTTON(list->data));
					gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_chooser), color);
					
					return;
				}
				
				list = list->next;
			}
			
			//break;
		default:
			color->red = 0.0;
			color->green = 0.0;
			color->blue = 0.0;
			color->alpha = 1.0;
			break;
	}
}

gint
doodles_gui_controller_get_tool(DoodlesGuiController* self)
{
	return self->current_tool;
}

static ToolContainer*
doodles_gui_controller_get_tool_container(DoodlesGuiController* self)
{
	switch (self->current_tool)
	{
		case (TOOL_NONE):
			return NULL;
		case (TOOL_PEN):
			return self->tool_pen;
		case (TOOL_HIGHLIGHTER):
			return self->tool_highlighter;
		case (TOOL_ERASER):
			return self->tool_eraser;
	}
	
	return NULL;
}

gdouble
doodles_gui_controller_get_size(DoodlesGuiController* self)
{
	ToolContainer* container = doodles_gui_controller_get_tool_container(self);
	
	switch (self->current_tool)
	{
		case (TOOL_PEN):
		case (TOOL_HIGHLIGHTER):
		case (TOOL_ERASER):
			// Index 3 (last) is GtkScaleButton
			GList* list = g_list_last(container->box_children);
			GtkWidget* scale_button = GTK_WIDGET(list->data);
			
			return gtk_scale_button_get_value(GTK_SCALE_BUTTON(scale_button));
		default:
			return 0.1;
	}
}



// Color popup button
static GtkWidget*
create_color_button(gdouble r, gdouble g, gdouble b)
{
	// Create color chooser
	GtkWidget* color_chooser = gtk_color_chooser_widget_new();
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(color_chooser), FALSE);
	
	GdkRGBA* color = malloc(sizeof(GdkRGBA));
	color->red = r;
	color->green = g;
	color->blue = b;
	color->alpha = 1.0;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_chooser), color);
	
	// Create button
	DoodlesPopupButton* btn = DOODLES_POPUP_BUTTON(doodles_popup_button_new(color_chooser));
	doodles_popup_button_set_draw_func(btn, color_button_draw);
	
	// Connect signals
	g_signal_connect(	color_chooser,
						"color-activated",
						G_CALLBACK(color_button_on_color_activated),
						btn);
	
	// Done
	return GTK_WIDGET(btn);
}
static void
color_button_draw(	DoodlesPopupButton*	button,
					GtkSnapshot*		snap,
					gdouble				w,
					gdouble				h)
{
	GtkWidget* color_chooser = doodles_popup_button_get_popover_child(button);
	
	GdkRGBA color;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_chooser), &color);
	
	gtk_snapshot_append_color(	snap,
								&color,
								&GRAPHENE_RECT_INIT(0, 0, w, h));
}
static void
color_button_on_color_activated(	GtkColorChooser*	color_chooser,
									GdkRGBA*			color,
									gpointer			user_data)
{
	DoodlesPopupButton* btn = (DoodlesPopupButton*)user_data;
	
	// Close popup on selection
	doodles_popup_button_set_open(btn, FALSE);
	
	// Redraw newly chosen color
	gtk_widget_queue_draw(GTK_WIDGET(btn));
}















