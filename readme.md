Compile with `make`

(Ignore the text below, I'm just documenting it a bit for myself)

<br/>

# DoodlesContainer

Vertical box that carries DoodlesPage's. Allows scaling.
It will emit the following signals on DoodlesPage's:
* `void cursor_pressed(int button, double x, double y)`
* `void cursor_released(int button, double x, double y)`
* `void cursor_clicked(int button, double x, double y)`
* `void cursor_dragged(int button, double x, double y, double delta_x, double delta_y)`
* (I'll worry about keyboard-signals later)

TODO:
* Scaling via user-input (currently only works through code)
* Skip drawing on hidden widgets

****

# Canvas

This is a widget which can hold multiple kinds of drawings.

* Pen
* Highlighter
* Other widgets (Like images, text-fields or graphs)

They may also carry a child canvas ( doodles\_canvas\_set\_child(...) ) which will
be drawn behind them.

They're useless by themselves. The Page-class (see below) will handle all events
and work with the canvases.

<br/>

# Page

Creates a multi-layered canvas. Also handles all events (drawing, clicks, dragging, etc...)

Layering:

1. Working layer<br/>Unapplied changes are visible here
2. Highlighter layer
3. Pen layer
4. Graphics layer<br/>Images, Graphs, Texts, ...<br/>(They're all inside a "window")
5. Background image<br/>(Usually just a white page)

## Working Layer

"Overwrites" event & drawing functions.

* Freshly drawn lines are first shown here before moving to the pen layer
* Widget-Rectangles are being drawn here
* ..basically everything that's not part of the document (yet) will be drawn here...










