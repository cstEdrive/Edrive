/* gtk_graph.h */

#ifndef __GTK_GRAPH_H__
#define __GTK_GRAPH_H__

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define GTK_GRAPH(obj) GTK_CHECK_CAST(obj, gtk_graph_get_type (), GtkGraph)
#define GTK_GRAPH_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_graph_get_type(), GtkGraphClass)
#define GTK_IS_GRAPH(obj) GTK_CHECK_TYPE(obj, gtk_graph_get_type())

#define max(A,B) ((A)>(B)?(A):(B))
#define min(A,B) ((A)<(B)?(A):(B))
#define abs(A) ((A)>0?(A):-(A))

#define DEFAULTWIDTH 			400				/* default width of the graph */
#define DEFAULTHEIGHT 			300				/* default height of the graph */

#define FONTSIZE 				12				/* fontsize for tekst */
#define FONTSIZETICKS 			FONTSIZE-2		/* fontsize for ticks */
#define FONTSIZETICKSEXPONENT 	FONTSIZE-3 		/* fontsize for exponent of ticks */

#define DIST_TEXT2AXIS			0.5*FONTSIZE
#define DIST_AXIS2FIGURE_X		50				/* horizontal distance between the axis and the figure */
#define DIST_AXIS2FIGURE_Y		50				/* vertical distance between the axis and the figure */
#define DIST_XLABEL2AXIS_Y		2*FONTSIZE		/* vertical distance between (the top of) the xlabel and the axis */
#define DIST_YLABEL2AXIS_X		37				/* horizontal distance between (the right of) the ylabel and the axis */
#define DIST_TITLE2AXIS_Y		2*FONTSIZE		/* vertical distance between (the bottom) of the title and the axis */
#define DIST_XTICKS2AXIS_Y		DIST_TEXT2AXIS	/* vertical distance between (the top) of the xticks and the axis */
#define DIST_YTICKS2AXIS_X		DIST_TEXT2AXIS	/* horizontal distance between (the right) of the yticks and the axis */
#define DIST_XEXPONENT2AXIS_X	DIST_TEXT2AXIS	/* horizontal distance between (the left) of the x exponent and the axis */
#define DIST_YEXPONENT2AXIS_Y	DIST_TEXT2AXIS	/* vertical distance between (the bottom) of the y exponent and the axis */

#define PLOTEXTENTS

/* tools */
enum {
NONE = 0,
POINTER,
ZOOM,
NTOOLS
};

/* zooming states */
enum {
ZOOMINGORIGINAL=0,
ZOOMINGREADY,
ZOOMINGBUSY,
ZOOMINGDONE,
NZOOMINGSTATES
};

typedef struct _GtkGraph GtkGraph;
typedef struct _GtkGraphClass GtkGraphClass;

struct _GtkGraph {
	GtkWidget widget;
	gchar *xlabel;
	gchar *ylabel;
	gchar *title;
	gdouble *xdata;
	gdouble *ydata;
	gdouble ndata;
	gdouble xrange[2];
	gdouble yrange[2];
	gdouble xmouse;
	gdouble ymouse;
	gint save_to_image;
	gint tool;
	gint zooming;
	gdouble zoomstart[2];
	gdouble zoomend[2];
	gdouble xdata2pix;
	gdouble ydata2pix;
};

struct _GtkGraphClass {
	GtkWidgetClass parent_class;
};

/* prototypes */
GtkType gtk_graph_get_type(void);
GtkWidget *gtk_graph_new();
void gtk_graph_start_zooming(GtkWidget *); /* arguments: graph */
void gtk_graph_stop_zooming(GtkWidget *); /* arguments: graph */
void gtk_graph_plot(GtkWidget *, gdouble *, gdouble *, gint); /* arguments: graph, x, y, n */
void gtk_graph_xlabel(GtkWidget *, gchar *); /* arguments: graph, xlabel */
void gtk_graph_ylabel(GtkWidget *, gchar *); /* arguments: graph, ylabel */
void gtk_graph_title(GtkWidget *, gchar *); /* arguments: graph, title */
void gtk_graph_paint(GtkWidget *, GdkEvent*, gpointer);  /* arguments: graph, event, optional data */

G_END_DECLS

#endif /* __GKT_GRAPH_H__ */
