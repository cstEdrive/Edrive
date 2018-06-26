#include <gtk/gtk.h>
#include <stdlib.h>

/* compile: gcc -o cairo_graph cairo_graph.c `pkg-config --libs --cflags gtk+-2.0` 
 * run:		./cairo_graph
 */

#define WIDTH 1000
#define HEIGHT 400
#define BORDER 30

#define AMPLITUDE 100

cairo_surface_t *axis_surface;
cairo_surface_t *label_surface;
cairo_surface_t *grid_surface;
cairo_surface_t *data_surface;
cairo_surface_t *total_surface;

static int motion = 0;

static void on_expose_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	//	printf("expose event triggered\n");
	cairo_t *cr;

	/* create cairo drawing */
	cr = gdk_cairo_create (widget->window);


	/* set source surface to created axis surface (see on_realize callback function) */
	cairo_set_source_surface (cr, axis_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,axis_surface, 0, 0);
	cairo_fill(cr);


	/* set source surface to created grid surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
//	cairo_set_source_surface (cr, grid_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,grid_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created image surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 1, 0, 0);
//	cairo_set_source_surface (cr, data_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,data_surface, 0, 0);
	cairo_fill(cr);

	/* handle mouse event */
	if(motion) {
		GdkEventMotion *event_motion = (GdkEventMotion *)event;
		if(event_motion->x>BORDER && event_motion->x<WIDTH-BORDER && event_motion->y>BORDER && event_motion->y<HEIGHT-BORDER) {
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_move_to(cr,event_motion->x,BORDER+1);
			cairo_line_to(cr,event_motion->x,HEIGHT-BORDER);
		}
	}
	cairo_stroke(cr);

	cairo_destroy(cr);

	motion = 0;

	return;
}

static void on_realize(GtkWidget *widget, GdkEvent *event, gpointer data)
{
//	printf("realize event triggered\n");
	cairo_t *cr;

	gint i;



	/* create the axis surface, which will be used lateron in the on_expose_event callback function */
	axis_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,WIDTH,HEIGHT);
	cr = cairo_create(axis_surface);
	/* plot background */
	cairo_set_source_rgba(cr,0,0,0,0.0); /* translucent, do not copy */
	cairo_paint(cr);	
	/* plot axis box */
	cairo_set_source_rgba(cr,1,1,1,1.0); /* opacity, do copy */
	cairo_rectangle(cr,BORDER,BORDER,WIDTH-2*BORDER,HEIGHT-2*BORDER);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr,0,0,0,1.0);
	cairo_stroke(cr);
	cairo_destroy(cr);



	/* create the grid surface, which will be used lateron in the on_expose_event callback function */
	grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,WIDTH,HEIGHT);
	cr = cairo_create(grid_surface);
	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);
	/* prepare line attributes: color and dashed */
	const double dashes[2] = {5,5};
	cairo_set_source_rgba(cr,0, 0, 0, 1.0); /* opaque, do copy */
	cairo_set_dash (cr, dashes, 2, 0);
	/* plot vertical grid lines */
	for(i=BORDER+20;i<=WIDTH-BORDER-20;i+=20) {
		cairo_move_to(cr,i,BORDER+1);
		cairo_line_to(cr,i,HEIGHT-BORDER-1);
	}
	/* plot horizontal grid lines */
	for(i=BORDER+20;i<=HEIGHT-BORDER-20;i+=20) {
		cairo_move_to(cr,BORDER+1,i);
		cairo_line_to(cr,WIDTH-BORDER-1,i);
	}
	cairo_stroke(cr);
	cairo_fill(cr);
	cairo_destroy(cr);



	/* create the data surface, which will be used lateron in the on_expose_event callback function */
	data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,WIDTH,HEIGHT);
	cr = cairo_create(data_surface);
	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);
	/* plot data */
	cairo_set_source_rgba(cr,0, 0, 0, 1.0); /* opaque, do copy */
	cairo_move_to(cr,BORDER+1,HEIGHT/2.0);
	for(i=BORDER+2;i<WIDTH-BORDER;i++) {
		cairo_line_to(cr,i,HEIGHT/2.0+(1.0*rand()/RAND_MAX-0.5)*2.0*AMPLITUDE);
	}
	cairo_stroke(cr);
	cairo_fill(cr);
	cairo_destroy(cr);



	/* create the data surface, which will be used lateron in the on_expose_event callback function */
	total_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,WIDTH,HEIGHT);
	cr = cairo_create(total_surface);
	/* set source surface to created axis surface (see on_realize callback function) */
	cairo_set_source_surface (cr, axis_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,axis_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created grid surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
//	cairo_set_source_surface (cr, grid_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,grid_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created image surface (see on_realize callback function) */
	cairo_set_source_rgb(cr, 1, 0, 0);
//	cairo_set_source_surface (cr, data_surface, 0, 0);
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,data_surface, 0, 0);
	cairo_fill(cr);

	return;
}

static void on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	motion = 1;
	on_expose_event(widget, (GdkEvent*)event, NULL);

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

