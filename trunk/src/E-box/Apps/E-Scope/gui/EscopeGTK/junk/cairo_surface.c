#include <gtk/gtk.h>

/* compile: gcc -o cairo_surface cairo_surface.c `pkg-config --libs --cflags gtk+-2.0` 
 * run:		./cairo_surface
 */

cairo_surface_t *image;

static void on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
//	printf("expose event triggered\n");
	cairo_t *cr;

	/* create cairo drawing */
	cr = gdk_cairo_create (widget->window);

	/* set source surface to created image surface (see on_realize callback function) */
	cairo_set_source_surface (cr, image, 0, 0);
	/* paint this surface */
	cairo_paint(cr);

	/* go on and draw other stuff (some simple line) */
    cairo_set_line_width(cr,1);
    cairo_set_source_rgb(cr,0,0,0);
	cairo_move_to(cr,0,0);
	cairo_line_to(cr,200,200);
	cairo_stroke(cr);

	cairo_destroy(cr);

	return;
}

static void on_realize(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
//	printf("realize event triggered\n");
	cairo_t *cr;

	gint i;

	/* create the image surface, which will be used lateron in the on_expose_event callback function */
	image = cairo_image_surface_create(CAIRO_FORMAT_RGB24,200,200);

	cr = cairo_create(image);
	
	/* put background to green */
	cairo_set_source_rgb(cr,0,1,0);
	cairo_paint(cr);

	/* plot many horizontal and vertical lines */
	cairo_set_line_width(cr,1);
	cairo_set_source_rgb(cr,0,0,0);
	for(i=0;i<200;i+=3)
	{
		cairo_move_to(cr,i,0);
		cairo_line_to(cr,i,200);
	}
	for(i=0;i<200;i+=3)
	{
		cairo_move_to(cr,0,i);
		cairo_line_to(cr,200,i);
	}
	cairo_stroke(cr);

	cairo_destroy(cr);

	return;
}

static void on_motion_notify_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	/*trigger the expose event */
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
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  /* turn on pointer motion mask */
  gtk_widget_set_events(darea, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

