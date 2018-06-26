#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>

/* compile: gcc -o cairo_graph2 cairo_graph2.c `pkg-config --libs --cflags gtk+-2.0` 
 * run:		./cairo_graph2
 */

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

#define WIDTH 500
#define HEIGHT 500

#define AMPLITUDE 100

#define PLOTEXTENTS

cairo_surface_t *axis_surface;
cairo_surface_t *label_surface;
cairo_surface_t *grid_surface;
cairo_surface_t *data_surface;
cairo_surface_t *total_surface;

static int motion = 0;

static int x = 0, y = 0;

static int create_axis_surface(int width, int height) {

	cairo_t *cr;

	/* create the axis surface, which will be used lateron in the on_expose_event callback function */
	axis_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	cr = cairo_create(axis_surface);
	/* plot background */
	cairo_set_source_rgba(cr,0,0,0,0.0); /* translucent, do not copy */
	cairo_paint(cr);
	/* plot axis box */
	cairo_set_source_rgba(cr,1,1,1,1.0); /* opacity, do copy */
	cairo_rectangle(cr,DIST_AXIS2FIGURE_X,DIST_AXIS2FIGURE_Y,width-2*DIST_AXIS2FIGURE_X,height-2*DIST_AXIS2FIGURE_Y);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr,0,0,0,1.0); /* opacity, do copy */
	cairo_stroke(cr);

	cairo_destroy(cr);

	return 0;
}



static int create_label_surface(int width, int height) {

	cairo_t *cr;
	cairo_text_extents_t extents;

	label_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	cr = cairo_create(label_surface);

	/* plot background */
	cairo_set_source_rgba(cr,0,0,0,0.0); /* translucent, do not copy */
	cairo_paint(cr);

	/* set font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, FONTSIZE);
	cairo_set_source_rgba(cr,0,0,0,1.0); /* opaque, do copy */
	
	/* plot xlabel */
	cairo_text_extents(cr, "X label", &extents);
	cairo_move_to(cr, 0.5*width-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y+extents.height);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, "X label");
	cairo_rotate(cr,0.0);
	cairo_stroke(cr);

#ifdef PLOTEXTENTS
	/* plot xlabel extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to(cr, 0.5*width-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y+extents.height);
	cairo_rel_line_to (cr,extents.width,0);
	cairo_rel_line_to (cr,0,-extents.height);
	cairo_rel_line_to (cr,-extents.width,0);
	cairo_rel_line_to (cr,0,extents.height);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

	/* plot ylabel */
	cairo_text_extents(cr, "Y label", &extents);
	cairo_move_to (cr, DIST_AXIS2FIGURE_X-DIST_YLABEL2AXIS_X,0.5*height+0.5*extents.width);
	cairo_rotate(cr,-M_PI/2.0);
	cairo_show_text (cr, "Y label");
	cairo_rotate(cr,M_PI/2.0);

#ifdef PLOTEXTENTS
	/* plot ylabel extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to (cr, DIST_AXIS2FIGURE_X-DIST_YLABEL2AXIS_X,0.5*height+0.5*extents.width);
	cairo_rel_line_to (cr,0,-extents.width);
	cairo_rel_line_to (cr,-extents.height,0);
	cairo_rel_line_to (cr,0,extents.width);
	cairo_rel_line_to (cr,extents.height,0);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

	/* plot title */
	cairo_text_extents(cr, "Title", &extents);
	cairo_move_to (cr, 0.5*width-0.5*(extents.width),DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, "Title");
	cairo_rotate(cr,0.0);

#ifdef PLOTEXTENTS
	/* plot title extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to (cr, 0.5*width-0.5*(extents.width),DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y);
	cairo_rel_line_to (cr,extents.width,0);
	cairo_rel_line_to (cr,0,-extents.height);
	cairo_rel_line_to (cr,-extents.width,0);
	cairo_rel_line_to (cr,0,extents.height);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

	cairo_destroy(cr);

	return 0;
}

static int create_grid_surface(int width, int height) {

	gint i;
	
	cairo_t *cr;

	/* create the grid surface, which will be used lateron in the on_expose_event callback function */
	grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	cr = cairo_create(grid_surface);
	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);
	/* prepare line attributes: color and dashed */
	const double dashes[2] = {5,5};
	cairo_set_source_rgba(cr,0, 0, 0, 1.0); /* opaque, do copy */
	cairo_set_dash (cr, dashes, 2, 0);
	/* plot vertical grid lines */
	for(i=DIST_AXIS2FIGURE_X+20;i<=width-DIST_AXIS2FIGURE_X-20;i+=20) {
		cairo_move_to(cr,i,DIST_AXIS2FIGURE_Y+1);
		cairo_line_to(cr,i,height-DIST_AXIS2FIGURE_Y-1);
	}
	/* plot horizontal grid lines */
	for(i=DIST_AXIS2FIGURE_Y+20;i<=height-DIST_AXIS2FIGURE_Y-20;i+=20) {
		cairo_move_to(cr,DIST_AXIS2FIGURE_X+1,i);
		cairo_line_to(cr,width-DIST_AXIS2FIGURE_X-1,i);
	}
	cairo_stroke(cr);
	cairo_fill(cr);

	cairo_destroy(cr);
	
	return 0;
}



static int create_data_surface(int width, int height) {

	gint i;

	cairo_t *cr;

	/* create the data surface, which will be used lateron in the on_expose_event callback function */
	data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	cr = cairo_create(data_surface);
	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);
	/* plot data */
	cairo_set_source_rgba(cr,0, 0, 0, 1.0); /* opaque, do copy */
	cairo_move_to(cr,DIST_AXIS2FIGURE_X+1,height/2.0);
	for(i=DIST_AXIS2FIGURE_X+2;i<width-DIST_AXIS2FIGURE_X;i++) {
		cairo_line_to(cr,i,height/2.0+(1.0*rand()/RAND_MAX-0.5)*2.0*AMPLITUDE);
	}
	cairo_stroke(cr);
	cairo_fill(cr);

	cairo_destroy(cr);

	return 0;
}


static int create_total_surface(int width, int height) {

	cairo_t *cr;

	/* create the total surface, which will be used lateron in the on_expose_event callback function */
	total_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	cr = cairo_create(total_surface);
	
	/* set source surface to created axis surface (see on_realize callback function) */
	cairo_set_source_surface (cr, axis_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,axis_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created label surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 0, 0, 0);
//	cairo_set_source_surface (cr, label_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,label_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created grid surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
//	cairo_set_source_surface (cr, grid_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,grid_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created image surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 0, 0, 0);
//	cairo_set_source_surface (cr, data_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,data_surface, 0, 0);
	cairo_fill(cr);

	cairo_destroy(cr);
}


static void on_expose_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	printf("expose event triggered\n");
	cairo_t *cr;

	/* create cairo drawing */
	cr = gdk_cairo_create (widget->window);

	/* set source surface to created total surface (see on_realize callback function) */
	cairo_set_source_surface (cr, total_surface, 0, 0);
	cairo_paint(cr);

	/* handle mouse event */
	if(motion) {
		if(x>DIST_AXIS2FIGURE_X && x<WIDTH-DIST_AXIS2FIGURE_X && y>DIST_AXIS2FIGURE_Y && y<HEIGHT-DIST_AXIS2FIGURE_Y) {
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_move_to(cr,x,DIST_AXIS2FIGURE_Y+1);
			cairo_line_to(cr,x,HEIGHT-DIST_AXIS2FIGURE_Y);
			cairo_stroke(cr);
		}
	}

	cairo_destroy(cr);

	motion = 0;

	return;
}

static void on_realize(GtkWidget *widget, GdkEvent *event, gpointer data)
{
//	printf("realize event triggered\n");

	create_axis_surface(WIDTH,HEIGHT);
	create_label_surface(WIDTH,HEIGHT);
	create_grid_surface(WIDTH,HEIGHT);
	create_data_surface(WIDTH,HEIGHT);	

	create_total_surface(WIDTH,HEIGHT);


	return;
}

static void on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	motion = 1;
	x = event->x;
	y = event->y;
//	on_expose_event(widget, (GdkEvent*)event, NULL);
gtk_widget_queue_draw(widget);

	return;
}


int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;  

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(darea, "expose-event", G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(darea, "realize", G_CALLBACK(on_realize), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect (darea, "motion_notify_event",G_CALLBACK(on_motion_notify_event), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);

  /* turn on pointer motion mask */
  gtk_widget_set_events(darea, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

