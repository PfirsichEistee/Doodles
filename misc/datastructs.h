#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H


typedef struct _STR_POINT
{
	gdouble		x;
	gdouble		y;
} STR_POINT;

typedef struct _STR_LINE
{
	gdouble		x; // Bounding box
	gdouble		y;
	gdouble		w;
	gdouble		h;
	gdouble		size; // Line width
	gdouble		r, g, b, a; // Color
	STR_POINT**	points; // Array of STR_POINT
	guint		points_length;
} STR_LINE;

typedef struct _STR_LIST
{
	gpointer			data;
	struct _STR_LIST*	next;
} STR_LIST;

typedef struct _STR_WIDGET_CONTAINER
{
	GtkWidget*						widget;
	gdouble							x;
	gdouble							y;
	gdouble							w;
	gdouble							h;
	struct _STR_WIDGET_CONTAINER*	next;
} STR_WIDGET_CONTAINER;


#endif