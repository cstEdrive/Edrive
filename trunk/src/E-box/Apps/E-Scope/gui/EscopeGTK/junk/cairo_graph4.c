#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include "../../../engine/engine.h"
#include "../../../incomm/incomm.h"

/* requires: 	dvipng package
 * compile: 	gcc -Wall -g -o cairo_graph4 cairo_graph4.c `pkg-config --libs --cflags gtk+-2.0` -lm -lincomm_client -lpthread
 * run:			./cairo_graph4
 */

#define FONTSIZE 				14				/* fontsize for tekst */
#define FONTSIZETICKS 			FONTSIZE-2		/* fontsize for ticks */
#define FONTSIZETICKSEXPONENT 	FONTSIZE-3 		/* fontsize for exponent of ticks */

#define DIST_TEXT2AXIS			0.7*FONTSIZE
#define DIST_AXIS2FIGURE_X		70				/* horizontal distance between the axis and the figure */
#define DIST_AXIS2FIGURE_Y		70				/* vertical distance between the axis and the figure */
#define DIST_XLABEL2AXIS_Y		2*FONTSIZE		/* vertical distance between (the top of) the xlabel and the axis */
#define DIST_YLABEL2AXIS_X		45				/* horizontal distance between (the right of) the ylabel and the axis */
#define DIST_TITLE2AXIS_Y		2*FONTSIZE		/* vertical distance between (the bottom) of the title and the axis */
#define DIST_XTICKS2AXIS_Y		DIST_TEXT2AXIS	/* vertical distance between (the top) of the xticks and the axis */
#define DIST_YTICKS2AXIS_X		DIST_TEXT2AXIS	/* horizontal distance between (the right) of the yticks and the axis */
#define DIST_XEXPONENT2AXIS_X	DIST_TEXT2AXIS	/* horizontal distance between (the left) of the x exponent and the axis */
#define DIST_YEXPONENT2AXIS_Y	DIST_TEXT2AXIS	/* vertical distance between (the bottom) of the y exponent and the axis */

#define WIDTH 500
#define HEIGHT 400

#define N 4096
#define AMPLITUDE 100

#define DEC (void *(*)(void*))

#define min(A,B) (A)<(B)?(A):(B)
#define max(A,B) (A)>(B)?(A):(B)

//#define PLOTEXTENTS /* plot boundaries around labels */
//#define LATEXDEMO /* plot labels in with LaTeX */
//#define DEBUG /* only printf is DEBUG flag is set */

cairo_surface_t *axis_surface = NULL;
cairo_surface_t *label_surface = NULL;
cairo_surface_t *grid_surface = NULL;
cairo_surface_t *data_surface = NULL;
cairo_surface_t *total_surface = NULL;

gdouble t0;

/* plot types */
enum {
LINEAR = 0,
LOG,
NPLOTTYPES
};

/* tools */
enum {
NONE = 0,
POINT,
ZOOM,
NTOOLS
};

/* tool states */
enum {
ZOOM_READY=0,
ZOOM_BUSY,
NZOOMINGSTATES
};

/* ticks_t structure */
typedef struct {
	gint tick_n; /* number of ticks */
	gint tick_exponent; /* exponent of ticks */
	gdouble tick_step; /* step of ticks */
	gint tick_step_significance; /* step significance of ticks */
	gdouble *tick_labels; /* ticks labels */
} ticks_t;

/* graph_t structure */
typedef struct {
	gchar *xlabel; /* string for xlabel */
	gchar *ylabel; /* string for ylabel */
	gchar *title; /* string for title */
	gdouble *xdata; /* x data */
	gdouble *ydata; /* y data */
	gint ndata; /* number of data points */
	gdouble xrange[2]; /* range of x data */
	gdouble yrange[2]; /* range of y data */
	gdouble xrangezoom[2]; /* zoomed range of x data */
	gdouble yrangezoom[2]; /* zoomed range of y data */
	gint mouse[2]; /* mouse position [pix] */
	ticks_t *xticks; /* ticks for x axis */
	ticks_t *yticks; /* ticks for y axis */
	gint tool; /* tool: zoom or point */
	gint tool_state; /* state: busy or something */
	gint xtype; /* linear or log x axis */
	gint ytype; /* linear or log y axis */
} graph_t;

static graph_t graph;
GtkWidget *window;
	GtkWidget *darea;


#include "tde.c"
static ptde_shm pthe_shm;


static int start_timer(void) {
	struct timeval t;	
	gettimeofday(&t,NULL);
	t0 = t.tv_sec+t.tv_usec/1000000.0;
	return 0;
}

static int stop_timer(void) {
	struct timeval t;	
	gettimeofday(&t,NULL);
#ifdef DEBUG
	printf("dt = %f\n",(t.tv_sec+t.tv_usec/1000000.0)-t0);
#endif
	return 0;
}

static gdouble nicenumber(gdouble value) {

	gint exponent;
	gdouble fraction;
	gdouble nicefraction;

	exponent = (gint) floor(log10(value));
	fraction = value/pow(10, (gdouble)exponent);

    if (fraction <= 1.0) {
      nicefraction = 1.0;
    } else if (fraction <= 2.0) {
      nicefraction = 2.0;
    } else if (fraction <= 5.0) {
      nicefraction = 5.0;
    } else {
      nicefraction = 10.0;
 	}

	return nicefraction*pow(10, (gdouble)exponent);
}

static ticks_t *create_ticks(gdouble *range) {

	gint i, exp0, exp1;

	/* allocate memory for ticks structure */
	ticks_t *ticks = (ticks_t *)malloc(sizeof(ticks_t));

	/* check for special case if range is just one value */
	if(range[1] - range[0] <1e-10) {
		if(range[0]!=0) {
			range[0] = range[0]-0.01*range[0];
			range[1] = range[1]+0.01*range[1];
		} else {
			range[0] = -1;
			range[1] = 1;
		}
	}

	/* calculate tick step: try to divide the range in roughly 10 steps */
	ticks->tick_step = nicenumber((range[1]-range[0])/10.0);

	/* adapt range */
	range[0] = floor(range[0]/ticks->tick_step)*ticks->tick_step;
	range[1] = ceil(range[1]/ticks->tick_step)*ticks->tick_step;
	
	/* calculate number of ticks */
	ticks->tick_n = (gint)round((range[1]-range[0])/ticks->tick_step+1);

	/* allocate memory for tick labels */
	ticks->tick_labels = (gdouble *)malloc(ticks->tick_n*sizeof(gdouble));

	/* calculate ticks */
	for(i=0;i<ticks->tick_n;i++)
	{	
		ticks->tick_labels[i] = range[0]+i*ticks->tick_step;
	}

	/* determine exponent */
	if(range[0]) {
		exp0 = (gint) floor(log10(fabs(range[0])));
	} else {
		exp0 = 0;
	}
	if(range[1]) {
		exp1 = (gint) floor(log10(fabs(range[1])));
	} else {
		exp1 = 0;
	}
	if(exp0 && exp1) {
		ticks->tick_exponent = max(exp0,exp1);
	} else if(exp0 && !exp1) {
		ticks->tick_exponent = exp0;
	} else if(exp1 && !exp0) {
		ticks->tick_exponent = exp1;
	} else {
		ticks->tick_exponent = 0;
	}

	if(fabs(ticks->tick_exponent)<3) {
		ticks->tick_exponent = 0;
	}

	/* determine step significance for rounding of tick labels */
	if((gint)floor(log10(ticks->tick_step))-ticks->tick_exponent<0) {
		ticks->tick_step_significance = -((gint)floor(log10(ticks->tick_step))-ticks->tick_exponent);
	} else {
		ticks->tick_step_significance = 0;
	}

	return ticks;
	
}

static int destroy_ticks(ticks_t *ticks) {

	free(ticks->tick_labels);
	free(ticks);

	return 0;
}

#ifdef LATEXDEMO
static int latexstring2png(gchar* latexstring) {

	FILE* fp;

	fp = fopen("temp.tex","w");
	fprintf(fp,"\\documentclass{article}\n");
	fprintf(fp,"\\pagestyle{empty}\n");
	fprintf(fp,"\\usepackage{amsmath}\n");
	fprintf(fp,"\\usepackage{amssymb}\n");
	fprintf(fp,"\\usepackage{amsfonts}\n");
	fprintf(fp,"\\begin{document}\n");
	fprintf(fp,"\\textbf{%s}\n",latexstring);
	fprintf(fp,"\\end{document}\n");
	fclose(fp);

	system("latex temp.tex > /dev/null");
	system("dvipng -T tight -z 9 --gamma 4 -q* -x 1200 -D 95 -bg Transparent -o temp.png temp.dvi > /dev/null");
	system("rm -f temp.aux");
	system("rm -f temp.dvi");
	system("rm -f temp.log");
	system("rm -f temp.aux");
	system("rm -f temp.tex");

	return 0;
}
#endif

static int create_axis_surface(GtkWidget *widget) {

	cairo_t *cr;
	gint width, height;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the axis surface, which will be used lateron in the on_expose_event callback function */
	if(axis_surface) {
		cairo_surface_destroy(axis_surface);
		axis_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		axis_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
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

static int create_label_surface(GtkWidget *widget) {

	cairo_t *cr;
	gint width, height;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	if(label_surface) {
		cairo_surface_destroy(label_surface);
		label_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		label_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(label_surface);
	
#ifdef LATEXDEMO /* LaTeX code for label */

	cairo_surface_t *surface;

	/* title */
	latexstring2png(graph.title);
	surface = cairo_image_surface_create_from_png("temp.png");
	
	cairo_set_source_surface(cr,surface,width/2.0-cairo_image_surface_get_width(surface)/2.0,DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y-cairo_image_surface_get_height(surface));
	cairo_mask_surface(cr,surface,width/2.0-cairo_image_surface_get_width(surface)/2.0,DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y-cairo_image_surface_get_height(surface));
	cairo_fill(cr);
#ifdef PLOTEXTENTS
	/* plot title extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to(cr, width/2.0-cairo_image_surface_get_width(surface)/2.0,DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y);
	cairo_rel_line_to (cr,cairo_image_surface_get_width(surface),0);
	cairo_rel_line_to (cr,0,-cairo_image_surface_get_height(surface));
	cairo_rel_line_to (cr,-cairo_image_surface_get_width(surface),0);
	cairo_rel_line_to (cr,0,cairo_image_surface_get_height(surface));
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif	
	cairo_surface_destroy(surface);
	
	/* xlabel */
	latexstring2png(graph.xlabel);
	surface = cairo_image_surface_create_from_png("temp.png");
	
	cairo_set_source_surface(cr,surface,width/2.0-cairo_image_surface_get_width(surface)/2.0,height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y);
	cairo_mask_surface(cr,surface,width/2.0-cairo_image_surface_get_width(surface)/2.0,height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y);
	cairo_fill(cr);
#ifdef PLOTEXTENTS
	/* plot xlabel extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to(cr, width/2.0-cairo_image_surface_get_width(surface)/2.0,height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y+cairo_image_surface_get_height(surface));
	cairo_rel_line_to (cr,cairo_image_surface_get_width(surface),0);
	cairo_rel_line_to (cr,0,-cairo_image_surface_get_height(surface));
	cairo_rel_line_to (cr,-cairo_image_surface_get_width(surface),0);
	cairo_rel_line_to (cr,0,cairo_image_surface_get_height(surface));
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif	
	cairo_surface_destroy(surface);

	/* ylabel */
	latexstring2png(graph.ylabel);
	surface = cairo_image_surface_create_from_png("temp.png");
	
	cairo_rotate(cr,-M_PI/2.0);
	cairo_set_source_surface(cr,surface,-height/2.0-cairo_image_surface_get_width(surface)/2.0,DIST_AXIS2FIGURE_X-cairo_image_surface_get_height(surface)-DIST_YLABEL2AXIS_X);
	cairo_mask_surface(cr,surface,-height/2.0-cairo_image_surface_get_width(surface)/2.0,DIST_AXIS2FIGURE_X-cairo_image_surface_get_height(surface)-DIST_YLABEL2AXIS_X);
	cairo_fill(cr);
	cairo_rotate(cr,M_PI/2.0);
#ifdef PLOTEXTENTS
	/* plot xlabel extents */
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_move_to(cr, DIST_AXIS2FIGURE_X-DIST_YLABEL2AXIS_X,height/2.0-cairo_image_surface_get_width(surface)/2.0);
	cairo_rel_line_to (cr,0,cairo_image_surface_get_width(surface));
	cairo_rel_line_to (cr,-cairo_image_surface_get_height(surface),0);
	cairo_rel_line_to (cr,0,-cairo_image_surface_get_width(surface));
	cairo_rel_line_to (cr,cairo_image_surface_get_height(surface),0);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif	
	cairo_surface_destroy(surface);

	cairo_destroy(cr);

	return 0;

#else /* no LaTeX code for label */

	cairo_text_extents_t extents;

	/* plot background */
	cairo_set_source_rgba(cr,0,0,0,0.0); /* translucent, do not copy */
	cairo_paint(cr);

	/* set font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, FONTSIZE);
	cairo_set_source_rgba(cr,0,0,0,1.0); /* opaque, do copy */
	
	/* plot xlabel */
	cairo_text_extents(cr, graph.xlabel, &extents);
	cairo_move_to(cr, 0.5*width-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+DIST_XLABEL2AXIS_Y+extents.height);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, graph.xlabel);
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
	cairo_text_extents(cr, graph.ylabel, &extents);
	cairo_move_to (cr, DIST_AXIS2FIGURE_X-DIST_YLABEL2AXIS_X,0.5*height+0.5*extents.width);
	cairo_rotate(cr,-M_PI/2.0);
	cairo_show_text (cr, graph.ylabel);
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
	cairo_text_extents(cr, graph.title, &extents);
	cairo_move_to (cr, 0.5*width-0.5*(extents.width),DIST_AXIS2FIGURE_Y-DIST_TITLE2AXIS_Y);
	cairo_rotate(cr,0.0);
	cairo_show_text (cr, graph.title);
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

#endif

}

static int create_grid_surface_log(GtkWidget *widget) {
	
	cairo_t *cr;
	gint kmin, kmax, k, i, width, height;
	gchar tick[100];
	gchar str[100];
	cairo_text_extents_t extents;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the grid surface, which will be used lateron in the on_expose_event callback function */
	if(grid_surface) {
		cairo_surface_destroy(grid_surface);
		grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(grid_surface);

	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);

	/* set line color to gray */
	cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0); /* opaque, do copy */
	/* set line type to dashed */
	gdouble dashes[2] = {5,5};	
	cairo_set_dash (cr, dashes, 2, 0);

	/* minimum and maximum exponent */
	if(graph.xrange[0]>0) {
		kmin = floor(log10(graph.xrange[0]));
	} else {
	    kmin = 0;
	}
	
	if(graph.xrange[1]>0) {
		kmax = ceil(log10(graph.xrange[1]));
	} else {
		kmax = 1;
	}
	
	graph.xrange[0] = pow(10,kmin);
	graph.xrange[1] = pow(10,kmax);

	/* plot vertical grid lines */
	for(k=kmin;k<kmax;k++) {
		for(i=1;i<=9;i++) {
			if ( !(k==kmin && i==1) ) {
				cairo_move_to(cr,DIST_AXIS2FIGURE_X+(width-2*DIST_AXIS2FIGURE_X)/(kmax-kmin)*(log10(i*pow(10,k))-kmin),height-DIST_AXIS2FIGURE_Y);
				cairo_line_to(cr,DIST_AXIS2FIGURE_X+(width-2*DIST_AXIS2FIGURE_X)/(kmax-kmin)*(log10(i*pow(10,k))-kmin),DIST_AXIS2FIGURE_Y);
				cairo_stroke(cr);
			}
		}
	}

	/* set font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, FONTSIZETICKS);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0); /* opaque, do copy */

	/* plot x ticks */
	for(k=kmin;k<=kmax;k++) {
		cairo_set_font_size (cr, FONTSIZETICKS);
		cairo_text_extents(cr,"10", &extents);
		cairo_move_to(cr,DIST_AXIS2FIGURE_X+(width-2*DIST_AXIS2FIGURE_X)/(kmax-kmin)*(log10(pow(10,k))-kmin)-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+extents.height+DIST_XTICKS2AXIS_Y);
		cairo_show_text (cr, "10");

		sprintf(str,"%d",k);
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
		cairo_move_to(cr,DIST_AXIS2FIGURE_X+(width-2*DIST_AXIS2FIGURE_X)/(kmax-kmin)*(log10(pow(10,k))-kmin)+0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+DIST_XTICKS2AXIS_Y);
		cairo_show_text (cr, str);
	}

	/* destroy ticks (if needed) to prevent memory leakage */
	if(graph.yticks) {
		destroy_ticks(graph.yticks);
	}

	/* create ticks */
	graph.yticks = create_ticks(graph.yrange);

	/* determine significance for rounding tick labels */
	sprintf(str,"%%.%df",graph.yticks->tick_step_significance);

	/* plot y ticks */
	for(i=0;i<graph.yticks->tick_n;i++) {
		sprintf(tick,str,graph.yticks->tick_labels[graph.yticks->tick_n-1-i]/pow(10,graph.yticks->tick_exponent));
		cairo_text_extents(cr,tick, &extents);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i+(extents.height)/2.0);
		cairo_show_text (cr, tick);
		cairo_stroke(cr);
#ifdef PLOTEXTENTS
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i+(extents.height)/2.0);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
#endif
	}

/* y exponent */
	if(graph.yticks->tick_exponent) {
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
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
		sprintf(tick,"%d",graph.yticks->tick_exponent);
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
	
	/* set line color to gray */
	cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0); /* opaque, do copy */
	/* set line type to dashed */
//	gdouble dashes[2] = {5,5};	
	//cairo_set_dash (cr, dashes, 2, 0);

	/* plot horizontal grid lines */
	for ( i = 1; i < graph.yticks->tick_n-1; i++ ) {
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,(gdouble)DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i);
		cairo_line_to (cr, width-DIST_AXIS2FIGURE_X,(gdouble)DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i);
	}
	cairo_stroke(cr);

	/* destroy cairo object */
	cairo_destroy(cr);

	return 0;
}

static int create_grid_surface_linear(GtkWidget *widget) {

	gint i;
	cairo_t *cr;
	gint width, height;
	gchar tick[100];
	gchar str[100];
	cairo_text_extents_t extents;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the grid surface, which will be used lateron in the on_expose_event callback function */
	if(grid_surface) {
		cairo_surface_destroy(grid_surface);
		grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		grid_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(grid_surface);

	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);	

	/* destroy ticks (if needed) to prevent memory leakage */
	if(graph.xticks) {
		destroy_ticks(graph.xticks);
	}
	if(graph.yticks) {
		destroy_ticks(graph.yticks);
	}

	/* create ticks */
	graph.xticks = create_ticks(graph.xrange);
	graph.yticks = create_ticks(graph.yrange);

	/* set font */
	cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, FONTSIZETICKS);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0); /* opaque, do copy */

	/* determine significance for rounding tick labels */
	sprintf(str,"%%.%df",graph.xticks->tick_step_significance);
	
	/* plot x ticks */
	for(i=0;i<graph.xticks->tick_n;i++)	{		
		sprintf(tick,str,graph.xticks->tick_labels[i]/pow(10,graph.xticks->tick_exponent));
		cairo_text_extents(cr,tick, &extents);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(graph.xticks->tick_n-1)*i-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+extents.height+DIST_XTICKS2AXIS_Y);
		cairo_show_text (cr, tick);
#ifdef PLOTEXTENTS
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(graph.xticks->tick_n-1)*i-0.5*(extents.width),height-DIST_AXIS2FIGURE_Y+extents.height+DIST_XTICKS2AXIS_Y);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
#endif
	}

	/* determine significance for rounding tick labels */
	sprintf(str,"%%.%df",graph.yticks->tick_step_significance);

	/* plot y ticks */
	for(i=0;i<graph.yticks->tick_n;i++) {
		sprintf(tick,str,graph.yticks->tick_labels[graph.yticks->tick_n-1-i]/pow(10,graph.yticks->tick_exponent));
		cairo_text_extents(cr,tick, &extents);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i+(extents.height)/2.0);
		cairo_show_text (cr, tick);
		cairo_stroke(cr);
#ifdef PLOTEXTENTS
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		cairo_move_to (cr, DIST_AXIS2FIGURE_X-extents.width-DIST_YTICKS2AXIS_X,DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i+(extents.height)/2.0);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
#endif
	}

	/* plot x exponent */
	if(graph.xticks->tick_exponent) {
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
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
		sprintf(tick,"%d",graph.xticks->tick_exponent);
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
	}

	/* y exponent */
	if(graph.yticks->tick_exponent) {
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
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
		sprintf(tick,"%d",graph.yticks->tick_exponent);
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
	
	/* set line color to gray */
	cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0); /* opaque, do copy */
	/* set line type to dashed */
	gdouble dashes[2] = {5,5};	
	cairo_set_dash (cr, dashes, 2, 0);

	/* plot vertical grid lines */
	for ( i = 1; i < graph.xticks->tick_n-1; i++ ) {
		cairo_move_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(graph.xticks->tick_n-1)*i,height-DIST_AXIS2FIGURE_Y);
		cairo_line_to (cr, DIST_AXIS2FIGURE_X+(gdouble)(width-2*DIST_AXIS2FIGURE_X)/(graph.xticks->tick_n-1)*i,DIST_AXIS2FIGURE_Y);
	}
	/* plot horizontal grid lines */
	for ( i = 1; i < graph.yticks->tick_n-1; i++ ) {
		cairo_move_to (cr, DIST_AXIS2FIGURE_X,(gdouble)DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i);
		cairo_line_to (cr, width-DIST_AXIS2FIGURE_X,(gdouble)DIST_AXIS2FIGURE_Y+(gdouble)(height-2*DIST_AXIS2FIGURE_Y)/(graph.yticks->tick_n-1)*i);
	}
	cairo_stroke(cr);

	cairo_destroy(cr);
	
	return 0;
}

static int create_grid_surface(GtkWidget *widget) {

	if(graph.xtype==LINEAR && graph.ytype==LINEAR) {
		return create_grid_surface_linear(widget);
	} else if (graph.xtype==LOG && graph.ytype==LINEAR) {
		return create_grid_surface_log(widget);
	} else if (graph.xtype==LINEAR && graph.ytype==LOG) {
		return create_grid_surface_log(widget);
	} else if (graph.xtype==LOG && graph.ytype==LOG) {
		return create_grid_surface_log(widget);
	}

	return 0;
}

static int create_data_surface_log(GtkWidget *widget) {

	gint i;
	cairo_t *cr;
	gint width, height;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the data surface, which will be used lateron in the on_expose_event callback function */
	if(data_surface) {
		cairo_surface_destroy(data_surface);
		data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(data_surface);

	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);

	/* scaling factor from data to pixels x */
	gdouble xdata2pix = (width-2.0*DIST_AXIS2FIGURE_X)/(log10(graph.xrange[1])-log10(graph.xrange[0]));

	/* scaling factor from data to pixels y */
	gdouble ydata2pix = (height-2.0*DIST_AXIS2FIGURE_Y)/(graph.yrange[1]-graph.yrange[0]);

	/* plot data */
	cairo_set_line_width(cr, 2.0);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);

	cairo_move_to(cr, DIST_AXIS2FIGURE_X+(log10(graph.xdata[0])-log10(graph.xrange[0]))*xdata2pix, height-DIST_AXIS2FIGURE_Y-(graph.ydata[0]-graph.yrange[0])*ydata2pix);
	for(i=1;i<graph.ndata;i++) {
		if(graph.xdata[i]>=graph.xrange[0] && graph.xdata[i]<=graph.xrange[1] && graph.ydata[i]>=graph.yrange[0] && graph.ydata[i]<=graph.yrange[1]) {
			cairo_line_to(cr, DIST_AXIS2FIGURE_X+(log10(graph.xdata[i])-log10(graph.xrange[0]))*xdata2pix, height-DIST_AXIS2FIGURE_Y-(graph.ydata[i]-graph.yrange[0])*ydata2pix);
		} else {
			cairo_stroke(cr);
		}
	}
	cairo_stroke(cr);

	cairo_destroy(cr);

	return 0;
}

static int create_data_surface_linear(GtkWidget *widget) {

	gint i;
	cairo_t *cr;
	gint width, height;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the data surface, which will be used lateron in the on_expose_event callback function */
	if(data_surface) {
		cairo_surface_destroy(data_surface);
		data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		data_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(data_surface);

	/* plot background */
	cairo_set_source_rgba(cr, 0, 0, 0, 0.0); /* translucent, do not copy */
	cairo_paint(cr);

	/* scaling factor from data to pixels x */
	gdouble xdata2pix = (width-2.0*DIST_AXIS2FIGURE_X)/(graph.xrange[1]-graph.xrange[0]);

	/* scaling factor from data to pixels y */
	gdouble ydata2pix = (height-2.0*DIST_AXIS2FIGURE_Y)/(graph.yrange[1]-graph.yrange[0]);

	/* plot data */
	cairo_set_line_width(cr, 2.0);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);

	cairo_move_to(cr, DIST_AXIS2FIGURE_X+(graph.xdata[0]-graph.xrange[0])*xdata2pix, height-DIST_AXIS2FIGURE_Y-(graph.ydata[0]-graph.yrange[0])*ydata2pix);
	for(i=1;i<graph.ndata;i++) {
		if(graph.xdata[i]>=graph.xrange[0] && graph.xdata[i]<=graph.xrange[1] && graph.ydata[i]>=graph.yrange[0] && graph.ydata[i]<=graph.yrange[1]) {
			cairo_line_to(cr, DIST_AXIS2FIGURE_X+(graph.xdata[i]-graph.xrange[0])*xdata2pix, height-DIST_AXIS2FIGURE_Y-(graph.ydata[i]-graph.yrange[0])*ydata2pix);
		} else {
			cairo_stroke(cr);
		}
	}
	cairo_stroke(cr);

	cairo_destroy(cr);

	return 0;
}

static int create_data_surface(GtkWidget *widget) {
	
	if(graph.xtype==LINEAR && graph.ytype==LINEAR) {
		return create_data_surface_linear(widget);
	} else if (graph.xtype==LOG && graph.ytype==LINEAR) {
		return create_data_surface_log(widget);
	} else if (graph.xtype==LINEAR && graph.ytype==LOG) {
		return create_data_surface_log(widget);
	} else if (graph.xtype==LOG && graph.ytype==LOG) {
		return create_data_surface_log(widget);
	}

	return 0;
}

static int create_total_surface(GtkWidget *widget) {

	cairo_t *cr;
	gint width, height;

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* create the total surface, which will be used lateron in the on_expose_event callback function */
	if(total_surface) {
		cairo_surface_destroy(total_surface);
		total_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	} else {
		total_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
	}
	cr = cairo_create(total_surface);
	
	/* set source surface to created axis surface or a one color surface (see on_realize callback function) */
	cairo_set_source_surface (cr, axis_surface, 0, 0); /* source = axis surface */
//	cairo_set_source_rgb(cr, 0, 0, 0); /* source = one color surface */
	/* create a mask for this surface (equal to the axis surface itself here) */
	cairo_mask_surface(cr,axis_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created label surface or a one color surface (see on_realize callback function) */
	cairo_set_source_surface (cr, label_surface, 0, 0); /* source = label surface */
//	cairo_set_source_rgb(cr, 0, 0, 0); /* source = one color surface */
	/* create a mask for this surface (equal to the label surface itself here) */
	cairo_mask_surface(cr,label_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created grid surface or a one color surface (see on_realize callback function) */
    cairo_set_source_surface (cr, grid_surface, 0, 0); /* source = grid surface */
//	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); /* source = one color surface */
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,grid_surface, 0, 0);
	cairo_fill(cr);

	/* set source surface to created data surface or a one color surface (see on_realize callback function) */
//	cairo_set_source_surface (cr, data_surface, 0, 0); /* source = data surface */
	cairo_set_source_rgb(cr, 0, 0, 1); /* source = one color surface */
	/* create a mask for this surface (equal to the surface itself here) */
	cairo_mask_surface(cr,data_surface, 0, 0);
	cairo_fill(cr);

	cairo_destroy(cr);

	return 0;
}

static int draw_zoom(cairo_t *cr, gint width, gint height) {

	/* draw the zoom rectangele */
	if(graph.tool_state == ZOOM_BUSY) {
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_rectangle(cr,(graph.xrangezoom[0]-graph.xrange[0])/(graph.xrange[1]-graph.xrange[0])*(width-2*DIST_AXIS2FIGURE_X)+DIST_AXIS2FIGURE_X,-(graph.yrangezoom[0]-graph.yrange[0])/(graph.yrange[1]-graph.yrange[0])*(height-2*DIST_AXIS2FIGURE_Y)+height-DIST_AXIS2FIGURE_Y,graph.mouse[0]-((graph.xrangezoom[0]-graph.xrange[0])/(graph.xrange[1]-graph.xrange[0])*(width-2*DIST_AXIS2FIGURE_X)+DIST_AXIS2FIGURE_X),graph.mouse[1]-(-(graph.yrangezoom[0]-graph.yrange[0])/(graph.yrange[1]-graph.yrange[0])*(height-2*DIST_AXIS2FIGURE_Y)+height-DIST_AXIS2FIGURE_Y));
		cairo_stroke(cr);
	}

	return 0;
}

static int draw_point(cairo_t *cr, gint width, gint height) {

	gchar tick[100];
	
	/* only draw pointer when mouse is in axis area */
	if(graph.mouse[0]>DIST_AXIS2FIGURE_X && graph.mouse[0]<width-DIST_AXIS2FIGURE_X && graph.mouse[1]>DIST_AXIS2FIGURE_Y && graph.mouse[1]<height-DIST_AXIS2FIGURE_Y) {

		/* scaling factor from data to pixels and vice versa */
		//gdouble pix2xdata = (graph.xrange[1]-graph.xrange[0])/(width-2.0*DIST_AXIS2FIGURE_X);
		//gdouble xdata2pix = 1.0/pix2xdata;

		/* scaling factor from data to pixels and vice versa */
		gdouble pix2ydata = (graph.yrange[1]-graph.yrange[0])/(height-2.0*DIST_AXIS2FIGURE_Y);
		gdouble ydata2pix = 1.0/pix2ydata;

		/* plot vertical line */
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_move_to(cr,graph.mouse[0],DIST_AXIS2FIGURE_Y+1);
		cairo_line_to(cr,graph.mouse[0],height-DIST_AXIS2FIGURE_Y);
		cairo_stroke(cr);

		/* calculate index of data at mouse position (now via bi-section algorithm but maybe this can be made faster in some way) */
		gint i;
		gint MIN = 0;
		gint MAX = graph.ndata;
		gint elem = graph.ndata/2;

		if(graph.xtype==LINEAR) {
			gdouble x = (graph.mouse[0]-DIST_AXIS2FIGURE_X)*1.0/(width-2*DIST_AXIS2FIGURE_X)*(graph.xrange[1]-graph.xrange[0])+graph.xrange[0];

			for (i=0;i<(gint) ceil(log(graph.ndata)/log(2));i++) {
				if(graph.xdata[elem]<x) {
					MIN = elem;
					elem = (elem+MAX)/2;
				} else if (graph.xdata[elem]>x) {
					MAX = elem;
					elem = (elem+MIN)/2;
				}
			}
		} else if (graph.xtype==LOG) {
			gdouble x = pow(10,(((graph.mouse[0]-DIST_AXIS2FIGURE_X)*1.0/(width-2*DIST_AXIS2FIGURE_X))*((ceil(log10(graph.xrange[1])))-(floor(log10(graph.xrange[0]))))+(floor(log10(graph.xrange[0])))));

			for (i=0;i<(gint) ceil(log(graph.ndata)/log(2));i++) {
				if(graph.xdata[elem]<x) {
					MIN = elem;
					elem = (elem+MAX)/2;
				} else if (graph.xdata[elem]>x) {
					MAX = elem;
					elem = (elem+MIN)/2;
				}
			}
		}

		if ((graph.ydata[elem]-graph.yrange[0])*ydata2pix<(height-2*DIST_AXIS2FIGURE_Y) && (graph.ydata[elem]-graph.yrange[0])*ydata2pix>0) {
			/* plot horizontal line */
			cairo_move_to(cr,DIST_AXIS2FIGURE_X+1,height-DIST_AXIS2FIGURE_Y-(graph.ydata[elem]-graph.yrange[0])*ydata2pix);
			cairo_line_to(cr,width-DIST_AXIS2FIGURE_X,height-DIST_AXIS2FIGURE_Y-(graph.ydata[elem]-graph.yrange[0])*ydata2pix);
			cairo_stroke(cr);

			/* plot small box around data point of interest */
			cairo_move_to(cr, graph.mouse[0]-4, height-DIST_AXIS2FIGURE_Y-(graph.ydata[elem]-graph.yrange[0])*ydata2pix-4);
			cairo_rel_line_to (cr,8,0);
			cairo_rel_line_to (cr,0,8);
			cairo_rel_line_to (cr,-8,0);
			cairo_rel_line_to (cr,0,-8);
			cairo_stroke(cr);
		}

		/* plot current position text */
		cairo_set_font_size (cr, FONTSIZETICKSEXPONENT);
		sprintf(tick,"(x,y) = (%.3e,%.3e)",graph.xdata[elem],graph.ydata[elem]);
		cairo_move_to(cr, 5, height-5);
		cairo_show_text (cr, tick);

#ifdef PLOTEXTENTS
		/* plot exponent extents */
		cairo_text_extents_t extents;
		cairo_text_extents(cr,tick, &extents);
		cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		cairo_move_to (cr, 5,height-4);
		cairo_rel_line_to (cr,extents.width,0);
		cairo_rel_line_to (cr,0,-extents.height);
		cairo_rel_line_to (cr,-extents.width,0);
		cairo_rel_line_to (cr,0,extents.height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
#endif
	}

	return 0;
}

static void on_expose_event(GtkWidget *widget, GdkEvent *event, gpointer data) {

#ifdef DEBUG
	printf("*** expose event triggered ***\n");
#endif
	cairo_t *cr;
	gint width, height;

	/* start timer */
	start_timer();

	/* create cairo drawing */
	cr = gdk_cairo_create (widget->window);

	/* determine size of drawing area */
	width = gdk_window_get_width(gtk_widget_get_window(widget)); 
	height = gdk_window_get_height(gtk_widget_get_window(widget));

	/* set source surface to created total surface (see on_realize callback function) */
	cairo_set_source_surface (cr, total_surface, 0, 0);
	cairo_paint(cr);

	switch(graph.tool) {
		case ZOOM:
			draw_zoom(cr,width,height);
			break;
		case POINT:
			draw_point(cr,width,height);
			break;
		default:
			break;
	}

	cairo_destroy(cr);

	/* stop timer */
	stop_timer();

	return;
}

static void on_realize(GtkWidget *widget, GdkEvent *event, gpointer data) {

#ifdef DEBUG
	printf("*** realize event triggered ***\n");
#endif

//	gint i;
//	create_axis_surface(widget);
//	create_label_surface(widget);

	/* determine x range */
/*	graph.xrange[0] = INFINITY;
	graph.xrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.xdata[i] < graph.xrange[0]) {
			graph.xrange[0] = graph.xdata[i];
		}
		if (graph.xdata[i] > graph.xrange[1]) {
			graph.xrange[1] = graph.xdata[i];
		}
	}
*/
	/* determine y range */
/*	graph.yrange[0] = INFINITY;
	graph.yrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.ydata[i] < graph.yrange[0]) {
			graph.yrange[0] = graph.ydata[i];
		}
		if (graph.ydata[i] > graph.yrange[1]) {
			graph.yrange[1] = graph.ydata[i];
		}
	}

	create_grid_surface(widget);
	create_data_surface(widget);	

	create_total_surface(widget);

*/
	return;
}

static void on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data) {

#ifdef DEBUG
	printf("*** motion notify event triggered ***\n");
#endif

	/* store mouse position */
	graph.mouse[0] = event->x;
	graph.mouse[1] = event->y;
	
	/* trigger expose event */	
	gtk_widget_queue_draw(widget);

	return;
}

static void on_configure_event(GtkWidget *widget, GdkEvent *event, gpointer data) {

#ifdef DEBUG
	printf("*** configure event triggered ***\n");
#endif

	/*create_axis_surface(widget);
	create_label_surface(widget);
	create_grid_surface(widget);
	create_data_surface(widget);	

	create_total_surface(widget);*/
	
	return;
}

static void on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {

#ifdef DEBUG
	printf("*** button pressed event triggered ***\n");
#endif

	gint i, width, height;

	/* set tool_state */
	if(graph.tool==ZOOM) {
	
		/* determine size of drawing area */
		width = gdk_window_get_width(gtk_widget_get_window(widget)); 
		height = gdk_window_get_height(gtk_widget_get_window(widget));

		/* get mouse pointer in data units */
		graph.xrangezoom[0] = 1.0*(graph.mouse[0]-DIST_AXIS2FIGURE_X)/(width-2*DIST_AXIS2FIGURE_X)*(graph.xrange[1]-graph.xrange[0])+graph.xrange[0];
		graph.yrangezoom[0] = 1.0*(height-DIST_AXIS2FIGURE_Y-graph.mouse[1])/(height-2*DIST_AXIS2FIGURE_Y)*(graph.yrange[1]-graph.yrange[0])+graph.yrange[0];

		/* set tool state to ZOOM_BUSY */
		graph.tool_state = ZOOM_BUSY;
	}

	/* detect double click */
	if ( event->type==GDK_2BUTTON_PRESS) {

		create_axis_surface(widget);
		create_label_surface(widget);

		/* determine x range */
		graph.xrange[0] = INFINITY;
		graph.xrange[1] = -INFINITY;
		for(i=0;i<graph.ndata;i++) {
			if (graph.xdata[i] < graph.xrange[0]) {
				graph.xrange[0] = graph.xdata[i];
			}
			if (graph.xdata[i] > graph.xrange[1]) {
				graph.xrange[1] = graph.xdata[i];
			}
		}

		/* determine y range */
		graph.yrange[0] = INFINITY;
		graph.yrange[1] = -INFINITY;
		for(i=0;i<graph.ndata;i++) {
			if (graph.ydata[i] < graph.yrange[0]) {
				graph.yrange[0] = graph.ydata[i];
			}
			if (graph.ydata[i] > graph.yrange[1]) {
				graph.yrange[1] = graph.ydata[i];
			}
		}

		create_grid_surface(widget);
		create_data_surface(widget);	

		create_total_surface(widget);

		/* set tool state to ZOOM_READY */
		graph.tool_state = ZOOM_READY;

		/* trigger expose event */	
		gtk_widget_queue_draw(widget);
  	}

	/* detect triple click */
	if(event->type==GDK_3BUTTON_PRESS) {

		cairo_t *cr;
		cairo_surface_t *image;

		gint width, height;

		/* determine size of drawing area */
		width = gdk_window_get_width(gtk_widget_get_window(widget)); 
		height = gdk_window_get_height(gtk_widget_get_window(widget));

		/* create image surface */
		image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
		cr = cairo_create(image);
	
		/* plot white background */
		cairo_set_source_rgba(cr,1,1,1,1.0); /* opague, do copy */
		cairo_paint(cr);

		/* set source surface to created total surface (see on_realize callback function) */
		cairo_set_source_surface (cr, total_surface, 0, 0);
		cairo_paint(cr);

		/* save image */
		cairo_surface_write_to_png(image, "/home/jbest/Desktop/image.png");

		/* destroy cr */
		cairo_destroy(cr);

		/* destroy the image surface */
		cairo_surface_destroy(image);
	}

	return;
}

static void on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {

#ifdef DEBUG
	printf("*** button release event triggered ***\n");
#endif

	gint width, height;

	if(graph.tool==ZOOM && graph.tool_state==ZOOM_BUSY) {
		
		/* determine size of drawing area */
		width = gdk_window_get_width(gtk_widget_get_window(widget)); 
		height = gdk_window_get_height(gtk_widget_get_window(widget));

		/* get mouse pointer in data units */
		graph.xrangezoom[1] = 1.0*(graph.mouse[0]-DIST_AXIS2FIGURE_X)/(width-2*DIST_AXIS2FIGURE_X)*(graph.xrange[1]-graph.xrange[0])+graph.xrange[0];
		graph.yrangezoom[1] = 1.0*(height-DIST_AXIS2FIGURE_Y-graph.mouse[1])/(height-2*DIST_AXIS2FIGURE_Y)*(graph.yrange[1]-graph.yrange[0])+graph.yrange[0];

		/* set tool state to ZOOM_READY */
		graph.tool_state = ZOOM_READY;

		/* new xrange */
		if(graph.xrangezoom[0]<graph.xrangezoom[1]) {
			graph.xrange[0]=graph.xrangezoom[0];
			graph.xrange[1]=graph.xrangezoom[1];
		} else {
			graph.xrange[1]=graph.xrangezoom[0];
			graph.xrange[0]=graph.xrangezoom[1];
		}

		/* new yrange */
		if(graph.yrangezoom[0]<graph.yrangezoom[1]) {
			graph.yrange[0]=graph.yrangezoom[0];
			graph.yrange[1]=graph.yrangezoom[1];
		} else {
			graph.yrange[1]=graph.yrangezoom[0];
			graph.yrange[0]=graph.yrangezoom[1];
		}	
	
		if(graph.xrangezoom[0]==graph.xrangezoom[1] || graph.yrangezoom[0]==graph.yrangezoom[1]) {
			gint i;
			/* determine x range */
			graph.xrange[0] = INFINITY;
			graph.xrange[1] = -INFINITY;
			for(i=0;i<graph.ndata;i++) {
				if (graph.xdata[i] < graph.xrange[0]) {
					graph.xrange[0] = graph.xdata[i];
				}
				if (graph.xdata[i] > graph.xrange[1]) {
					graph.xrange[1] = graph.xdata[i];
				}
			}

			/* determine y range */
			graph.yrange[0] = INFINITY;
			graph.yrange[1] = -INFINITY;
			for(i=0;i<graph.ndata;i++) {
				if (graph.ydata[i] < graph.yrange[0]) {
					graph.yrange[0] = graph.ydata[i];
				}
				if (graph.ydata[i] > graph.yrange[1]) {
					graph.yrange[1] = graph.ydata[i];
				}
			}
		}	

		create_axis_surface(widget);
		create_label_surface(widget);		
		create_grid_surface(widget);
		create_data_surface(widget);	

		create_total_surface(widget);

		/* trigger expose event */	
		gtk_widget_queue_draw(widget);

	//	graph.tool=POINT;
	}

	return;
}

int title(char* title) {

	graph.title = title;
	
	return 0;
}

int xlabel(char* xlabel) {

	graph.xlabel = xlabel;
	
	return 0;
}

int ylabel(char* ylabel) {

    graph.ylabel = ylabel;
	
	return 0;
}

int plot(double* xdata, double* ydata, gint ndata) {
	
	graph.xdata = xdata;
	graph.ydata = ydata;
	graph.ndata = ndata;
	graph.xtype = LINEAR;
	graph.ytype = LINEAR;

	int i;

	/* determine x range */
	graph.xrange[0] = INFINITY;
	graph.xrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.xdata[i] < graph.xrange[0]) {
			graph.xrange[0] = graph.xdata[i];
		}
		if (graph.xdata[i] > graph.xrange[1]) {
			graph.xrange[1] = graph.xdata[i];
		}
	}

	/* determine y range */
	graph.yrange[0] = INFINITY;
	graph.yrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.ydata[i] < graph.yrange[0]) {
			graph.yrange[0] = graph.ydata[i];
		}
		if (graph.ydata[i] > graph.yrange[1]) {
			graph.yrange[1] = graph.ydata[i];
		}
	}

//	create_axis_surface(darea);
//	create_label_surface(darea);		
	create_grid_surface(darea);
	create_data_surface(darea);	

	create_total_surface(darea);

	gtk_widget_queue_draw(darea);

	return 0;
}

int semilogx(double* xdata, double* ydata, gint ndata) {

	graph.xdata = xdata;
	graph.ydata = ydata;
	graph.ndata = ndata;
	graph.xtype = LOG;
	graph.ytype = LINEAR;

	return 0;
}

int semilogy(double* xdata, double* ydata, gint ndata) {

	graph.xdata = xdata;
	graph.ydata = ydata;
	graph.ndata = ndata;
	graph.xtype = LINEAR;
	graph.ytype = LOG;

	return 0;
}

int loglog(double* xdata, double* ydata, gint ndata) {

	graph.xdata = xdata;
	graph.ydata = ydata;
	graph.ndata = ndata;
	graph.xtype = LOG;
	graph.ytype = LOG;

	return 0;
}

void start_zooming(void) {

	gint i;

	/* determine x range */
	graph.xrange[0] = INFINITY;
	graph.xrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.xdata[i] < graph.xrange[0]) {
			graph.xrange[0] = graph.xdata[i];
		}
		if (graph.xdata[i] > graph.xrange[1]) {
			graph.xrange[1] = graph.xdata[i];
		}
	}

	/* determine y range */
	graph.yrange[0] = INFINITY;
	graph.yrange[1] = -INFINITY;
	for(i=0;i<graph.ndata;i++) {
		if (graph.ydata[i] < graph.yrange[0]) {
			graph.yrange[0] = graph.ydata[i];
		}
		if (graph.ydata[i] > graph.yrange[1]) {
			graph.yrange[1] = graph.ydata[i];
		}
	}

	/* set initial zoomranges */
	graph.xrangezoom[0]=graph.xrange[0];
	graph.yrangezoom[0]=graph.yrange[0];
	graph.xrangezoom[1]=graph.xrange[1];
	graph.yrangezoom[1]=graph.yrange[1];

	graph.tool = ZOOM;
	graph.tool_state = ZOOM_READY;
}

void stop_zooming(void) {
	graph.tool = NONE;
}

void start_pointing(void) {
	graph.tool = POINT;
}

void stop_pointing(void) {
	graph.tool = NONE;
}

void* thread(void* dummie)
{
	int i, j, maxchan = 8;
	int nsamp =  256;
	int k = 0;

	double t[N];
	double datay[N];
	double data[N];
	double *pdata = data;
	
	usleep(10000);

	create_axis_surface(darea);
	create_label_surface(darea);		

	for (j=0;j<100;j++){
        pthe_shm->retval = 1;
		while(pthe_shm->retval)
		{

			tde_gate_cmd_begin(CALL_TDE_CHECK_DATA,pthe_shm);
			tde_gate_cmd_wait(CALL_TDE_CHECK_DATA,pthe_shm,NULL,0);
		}
	
		tde_gate_cmd_begin(CALL_TDE_GET_DATA,pthe_shm);
		pthe_shm->argin1 = maxchan*nsamp*2;
		tde_gate_cmd_wait(CALL_TDE_GET_DATA,pthe_shm,pdata,maxchan*nsamp*2);
		
        tde_gate_cmd_begin(CALL_TDE_SAY_READY,pthe_shm);
		tde_gate_cmd_wait(CALL_TDE_SAY_READY,pthe_shm,NULL,0);
        
		k=0;

		for(i=1;i<(int)maxchan*nsamp*2;i+=16) {
			datay[k]=(gdouble) data[i];
			t[k]=(gdouble) k;
			k++;
		}

		gdk_threads_enter();
		plot(t,datay,nsamp);
		gdk_threads_leave();

		usleep(10000);
	}
    
//    tde_gate_cmd_begin(CALL_TDE_STOP_ACQ,pthe_shm);
//    tde_gate_cmd_wait(CALL_TDE_STOP_ACQ,pthe_shm,NULL,0);
	
	return 0;
}


int main (int argc, char *argv[]) {

	gint i;

	gdouble time[N];
	gdouble data[N];

	GError *error = NULL;

	gdouble A[N];
	gdouble P[N];
	gdouble R[N];
	gdouble I[N];
	gdouble F[N];
	gdouble f;

	gdouble m = 1;
	gdouble b = 100;
	gdouble k = 50*50*(4*M_PI*M_PI);

	/* init threads */
	g_thread_init(NULL);
	gdk_threads_init();

	/* init gtk */
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER (window), darea);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	g_signal_connect(darea, "expose-event", G_CALLBACK(on_expose_event), NULL);
	g_signal_connect(darea, "realize", G_CALLBACK(on_realize), NULL);
	g_signal_connect(darea, "motion_notify_event",G_CALLBACK(on_motion_notify_event), NULL);
	g_signal_connect(darea, "configure_event", G_CALLBACK(on_configure_event), NULL);
	g_signal_connect(darea, "button_press_event",G_CALLBACK(on_button_press), NULL);
	g_signal_connect(darea, "button_release_event",G_CALLBACK(on_button_release), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);

	/* turn on pointer motion mask */
	gtk_widget_set_events(darea, GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	/* create dummy time data */
	for(i=0;i<N;i++) {
		time[i]=0+(gdouble)i/N;
		data[i]=2*sin(2*M_PI*2*time[i]);
		data[i]=0*rand();
	}
	
	/* plot dummy time data */
//	plot(time,data,N);
#ifdef LATEXDEMO	
	xlabel("Time $\\frac{d\\alpha}{dt}$ [s]");
	ylabel("Data \\LaTeX [V]");
	title("Title \\LaTeX");
#else
	xlabel("Time [s]");
	ylabel("Data [V]");
	title("Title");
#endif

#ifdef ZOOMDEMO
	start_zooming();
#endif
#ifdef POINTDEMO
	start_pointing();
#endif

	/* create dummy frequency data: P(s) = 1/(m*s^2+b*s) ==> P(2*pi*f*j) = 1/(m*(2*pi*f*j)^2+b*(2*pi*f*j)) */
	for(i=1;i<=N;i++) {
		f = i;
		R[i-1] = -1*m*pow(2*M_PI*f,2)/(m*m*pow(2*M_PI*f,4)+pow(b*2*M_PI*f,2)); /* real part of data */
		I[i-1] = -1*b*2*M_PI*f/(m*m*pow(2*M_PI*f,4)+pow(b*2*M_PI*f,2)); /* imaginary part of data */
		A[i-1] = 20*log10(sqrt(R[i-1]*R[i-1]+I[i-1]*I[i-1])); /* amplitude of data */
		P[i-1] = atan2(I[i-1],R[i-1])/M_PI*180; /* angle of data */
		F[i-1] = f; /* frequency */
	}

	/* create dummy frequency data: P(s) = 1/(m*s^2+b*s+k) ==> P(2*pi*f*j) = 1/(m*(2*pi*f*j)^2+b*(2*pi*f*j)+k) */
	for(i=1;i<=N;i++) {
		f = i;
		R[i-1] = (k-4*M_PI*M_PI*f*f*m)/(4*M_PI*M_PI*b*b*f*f+(k-4*M_PI*M_PI*f*f*m)*(k-4*M_PI*M_PI*f*f*m));
		I[i-1] = -(2*M_PI*b*f)/(4*M_PI*M_PI*b*b*f*f+(k-4*M_PI*M_PI*f*f*m)*(k-4*M_PI*M_PI*f*f*m));
		A[i-1] = 20*log10(sqrt(R[i-1]*R[i-1]+I[i-1]*I[i-1]));
		P[i-1] = atan2(I[i-1],R[i-1])/M_PI*180;
		F[i-1] = f;
	}

#ifdef FRFMAG
	int frfmag = 1;
#else
	int frfmag = 0;
#endif
	if (frfmag) {
		/* plot amplitude of dummy frequency data */
		semilogx(F,A,N);
		xlabel("Frequency [Hz]");
		ylabel("Amplitude [dB]");
#ifdef LATEXDEMO
		title("Frequency response function $H(s)=\\cfrac{1}{s^2+10s+4\\cdot\\pi^2\\cdot 50^2}$");
		title("Frequency response function $\\begin{pmatrix}\\int 1&2\\\\3&4\\end{pmatrix}$");
#else
		title("Frequency response function");
#endif
#ifdef ZOOMDEMO
		start_zooming();
#endif
#ifdef POINTDEMO
		start_pointing();
#endif
	}

#ifdef FRFPHASE
	int frfphase = 1;
#else
	int frfphase = 0;
#endif
	if(frfphase) {
		/* plot phase of dummy frequency data */
		semilogx(F,P,N);
		xlabel("Frequency [Hz]");
		ylabel("Phase [$^o$]");
#ifdef LATEXDEMO
		title("Frequency response function $H(s)=\\frac{1}{s^2+10s+4\\cdot\\pi^2\\cdot 50^2}$");
#else
		title("Frequency response function");
#endif
#ifdef ZOOMDEMO
		start_zooming();
#endif
#ifdef POINTDEMO
		start_pointing();
#endif
	}

	pthe_shm = (ptde_shm) calloc(sizeof(ptde_shm),1);

	/* create periodic thread */
	if (!g_thread_create(thread, NULL, FALSE, &error))
    {
      g_printerr ("Failed to create thread: %s\n", error->message);
      return 1;
    }

	/* show the window */
	gtk_widget_show_all(window);

	/* enter the GTK main loop */	
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return 0;
}

