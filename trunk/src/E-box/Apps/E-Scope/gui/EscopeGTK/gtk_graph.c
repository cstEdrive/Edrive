#include "gtk_graph.h"

static void		gtk_graph_class_init(GtkGraphClass *klass);
static void		gtk_graph_init(GtkGraph *graph);
static void		gtk_graph_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void		gtk_graph_realize(GtkWidget *widget);
static gboolean gtk_graph_expose(GtkWidget *widget, GdkEvent *event, gpointer data);
static void 	gtk_graph_destroy(GtkObject *object);
static void		gtk_graph_plot_get_range(gdouble *range,gdouble *data,gdouble ndata);
static gdouble	gtk_graph_plot_nicenumber(gdouble value, gint round);
static gint		gtk_graph_plot_get_ticks(gdouble *range, gdouble** ticks, gint *exponent, gdouble *nicetick);
static void 	gtk_graph_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);
static void 	gtk_graph_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void 	gtk_graph_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data);

static gdouble gtk_graph_plot_nicenumber(gdouble value, gint round) {
  gint    exponent;
  gdouble fraction;
  gdouble nicefraction;

  exponent = (gint) floor(log10(value));
  fraction = value/pow(10, (gdouble)exponent);

  if (0) {
    if (fraction < 1.5) 
      nicefraction = 1.0;
    else if (fraction < 3.0)
      nicefraction = 2.0;
    else if (fraction < 7.0)
      nicefraction = 5.0;
    else
      nicefraction = 10.0;
   }
  else {
    if (fraction <= 1.0)
      nicefraction = 1.0;
    else if (fraction <= 2.0)
      nicefraction = 2.0;
    else if (fraction <= 5.0)
      nicefraction = 5.0;
    else
      nicefraction = 10.0;
   }

	return nicefraction*pow(10, (gdouble)exponent);
}

static gint gtk_graph_plot_get_ticks(gdouble *range, gdouble** ticks, gint *exponent, gdouble *nicetick)
{
	gint i, numticks;
	gdouble nicerange;

	/* Check for special cases */

	if (range[1] - range[0] <1e-10)
	{
		if (range[0]!=0)
		{
			range[0]=range[0]-0.01*range[0];
			range[1]=range[1]+0.01*range[1];
		} else {
			range[0]=-1;
			range[1]=1;
		}
	}

//	printf("range[0]=%f,range[1]=%f\n",range[0],range[1]);

	/* determine if x10^X is necessary */
	int temp1, temp2;
	if(range[0]) {
		temp1 = (gint) floor(log10(abs(range[0])));
//		printf("yes [0]\n");
	} else {
		temp1 = 0;
//		printf("no [0]\n");
	}
	if(range[1]) {
		temp2 = (gint) floor(log10(abs(range[1])));
//		printf("yes [1]\n");
	} else {
		temp2 = 0;
//		printf("no [1]\n");
	}
//	printf("t1 = %d,t2=%d\n",temp1,temp2);
	if (temp1>=temp2 && (temp1>=3 ||temp1<=-3)) {
		*exponent = temp1;
	} else if (temp2>=temp1 && (temp2>=3 ||temp2<=-3)) {
		*exponent = temp2;
	} else {
		*exponent = 0;
	}
		
//	printf("range[1]-range[0] = %f\n",range[1]-range[0]);
// 	/* Compute the new nice range and ticks */
//	nicerange = gtk_graph_plot_nicenumber(range[1] - range[0], 0);
//	printf("nicerange = %f\n",nicerange);
//	printf("nicerange/10 = %f\n",nicerange/10);
//	nicetick[0] = gtk_graph_plot_nicenumber(nicerange/(10), 1);
	nicetick[0] = gtk_graph_plot_nicenumber((range[1]-range[0])/(10), 1); /* try */
//	printf("nicetick = %f\n",nicetick[0]);

	/* Compute the new nice start and end values */
	range[0] = floor(range[0]/nicetick[0])*nicetick[0];
	range[1] = ceil(range[1]/nicetick[0])*nicetick[0];
//	printf("range[0]=%f,range[1]=%f\n\n",range[0],range[1]);

//	printf("NiceRange = %f\n",nicerange);
//	printf("NiceTick = %f\n",nicetick[0]);
//	printf("AxisStart = %f\n",range[0]);
//	printf("AxisEnd = %f\n",range[1]);
//	printf("NumTicks = %d\n",(gint)round((range[1]-range[0])/nicetick[0]+1));
	
	/* write NumTicks */
	numticks = (gint)round((range[1]-range[0])/nicetick[0]+1);

	/* write Ticks */
	*ticks = (double *)malloc(numticks*sizeof(double));
	for(i=0;i<numticks;i++)
	{	
		(*ticks)[i] = range[0]+i*nicetick[0];
	}

	return numticks;
}

static void gtk_graph_plot_get_range(gdouble *range, gdouble *data, gdouble ndata)
{
	gint i;

	range[0]=INFINITY;
	range[1]=-INFINITY;

	for(i=0;i<ndata;i++)
	{
		if(data[i]<range[0])
		{
			range[0]=data[i];
		}
		if(data[i]>range[1])
		{
			range[1]=data[i];
		}
	}
}

GtkWidget *gtk_graph_new(void)
{
	return GTK_WIDGET(gtk_type_new(gtk_graph_get_type()));
}

GtkType gtk_graph_get_type(void)
{
/* see also http://www.gtk.org/api/2.6/gtk/gtk-changes-1-2.html */
	static GtkType gtk_graph_type = 0;
	
	if (!gtk_graph_type) {
		static const GtkTypeInfo gtk_graph_info = {
			"GtkGraph",
			sizeof(GtkGraph),
			sizeof(GtkGraphClass),
			(GtkClassInitFunc) gtk_graph_class_init,
			(GtkObjectInitFunc) gtk_graph_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		gtk_graph_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_graph_info);
	}

	return gtk_graph_type;
}

static void gtk_graph_init(GtkGraph *graph)
{
	graph->tool = NONE;
	graph->zooming = ZOOMINGORIGINAL;
//	graph->zoomstart[0]=20;
//	graph->zoomstart[1]=20;
}

static void gtk_graph_class_init(GtkGraphClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;	

	object_class = (GtkObjectClass *) klass;
//	widget_class = (GtkWidgetClass *) klass;
	widget_class = GTK_WIDGET_CLASS(klass);	

	/* overwrite default signal handlers */
	widget_class->button_press_event = gtk_graph_button_press;
	widget_class->button_release_event = gtk_graph_button_release;
	widget_class->motion_notify_event = gtk_graph_motion_notify_event;
	widget_class->realize = gtk_graph_realize;
	widget_class->size_request = gtk_graph_size_request;
	widget_class->expose_event = gtk_graph_expose;

	object_class->destroy = gtk_graph_destroy;
}

static void gtk_graph_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkGraph * graph = (GtkGraph *)widget;
	printf("GtkGraph: button pressed\n");
	if(graph->tool==ZOOM)
	{	
		graph->zooming=ZOOMINGBUSY;
		graph->zoomstart[0]=event->x;
		graph->zoomstart[1]=event->y;
		printf("zooming started\np1 = (%f,%f), p2 = (%f,%f)\n",graph->zoomstart[0],graph->zoomstart[1],graph->zoomend[0],graph->zoomend[1]);
	}
	gtk_graph_expose(widget, (GdkEvent*)event, data);
}

static void gtk_graph_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkGraph * graph = (GtkGraph *)widget;
	printf("GtkGraph: button released\n");
	if(graph->tool==ZOOM)
	{	
		graph->zooming=ZOOMINGDONE;
		graph->zoomend[0]=event->x;
		graph->zoomend[1]=event->y;
		printf("zooming done\np1 = (%f,%f), p2 = (%f,%f)\n",graph->zoomstart[0],graph->zoomstart[1],graph->zoomend[0],graph->zoomend[1]);
		if (graph->zoomend[0]<graph->zoomstart[0]) {
			double temp;
			temp = graph->zoomend[0];
			graph->zoomend[0] = graph->zoomstart[0];
			graph->zoomstart[0] = temp; 
		}
		if (graph->zoomend[1]>graph->zoomstart[1]) {
			double temp;
			temp = graph->zoomend[1];
			graph->zoomend[1] = graph->zoomstart[1];
			graph->zoomstart[1] = temp; 
		}
	}
	gtk_graph_expose(widget, (GdkEvent*)event, data);
}

static void gtk_graph_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	GtkGraph * graph = (GtkGraph *)widget;

	graph->xmouse = event->x;
	graph->ymouse = event->y;

	if( (graph->tool==ZOOM && graph->zooming==ZOOMINGBUSY) || graph->tool==POINTER) {
		gtk_graph_expose(widget, (GdkEvent*)event, data);
	}
	return;

}

static void gtk_graph_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_GRAPH(widget));
  g_return_if_fail(requisition != NULL);

  requisition->width = DEFAULTWIDTH;
  requisition->height = DEFAULTHEIGHT;

	printf("size_request\n");
}

static void gtk_graph_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GRAPH(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;

	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(gtk_widget_get_parent_window (widget), &attributes, attributes_mask);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

static gboolean gtk_graph_expose(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_GRAPH(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	gtk_graph_paint(widget, event, NULL);

	return FALSE;
}

void gtk_graph_paint(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	GdkEventMotion *event_motion = (GdkEventMotion *)event;
	/* 1. create a cairo surface image */
	/* 2. plot to the image surface */
	/* 3. store image surface to png if necessary */
	/* 4. paint image surface to GTK drawing area */

	GtkGraph *graph = (GtkGraph *)widget;

	cairo_text_extents_t extents;
	cairo_surface_t *image;
	cairo_t *cr;

	gint i, width, height, exponent, numxticks, numyticks, temp;
	gdouble dashes[2] = {5.0, 5.0};
	gdouble nicetick, *xticks, *yticks;//, xdata2pix, ydata2pix;
	gchar tick[100];
	gchar str[100];

	/* determine size of drawing area */
	gdk_drawable_get_size(gtk_widget_get_window(widget), &width, &height);

	/*** CAIRO ***/

	/* create new cairo surface */
	image = cairo_image_surface_create(CAIRO_FORMAT_RGB24,width,height);

	/* create cairo context from image surface */
	cr = cairo_create(image);

	/* 1. AXIS */
	cairo_set_line_width(cr, 1);

	/* plot grey background */
	cairo_set_source_rgb(cr, 0.84, 0.84, 0.84);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	/* plot white axis box */
	cairo_set_line_width(cr, 1);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle(cr, DIST_AXIS2FIGURE_X, DIST_AXIS2FIGURE_Y, width-2*DIST_AXIS2FIGURE_X, height-2*DIST_AXIS2FIGURE_Y);
	cairo_fill(cr);

	/* plot black axis box border */
	cairo_set_line_width(cr, 1);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_rectangle(cr, DIST_AXIS2FIGURE_X, DIST_AXIS2FIGURE_Y, width-2*DIST_AXIS2FIGURE_X, height-2*DIST_AXIS2FIGURE_Y);
	cairo_stroke(cr);

	/* 2. LABELS */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, FONTSIZE);
	
	/* plot xlabel */
	cairo_text_extents(cr, graph->xlabel, &extents);
	cairo_move_to(cr, 0.5*width-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y+extents.height);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, graph->xlabel);
	cairo_rotate(cr,0.0);

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
	cairo_text_extents(cr, graph->ylabel, &extents);
	cairo_move_to (cr, DIST_AXIS2FIGURE_X-DIST_YLABEL2AXIS_X,0.5*height+0.5*extents.width);
	cairo_rotate(cr,-M_PI/2.0);
	cairo_show_text (cr, graph->ylabel);
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
	cairo_text_extents(cr, graph->title, &extents);
	cairo_move_to (cr, 0.5*width-0.5*(extents.width),DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, graph->title);
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

	/* 3. TICKS AND GRID */

	if(graph->tool==ZOOM) {
		if(graph->zooming==ZOOMINGDONE) {
			printf("graph->zooming==ZOOMINGDONE\n");
			printf("xrange before = [%f,%f]\n",graph->xrange[0],graph->xrange[1]);
			printf("xdata2pix = %f\n",graph->xdata2pix);
			double oldxrange[2],oldyrange[2];
			oldxrange[0]=graph->xrange[0];
			oldxrange[1]=graph->xrange[1];
			oldyrange[0]=graph->yrange[0];
			oldyrange[1]=graph->yrange[1];
			if(graph->zoomstart[0]!=graph->zoomend[0] && graph->zoomstart[1]!=graph->zoomend[1]) {
				graph->xrange[0]=(graph->zoomstart[0]-DIST_AXIS2FIGURE_X)/graph->xdata2pix+oldxrange[0];
				graph->xrange[1]=(graph->zoomend[0]-DIST_AXIS2FIGURE_X)/graph->xdata2pix+oldxrange[0];
				graph->yrange[0]=(height-DIST_AXIS2FIGURE_Y-graph->zoomstart[1])/graph->ydata2pix+oldyrange[0];
				graph->yrange[1]=(height-DIST_AXIS2FIGURE_Y-graph->zoomend[1])/graph->ydata2pix+oldyrange[0];
				printf("xrange after = [%f,%f]\n",graph->xrange[0],graph->xrange[1]);
				printf("yrange after = [%f,%f]\n",graph->yrange[0],graph->yrange[1]);
			}
		} else if(graph->zooming==ZOOMINGBUSY) {
//			printf("graph->zooming==ZOOMINGBUSY\n");
		}
	}

	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, FONTSIZETICKS);

	/* get x range */
	if (graph->zooming==ZOOMINGORIGINAL) {
		gtk_graph_plot_get_range(graph->xrange,graph->xdata,graph->ndata);
	}

	/* get x ticks */
	numxticks = gtk_graph_plot_get_ticks(graph->xrange,&xticks,&exponent,&nicetick);

	/* determine significance for rounding tick labels */
	temp = (gint)floor(log10(nicetick));
	temp = temp-exponent;
	if (temp<0) {
		sprintf(str,"%%.%df",-temp);
	} else {
		sprintf(str,"%%.%df",0);
	}
	/* plot x ticks */
	for(i=0;i<numxticks;i++)
	{		
		sprintf(tick,str,xticks[i]/pow(10,exponent));
		cairo_text_extents(cr,tick, &extents);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(numxticks-1)*i-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+extents.height+DIST_XTICKS2AXIS_Y);
		cairo_show_text (cr, tick);

#ifdef PLOTEXTENTS
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(numxticks-1)*i-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+extents.height+DIST_XTICKS2AXIS_Y);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif
	}

	/* x exponent */
	if(exponent) {
		cairo_text_extents(cr,"x10", &extents);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, width-DIST_AXIS2FIGURE_X+DIST_XEXPONENT2AXIS_X,height-DIST_AXIS2FIGURE_Y+extents.height/2.0);
		cairo_show_text (cr, "x10");

#ifdef PLOTEXTENTS
		/* plot exponent extents */
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, width-DIST_AXIS2FIGURE_X+DIST_XEXPONENT2AXIS_X,height-DIST_AXIS2FIGURE_Y+extents.height/2.0);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
	
		sprintf(tick,"%d",exponent);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, width-DIST_AXIS2FIGURE_X+DIST_XEXPONENT2AXIS_X+extents.width,height-DIST_AXIS2FIGURE_Y-extents.height/2.0);
		cairo_show_text (cr, tick);

#ifdef PLOTEXTENTS
		/* plot exponent extents */
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, width-DIST_AXIS2FIGURE_X+DIST_XEXPONENT2AXIS_X+extents.width,height-DIST_AXIS2FIGURE_Y-extents.height/2.0);
		cairo_text_extents(cr,tick, &extents);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, FONTSIZETICKS);
	}

	/* plot x grid */
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_set_dash (cr, dashes, 2, 0);
	cairo_set_line_width (cr, 1.0);
	for ( i = 1; i < numxticks-1; i++ ) {
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(numxticks-1)*i,height-DIST_AXIS2FIGURE_Y);
		cairo_line_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(numxticks-1)*i,DIST_AXIS2FIGURE_Y);
	}
	cairo_stroke(cr);
	cairo_set_dash (cr, dashes, 0, 0);
	free(xticks);

	/* get y range */
	if (graph->zooming==ZOOMINGORIGINAL) {
		gtk_graph_plot_get_range(graph->yrange,graph->ydata,graph->ndata);
	}

	/* get y ticks */
	numyticks = gtk_graph_plot_get_ticks(graph->yrange,&yticks,&exponent,&nicetick);

	/* determine significance for rounding tick labels */
	temp = (gint)floor(log10(nicetick));
	temp = temp-exponent;
	if (temp<0) {
		sprintf(str,"%%.%df",-temp);
	} else {
		sprintf(str,"%%.%df",0);
	}

	/* plot y ticks */
	for(i=0;i<numyticks;i++)
	{
		sprintf(tick,str,yticks[numyticks-1-i]/pow(10,exponent));
		cairo_text_extents(cr,tick, &extents);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(numyticks-1)*i+(extents.height)/2.0);
		cairo_show_text (cr, tick);
		cairo_stroke(cr);

#ifdef PLOTEXTENTS
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(numyticks-1)*i+(extents.height)/2.0);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif
	}

	/* y exponent */
	if(exponent) {
		cairo_text_extents(cr,"x10", &extents);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,DIST_AXIS2FIGURE_Y-DIST_YEXPONENT2AXIS_Y);
		cairo_show_text (cr, "x10");

#ifdef PLOTEXTENTS
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,DIST_AXIS2FIGURE_Y-DIST_YEXPONENT2AXIS_Y);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
	
		sprintf(tick,"%d",exponent);

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+extents.width,DIST_AXIS2FIGURE_Y-DIST_YEXPONENT2AXIS_Y-extents.height);
		cairo_show_text (cr, tick);
#ifdef PLOTEXTENTS
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+extents.width,DIST_AXIS2FIGURE_Y-DIST_YEXPONENT2AXIS_Y-extents.height);
		cairo_text_extents(cr,tick, &extents);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif

		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cr, FONTSIZETICKS);
	}

	/* plot y grid */
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_set_dash (cr, dashes, 2, 0);
	cairo_set_line_width (cr, 1.0);
	for ( i = 1; i < numyticks-1; i++ ) {
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,(double)DIST_AXIS2FIGURE_Y+(double)(height-2*DIST_AXIS2FIGURE_Y)/(numyticks-1)*i);
		cairo_line_to (cr, width-DIST_AXIS2FIGURE_X,(double)DIST_AXIS2FIGURE_Y+(double)(height-2*DIST_AXIS2FIGURE_Y)/(numyticks-1)*i);
	}
	cairo_stroke (cr);
	cairo_set_dash (cr, dashes, 0, 0);
	free(yticks);

	/* scaling factor from data to pixels */
	graph->xdata2pix = (width-2.0*DIST_AXIS2FIGURE_X)/(graph->xrange[1]-graph->xrange[0]);

	/* scaling factor from data to pixels */
	graph->ydata2pix = (height-2.0*DIST_AXIS2FIGURE_Y)/(graph->yrange[1]-graph->yrange[0]);

	/* plot data */
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgb(cr, 0, 0, 0);
	for(i=0;i<graph->ndata;i++)
	{
		if(graph->xdata[i]>=graph->xrange[0] && graph->xdata[i]<=graph->xrange[1] && graph->ydata[i]>=graph->yrange[0] && graph->ydata[i]<=graph->yrange[1]) {
			cairo_line_to(cr, DIST_AXIS2FIGURE_X+(graph->xdata[i]-graph->xrange[0])*graph->xdata2pix, height-DIST_AXIS2FIGURE_Y-(graph->ydata[i]-graph->yrange[0])*graph->ydata2pix);
		} else {
			cairo_stroke(cr);
		}
	}
	cairo_stroke(cr);

	/* store image surface to png if necessary */
	if(graph->save_to_image){
		cairo_surface_write_to_png(image, "/home/jbest/Desktop/image.png");
		graph->save_to_image=0;
	}

	/* mouse position */
	if (graph->tool==POINTER && graph->xmouse>DIST_AXIS2FIGURE_X && graph->xmouse<width-DIST_AXIS2FIGURE_X && graph->ymouse>DIST_AXIS2FIGURE_Y && graph->ymouse<height-DIST_AXIS2FIGURE_Y) {
		cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
		cairo_set_line_width (cr, 2.0);
		cairo_move_to (cr, graph->xmouse,DIST_AXIS2FIGURE_Y);
		cairo_line_to (cr, graph->xmouse,height-DIST_AXIS2FIGURE_Y);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,graph->ymouse);
		cairo_line_to (cr, width-DIST_AXIS2FIGURE_X,graph->ymouse);
		cairo_stroke (cr);

		cairo_set_source_rgb(cr, 1, 1, 1);
		gchar string[100];
		sprintf(string,"(%.1f, %.1f)",(graph->xmouse-DIST_AXIS2FIGURE_X)/graph->xdata2pix+graph->xrange[0],(height-DIST_AXIS2FIGURE_Y-graph->ymouse)/graph->ydata2pix+graph->yrange[0]);
		cairo_text_extents(cr, string, &extents);
		cairo_stroke(cr);
		cairo_rectangle(cr, graph->xmouse+10-2, graph->ymouse-10-extents.height-2, extents.width+4, extents.height+4);
		cairo_fill(cr);
	
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_move_to (cr, graph->xmouse+10,graph->ymouse-10);
		cairo_show_text (cr, string);
	} else if (graph->tool==ZOOM) {
		if(graph->zooming==ZOOMINGBUSY) {
			cairo_set_source_rgb(cr, 1, 0, 0);
			cairo_set_line_width (cr, 1.0);

			if(event_motion->x>width-DIST_AXIS2FIGURE_X) {
				event_motion->x = width-DIST_AXIS2FIGURE_X;
				graph->zoomend[0]=event_motion->x;
			} 
			if(event_motion->y>height-DIST_AXIS2FIGURE_Y) {
				event_motion->y=height-DIST_AXIS2FIGURE_Y;
				graph->zoomend[1]=event_motion->y;
			}

			cairo_rectangle(cr, graph->zoomstart[0], graph->zoomstart[1], event_motion->x-graph->zoomstart[0], event_motion->y-graph->zoomstart[1]);
			cairo_stroke(cr);

		} else if (graph->zooming==ZOOMINGDONE) {
				graph->zooming=ZOOMINGREADY;
		}
	/*else {
			if(event_motion->x>width-DIST_AXIS2FIGURE_X) {
				event_motion->x = width-DIST_AXIS2FIGURE_X;
				graph->zoomend[0]=event_motion->x;
			} 
			if(event_motion->y>height-DIST_AXIS2FIGURE_Y) {
				event_motion->y=height-DIST_AXIS2FIGURE_Y;
				graph->zoomend[1]=event_motion->y;
			}
//			printf("zoomstart = %f, %f\n",graph->zoomstart[0],graph->zoomstart[1]);
//			printf("zoomend   = %f, %f\n",graph->zoomend[0],graph->zoomend[1]);
		}*/
	}


	/* destroy cairo context */
	cairo_destroy(cr);

	/*** GTK ***/

	/* create cairo context GTK drawing area surface */
	cr = gdk_cairo_create(widget->window);

	/* paint image surface to GTK drawing area */
	cairo_set_source_surface (cr, image, 0, 0);
	cairo_paint(cr);

	/* destroy cairo context */
	cairo_destroy(cr);
	
	/* destroy cair surface image */
	cairo_surface_destroy(image);

	return;
}

static void gtk_graph_destroy(GtkObject *object)
{
	GtkGraph *graph;
	GtkGraphClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_GRAPH(object));

	graph = GTK_GRAPH(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

/* functions used by external code */

void gtk_graph_start_zooming(GtkWidget *widget)
{
	GtkGraph *graph = (GtkGraph *)widget;
	graph->tool = ZOOM;
	printf("start zooming\n");
	printf("p1 = (%f,%f),p2=(%f,%f)\n",graph->zoomstart[0],graph->zoomstart[1],graph->zoomend[0],graph->zoomend[1]);
}

void gtk_graph_stop_zooming(GtkWidget *widget)
{
	GtkGraph *graph = (GtkGraph *)widget;
	graph->tool = POINTER;
}

void gtk_graph_plot(GtkWidget *widget, gdouble *xdata, gdouble *ydata, gint ndata)
{
	GtkGraph *graph = (GtkGraph *)widget;

	graph->xdata = xdata;
	graph->ydata = ydata;
	graph->ndata = ndata;

	gtk_widget_queue_draw(widget);
}

void gtk_graph_xlabel(GtkWidget *widget, gchar *xlabel)
{
	GtkGraph *graph = (GtkGraph *)widget;
	graph->xlabel = xlabel;
	
	gtk_widget_queue_draw(widget);
}

void gtk_graph_ylabel(GtkWidget *widget, gchar *ylabel)
{
	GtkGraph *graph = (GtkGraph *)widget;
	graph->ylabel = ylabel;

	gtk_widget_queue_draw(widget);
}

void gtk_graph_title(GtkWidget *widget, gchar *title)
{
	GtkGraph *graph = (GtkGraph *)widget;
	graph->title = title;

	gtk_widget_queue_draw(widget);
}
