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
static ToolContainer* make_tool_highlighter();
static ToolContainer* make_tool_eraser();
static ToolContainer* make_tool_text();
static ToolContainer* make_tool_selection();
static void on_color_button_clicked(	DoodlesColorButton* self,
										gpointer user_data);


// Enums
enum tool {
	TOOL_NONE = 0,
	TOOL_PEN,
	TOOL_HIGHLIGHTER,
	TOOL_ERASER,
	TOOL_TEXT,
	TOOL_GRAPH,
	TOOL_SELECTION
};



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
doodles_gui_controller_destroy(DoodlesGuiController* self)
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
	
	// Box
	container->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	
	// Child list
	GtkWidget* clr1 = doodles_color_button_new(1.0, 0.0, 0.0);
	GtkWidget* clr2 = doodles_color_button_new(1.0, 0.0, 0.0);
	GtkWidget* clr3 = doodles_color_button_new(1.0, 0.0, 0.0);
	
	
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
	
	
	
	return container;
}

static ToolContainer*
make_tool_highlighter()
{
	return NULL;
}

static ToolContainer*
make_tool_eraser()
{
	return NULL;
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
on_color_button_clicked(	DoodlesColorButton* self,
							gpointer user_data)
{
	ToolContainer* container = user_data;
	
	// Indices 0-2 are color-buttons
	GList* list = g_list_first(container->box_children);
	
	for (guint i = 0; i < 3; i++)
	{
		doodles_color_button_set_state(	DOODLES_COLOR_BUTTON(list->data),
										FALSE);
		
		list = list->next;
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
				if (doodles_color_button_get_state(DOODLES_COLOR_BUTTON(list->data)))
				{
					doodles_color_button_get_color(DOODLES_COLOR_BUTTON(list->data), color);
					
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

gdouble
doodles_gui_controller_get_size(DoodlesGuiController* self)
{
	switch (self->current_tool)
	{
		case (TOOL_PEN):
			// Index 3 (last) is GtkScaleButton
			GList* list = g_list_last(self->tool_pen->box_children);
			GtkWidget* scale_button = GTK_WIDGET(list->data);
			
			return gtk_scale_button_get_value(GTK_SCALE_BUTTON(scale_button));
		default:
			return 0.1;
	}
}