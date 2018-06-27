/*
 * Escope v1.0
 * ===========
 *
 * compile with:
 * -  gcc -g -o Escope 'Escope v1.0.c' gtk_graph.c -export-dynamic `pkg-config --cflags --libs gtk+-2.0` -lm
 * run with:
 * -  ./'Escope v1.0'
 *
 * to change version number:
 * - open 'Escope v1.0.glade' and store it as 'Escope v2.0.glade'
 * - change the VERSION define from "v1.0" to "v2.0"
 * - compile with: gcc -g -o Escope 'Escope v2.0.c' gtk-graph.c -export-dynamic `pkg-config --cflags --libs gtk+-2.0`
 * - run with: ./'Escope v2.0'
 *
 * to show stock-items on button:
 * - start gconf-editor from a shell, so
		$ user@user-desktop ~ $ gconf-editor
 * - in the gconf-editor go to /desktop/gnome/interface
 * - enable buttons_have_icons
 *
 * 
 * Author:	Jeroen de Best
 *			j.j.t.h.d.best@tue.nl
 * Date: 	19-10-2011
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#include <gtk/gtk.h>
#include "../../engine/engine.h"
#include "../../incomm/incomm.h"
#include "gtk_graph.h"

#include "tde.c"

#define dabs(A) ((A)<0?-(A):(A))
#define VERSION "v1.0"
#define SCOPEFREQ 1

//#define NOEBOX

#define DEC (void *(*)(void*))

static ptde_shm pthe_shm;
static GtkBuilder *builder;
static GtkWidget *graph1, *graph2;

void* thread(void* dummie)
{
	int i, j, maxchan = 8;
	int nsamp =  256;
	double *data;
	double temp;
	temp = maxchan*nsamp*2;
	double *t;
	double *datay;
	int k = 0;

	printf("Bijna\n");
	usleep(2000000);
	printf("Gaan!\n");

	data = (double *)malloc(sizeof(double)*maxchan*nsamp*2);
	t = (gdouble *)malloc(sizeof(gdouble)*nsamp);
	datay = (gdouble *)malloc(sizeof(gdouble)*nsamp);
    
    
    tde_gate_cmd_begin(CALL_FG_SET_DEFAULTS,pthe_shm);
    tde_gate_cmd_wait(CALL_FG_SET_DEFAULTS,pthe_shm,NULL,0);
    
    tde_gate_cmd_begin(CALL_FG_SET_ONOFF,pthe_shm);
    (pthe_shm->argin1) = 1;
    tde_gate_cmd_wait(CALL_FG_SET_ONOFF,pthe_shm,NULL,0);

    tde_gate_cmd_begin(CALL_FG_START,pthe_shm);
    pthe_shm->argin1 = 0;
    tde_gate_cmd_wait(CALL_FG_START,pthe_shm,NULL,0);

	for (j=0;j<100;j++){
        pthe_shm->retval = 1;
		while(pthe_shm->retval)
		{
			tde_gate_cmd_begin(CALL_TDE_CHECK_DATA,pthe_shm);
			tde_gate_cmd_wait(CALL_TDE_CHECK_DATA,pthe_shm,NULL,0);
			usleep(100);
		}
//		printf("Data ready\n");
	
		tde_gate_cmd_begin(CALL_TDE_GET_DATA,pthe_shm);
		pthe_shm->argin1 = maxchan*nsamp*2;
		tde_gate_cmd_wait(CALL_TDE_GET_DATA,pthe_shm,data,maxchan*nsamp*2);
		
        tde_gate_cmd_begin(CALL_TDE_SAY_READY,pthe_shm);
		tde_gate_cmd_wait(CALL_TDE_SAY_READY,pthe_shm,NULL,0);
        
		k=0;

		for(i=0;i<(int)temp;i+=16) {
			datay[k]=(gdouble) data[i];
			t[k]=(gdouble) k;
			k++;
		}

		gtk_graph_plot(graph1,t,datay,nsamp);
		usleep(100);
	}
    
    tde_gate_cmd_begin(CALL_TDE_STOP_ACQ,pthe_shm);
    tde_gate_cmd_wait(CALL_TDE_STOP_ACQ,pthe_shm,NULL,0);
	
    /* Data must be cleaned on exit, otherwise segfault */
	free(data);
	free(t);
	return 0;
}

/*
 * signal handlers for Escope - signal_generator
 */
void on_window_escope_signal_generator_delete_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_window_escope_signal_generator_delete_event\n");
	
	GtkWidget *togglebutton_sg = GTK_WIDGET(gtk_builder_get_object(builder,"togglebutton_sg"));
	gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_sg,FALSE);

	return;
}


/*
 * signal handlers for Escope - main
 */
void on_window_escope_main_delete_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_window_escope_main_delete_event\n");
	
	printf("Quiting Escope %s\n",VERSION);
    gtk_main_quit();
	return;
}

void on_menu_about_activate(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	printf("\t ==> on_menu_about_activate\n");

	GtkWidget *aboutdialog = GTK_WIDGET(gtk_builder_get_object(builder,"aboutdialog"));
	gtk_widget_show(aboutdialog);
	return;
}

void on_togglebutton_sg_toggled(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	printf("\t ==> on_togglebutton_sg_toggled\n");

	GtkWidget *window_escope_signal_generator = GTK_WIDGET(gtk_builder_get_object(builder,"window_escope_signal_generator"));
	if(gtk_toggle_button_get_active((GtkToggleButton *)widget)) {
		gtk_widget_show(window_escope_signal_generator);
	} else {
		gtk_widget_hide(window_escope_signal_generator);
	}
	return;
}

/*
 * signal handlers for Escope - main: signal settings
 */
void combobox_signal_signal_settings_changed_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> combobox_signal_signal_settings_changed_cb\n");
	return;
}

void togglebutton_signal_signal_settings_toggled_cb(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> togglebutton_signal_signal_settings_toggled_cb\n");
	return;
}

void on_button_signal_signalsettings_clicked(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_button_signal_signalsettings_clicked\n");

	GtkWidget *window_escope_channel_information;

	window_escope_channel_information = GTK_WIDGET( gtk_builder_get_object(builder, "window_escope_channel_information"));
	gtk_widget_show(window_escope_channel_information);
	return;
}


void on_entry_offset_signal_settings_activate(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_entry_offset_signal_settings_activate\n");
	return;
}

void on_combobox_range_signal_settings_changed(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_combobox_range_signal_settings_changed\n");
	return;
}

void on_button_offset_signal_settings_clicked(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_button_offset_signal_settings_clicked\n");
	return;
}

/*
 * signal handlers for Escope - main: measurement settings
 */
void on_combobox_sampling_rate_measurement_settings_changed(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_combobox_sampling_rate_measurement_settings_changed\n");
	return;
}

void on_togglebutton_anti_aliasing_measurement_settings_toggled(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_togglebutton_anti_aliasing_measurement_settings_toggled\n");
	return;
}

void on_combobox_frame_length_measurement_settings_changed(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_combobox_frame_length_measurement_settings_changed\n");
	return;
}

void on_togglebutton_hardware_measurement_settings_toggled(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_togglebutton_hardware_measurement_settings_toggled\n");
	return;
}

void on_combobox_number_of_frames_measurement_settings_changed(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_combobox_number_of_frames_measurement_settings_changed\n");
	return;
}


/*
 * signal handlers for Escope - main: scope settings
 */
void on_combobox_scope_scope_settings_changed(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_combobox_scope_scope_settings_changed\n");

	GtkWidget *frame_scope1, *frame_scope2, *togglebutton_scope_scope_settings;
	gint scopenumber;

	frame_scope1 = GTK_WIDGET( gtk_builder_get_object(builder, "frame_scope1"));
	frame_scope2 = GTK_WIDGET( gtk_builder_get_object(builder, "frame_scope2"));
	togglebutton_scope_scope_settings = GTK_WIDGET( gtk_builder_get_object(builder, "togglebutton_scope_scope_settings"));

	scopenumber = gtk_combo_box_get_active((GtkComboBox *)widget);

	if(scopenumber==0) {
		if(gtk_widget_get_visible(frame_scope1)) {
			printf("Scope 1 visible\n");
			gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_scope_scope_settings,1);
		} else {
			printf("Scope 1 invisible\n");
			gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_scope_scope_settings,0);
		}
	} else {
		if(gtk_widget_get_visible(frame_scope2)) {
			printf("Scope 2 visible\n");
			gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_scope_scope_settings,1);
		} else {
			printf("Scope 2 invisible\n");
			gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_scope_scope_settings,0);
		}
	}
	
	return;
}

void on_togglebutton_scope_scope_settings_toggled(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_togglebutton_scope_scope_settings_toggled\n");

	GtkWidget *combobox_scope_scope_settings, *frame_scope1, *frame_scope2, *window_escope_scope;
	gint scopenumber;

	combobox_scope_scope_settings = GTK_WIDGET( gtk_builder_get_object(builder, "combobox_scope_scope_settings"));
	window_escope_scope = GTK_WIDGET( gtk_builder_get_object(builder, "window_escope_scope"));
	frame_scope1 = GTK_WIDGET( gtk_builder_get_object( builder, "frame_scope1" ) );
	frame_scope2 = GTK_WIDGET( gtk_builder_get_object( builder, "frame_scope2" ) );

	scopenumber = gtk_combo_box_get_active((GtkComboBox *)combobox_scope_scope_settings);

	if(gtk_toggle_button_get_active((GtkToggleButton *)widget)){
		if(scopenumber==0) {
			if(!gtk_widget_get_visible(frame_scope1) && !gtk_widget_get_visible(frame_scope2)){
				gtk_widget_show(window_escope_scope);
			}
			gtk_widget_set_visible(frame_scope1,TRUE);
		} else {
			if(!gtk_widget_get_visible(frame_scope1) && !gtk_widget_get_visible(frame_scope2)){
				gtk_widget_show(window_escope_scope);
			}
			gtk_widget_set_visible(frame_scope2,TRUE);
		}
	} else {
		if(scopenumber==0) {
			gtk_widget_set_visible(frame_scope1,FALSE);
			if(!gtk_widget_get_visible(frame_scope1) && !gtk_widget_get_visible(frame_scope2)){
				gtk_widget_hide(window_escope_scope);
			}
			
		} else {
			gtk_widget_set_visible(frame_scope2,FALSE);
			if(!gtk_widget_get_visible(frame_scope1) && !gtk_widget_get_visible(frame_scope2)){
				gtk_widget_hide(window_escope_scope);
			}
		}
	}
	return;
}


void on_scope_button_press_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_scope_button_press_event\n");
	
	/* detect double or triple click */
	if ( event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS ) {
		printf("%s clicked\n",event->type==GDK_2BUTTON_PRESS ? "double" : "triple");
	}
	return;
}

/*
 * signal handlers for Escope - channel information
 */

void on_button_channel_informatin_clicked(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_button_channel_informatin_clicked\n");
	
	GtkWidget *window_escope_channel_information;

	window_escope_channel_information = GTK_WIDGET( gtk_builder_get_object(builder, "window_escope_channel_information"));
	gtk_widget_hide(window_escope_channel_information);
	return;
}

/*
 * signal handlers for Escope - scope
 */

void on_window_escope_scope_delete_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	printf("\t ==> on_window_escope_scope_delete_event\n");
	
	GtkWidget *frame_scope1 = GTK_WIDGET( gtk_builder_get_object(builder, "frame_scope1"));
	GtkWidget *frame_scope2 = GTK_WIDGET( gtk_builder_get_object(builder, "frame_scope2"));
	GtkWidget *window_escope_scope = GTK_WIDGET( gtk_builder_get_object(builder, "window_escope_scope"));
	GtkWidget *togglebutton_scope_scope_settings = GTK_WIDGET( gtk_builder_get_object(builder, "togglebutton_scope_scope_settings"));

	gtk_widget_hide(window_escope_scope);
	gtk_widget_set_visible(frame_scope1,FALSE);
	gtk_widget_set_visible(frame_scope2,FALSE);
	gtk_toggle_button_set_active((GtkToggleButton *)togglebutton_scope_scope_settings,FALSE);

	return;
}

void on_imagemenuitem14_activate(GtkWidget *widget, GdkEvent *event, gpointer data) {
	printf("\t ==> on_imagemenuitem14_activate\n");

	GtkGraph *graph = (GtkGraph *)graph1;
	graph->save_to_image = 1;
	gtk_widget_queue_draw((GtkWidget *)graph1);
	return;
}

void on_togglebutton_zoomin_scope1_toggled(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(gtk_toggle_button_get_active((GtkToggleButton *)widget))
	{
		gtk_graph_start_zooming(graph1);
		gtk_graph_start_zooming(graph2);
	} else
	{
		gtk_graph_stop_zooming(graph1);
		gtk_graph_stop_zooming(graph2);
	}
}

/*
 * timer callback
 */
gboolean func(gpointer data) {

	static gdouble t, t0=0;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	t=tv.tv_sec+tv.tv_usec/1000000.0;
	if (!t0) {
		t0 = t;
	}
	printf("Timer interrupt @ %f\n",t-t0);
	return TRUE;
}


/* 
 * main
 */
int main(int argc, char **argv) {

    GtkWidget  *window_escope_main, *window_escope_signal_generator, *window_escope_scope, *window_escope_channel_information, *aboutdialog;
    GError     *error = NULL;
	char gladefile[100];
	gint i;

    /* init GTK+ */
    gtk_init( &argc, &argv );

	/* create glade filename */
	char gladefile_name[] = "Escope ";
    char gladefile_version[] = VERSION;
    char gladefile_extension[] = ".glade";
    strcpy(gladefile, gladefile_name);
    strcat(gladefile, gladefile_version);
    strcat(gladefile, gladefile_extension);
 
	printf("Starting Escope %s\n",VERSION);

    /* create new GtkBuilder object */
    builder = gtk_builder_new();

    /* load UI from file */
    if( ! gtk_builder_add_from_file( builder, gladefile, &error ) )
    {
        g_warning("%s", error->message);
        g_free(error);
        return(1);
    }
 
    /* get Escope main window pointer from UI */
    window_escope_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_escope_main"));
    /* get Escope signal generator window pointer from UI */
    window_escope_signal_generator = GTK_WIDGET(gtk_builder_get_object(builder, "window_escope_signal_generator"));
    /* get Escope scope window pointer from UI */
    window_escope_scope = GTK_WIDGET(gtk_builder_get_object(builder, "window_escope_scope")); 
    /* get Escope channel information window pointer from UI */
    window_escope_channel_information = GTK_WIDGET(gtk_builder_get_object(builder, "window_escope_channel_information")); 
    /* get Escope about window pointer from UI */
    aboutdialog = GTK_WIDGET(gtk_builder_get_object(builder, "aboutdialog"));
 
	/* pack custom graph widget */
	graph1 = gtk_graph_new();
	graph2 = gtk_graph_new();
	gtk_box_pack_start (GTK_BOX(GTK_WIDGET( gtk_builder_get_object( builder, "hbox2" ) )), graph1, 1, 1, 0);
	gtk_widget_show(graph1);
	gtk_box_pack_start (GTK_BOX(GTK_WIDGET( gtk_builder_get_object( builder, "hbox4" ) )), graph2, 1, 1, 0);
	gtk_widget_show(graph2);
//	g_signal_connect (G_OBJECT (graph1), "motion_notify_event", G_CALLBACK (on_scope_motion_notify_event), NULL);
//	g_signal_connect (G_OBJECT (graph1), "button_press_event", G_CALLBACK (on_scope_button_press_event), NULL);
//	g_signal_connect (G_OBJECT (graph2), "motion_notify_event", G_CALLBACK (on_scope_motion_notify_event), NULL);
//	g_signal_connect (G_OBJECT (graph2), "button_press_event", G_CALLBACK (on_scope_button_press_event), NULL);

    /* connect signals */
    gtk_builder_connect_signals( builder, NULL );
 
	/* generate example data */
	gint ndata = 4000;
	gdouble *xdata = (gdouble *)malloc(ndata*sizeof(gdouble));
	gdouble *ydata = (gdouble *)malloc(ndata*sizeof(gdouble));
	
	for(i=0;i<ndata;i++) {
		xdata[i]=i*0.01;
		ydata[i]=i*0.01+2*sin(2*M_PI*i*0.01);
//		ydata[i]=10.0*rand()/RAND_MAX;
	}

	gtk_graph_plot(GTK_WIDGET(graph1), xdata, ydata, ndata);
	gtk_graph_xlabel(GTK_WIDGET(graph1), "Time [s]");
	gtk_graph_ylabel(GTK_WIDGET(graph1), "Scope 1 [V]");
	gtk_graph_title(GTK_WIDGET(graph1), "Scope 1");

	gtk_graph_plot(GTK_WIDGET(graph2), xdata, ydata, ndata);
	gtk_graph_xlabel(GTK_WIDGET(graph2), "Time [s]");
	gtk_graph_ylabel(GTK_WIDGET(graph2), "Scope 2 [V]");
	gtk_graph_title(GTK_WIDGET(graph2), "Scope 2");

	pthe_shm = (ptde_shm) calloc(sizeof(ptde_shm),1);

gint numADC=3;
gint numDAC=0;
gint numDIN=0;
gint numDOUT=0;
gint numENC=0;
gint numPWM=0;

#ifndef NOEBOX
tde_gate_cmd_begin(CALL_EE_GET_NADC,pthe_shm);
tde_gate_cmd_wait(CALL_EE_GET_NADC,pthe_shm,NULL,0);
numADC = (gint)pthe_shm->retval;

tde_gate_cmd_begin(CALL_EE_GET_NDAC,pthe_shm);
tde_gate_cmd_wait(CALL_EE_GET_NDAC,pthe_shm,NULL,0);
numDAC = (gint)pthe_shm->retval;

tde_gate_cmd_begin(CALL_EE_GET_NDIN,pthe_shm);
tde_gate_cmd_wait(CALL_EE_GET_NDIN,pthe_shm,NULL,0);
numDIN = (gint)pthe_shm->retval;

tde_gate_cmd_begin(CALL_EE_GET_NDOUT,pthe_shm);
tde_gate_cmd_wait(CALL_EE_GET_NDOUT,pthe_shm,NULL,0);
numDOUT = (gint)pthe_shm->retval;
#endif


gchar str[100];
GtkTreeIter iter;
GtkWidget *combobox_signal_signal_settings;
GtkListStore *store;
GtkCellRenderer *cell;
combobox_signal_signal_settings = GTK_WIDGET(gtk_builder_get_object(builder,"combobox_signal_signal_settings"));
store = gtk_list_store_new (1,G_TYPE_STRING);
cell = gtk_cell_renderer_text_new();
/* pack ADCs */
for (i=0;i<numADC;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"ADC %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
/* pack DACs */
for (i=0;i<numDAC;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"DAC %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
/* pack DI */
for (i=0;i<numDIN;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"DI %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
/* pack DO */
for (i=0;i<numDOUT;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"DO %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
/* pack ENC */
for (i=0;i<numENC;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"ENC %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
/* pack PWM */
for (i=0;i<numPWM;i++)
{
gtk_list_store_append(store, &iter);
sprintf(str,"PWM %d",i+1);
gtk_list_store_set(store, &iter, 0, str, -1);
}
gtk_combo_box_set_model (GTK_COMBO_BOX(combobox_signal_signal_settings), GTK_TREE_MODEL(store));
gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox_signal_signal_settings), cell, TRUE );
gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox_signal_signal_settings), cell, "text", 0, NULL );

//GdkColor color;
//gdk_color_parse ("red", &color);
//GtkWidget *togglebutton2 = GTK_WIDGET(gtk_builder_get_object(builder,"togglebutton2"));
//gtk_widget_modify_bg ( GTK_WIDGET(togglebutton2), GTK_STATE_NORMAL, &color);

    /* show Escope main window */
    gtk_widget_show(window_escope_main);

    /* hide Escope scope window */
	GtkWidget *frame_scope1 = GTK_WIDGET(gtk_builder_get_object(builder, "frame_scope1"));
	GtkWidget *frame_scope2 = GTK_WIDGET(gtk_builder_get_object(builder, "frame_scope2"));
	gtk_widget_set_visible(frame_scope1,FALSE);
	gtk_widget_set_visible(frame_scope2,FALSE);

#ifndef NOEBOX
	pthread_t t1;
	pthread_create(&t1,NULL, DEC thread, NULL);
#endif	

//	g_timeout_add(1000/SCOPEFREQ,(GSourceFunc) func, NULL);

    /* start main loop */
    gtk_main();
 
    return( 0 );
}
