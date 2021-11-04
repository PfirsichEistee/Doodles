#ifndef DOODLES_TEXT_FIELD_H
#define DOODLES_TEXT_FIELD_H


// Includes
#include<adwaita.h>


// Macros
#define DOODLES_TYPE_TEXT_FIELD				( doodles_text_field_get_type() )
#define DOODLES_TEXT_FIELD(obj)				( G_TYPE_CHECK_INSTANCE_CAST((obj), DOODLES_TYPE_TEXT_FIELD, DoodlesTextField) )
#define DOODLES_TEXT_FIELD_CLASS(klass)		( G_TYPE_CHECK_CLASS_CAST((klass), DOODLES_TYPE_TEXT_FIELD, DoodlesTextFieldClass) )
#define DOODLES_IS_TEXT_FIELD(obj)			( G_TYPE_CHECK_INSTANCE_TYPE((obj), DOODLES_TYPE_TEXT_FIELD) )
#define DOODLES_IS_TEXT_FIELD_CLASS(klass)	( G_TYPE_CHECK_KLASS_TYPE((klass), DOODLES_TYPE_TEXT_FIELD) )
#define DOODLES_TEXT_FIELD_GET_CLASS(obj)	( G_TYPE_INSTANCE_GET_CLASS((obj), DOODLES_TYPE_TEXT_FIELD, DoodlesTextFieldClass) )


// Typedefs
typedef struct _DoodlesTextField		DoodlesTextField;
typedef struct _DoodlesTextFieldClass	DoodlesTextFieldClass;


// Prototypes
GType doodles_text_field_get_type();
GtkWidget* doodles_text_field_new();



#endif