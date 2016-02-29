#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdbool.h>
#include <stdatomic.h>

#include "et_error.h"
#include "et_canvas.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_layer_view.h"

void __pvui_app_set_style();

GtkWidget *status_bar= NULL;
static gboolean in_worker_func(gpointer data)
{
	static int count = 0;
	count++;
	// Gtk control.
	printf("work:%d\n", count);
	char s[64];
	snprintf(s, sizeof(s), "%d", count);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), 1, s);

	return G_SOURCE_REMOVE;
}
static atomic_int isStop = ATOMIC_VAR_INIT(0);
static gpointer worker_func(gpointer data)
{
	while( !isStop ){
		g_usleep(500000);
		gdk_threads_add_idle(in_worker_func, NULL);
	}

	return NULL;
}

static gboolean cb_key_press(GtkWidget *widget, GdkEventKey * event, gpointer user_data)
{
	g_print("keyval=%d static=%d string=%s\n",
			event->keyval, event->state, event->string);

	EtKeyAction ka = {
		.key = event->keyval,
		.action = EtKeyAction_Down,
	};
	et_etaion_slot_key_action(ka);

	return FALSE;
}

EtDocId _open_doc_new(GtkWidget *notebook, EtCanvas *canvas_thumbnail,
		EtLayerView *layer_view)
{
	EtCanvas *canvas1 = et_canvas_new();
	GtkWidget *canvas_frame1 = et_canvas_get_widget_frame(canvas1);
	if(NULL == canvas_frame1){
		et_bug("");
		return -1;
	}
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), canvas_frame1, NULL);

	EtDoc *doc1 = et_doc_new();
	if(NULL == doc1){
		et_error("");
		return -1;
	}

	if(!et_doc_manager_add_doc(doc1)){
		et_error("");
		return -1;
	}

	EtRenderer *renderer = et_renderer_new();
	if(NULL == renderer){
		et_error("");
		return -1;
	}

	if(!et_renderer_set_connection(renderer, canvas1, doc1)){
		et_error("");
		return -1;
	}
	if(!et_renderer_set_connection(renderer, canvas_thumbnail, doc1)){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc1, et_layer_view_slot_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	if(!et_layer_view_set_doc_id(layer_view, et_doc_get_id(doc1))){
		et_error("");
		return -1;
	}

	if(0 > et_canvas_set_slot_mouse_action(canvas1,
				et_pointing_manager_slot_mouse_action, NULL)){
		et_error("");
		return -1;
	}

	if(!et_doc_signal_update(doc1)){
		et_error("");
		return -1;
	}

return et_doc_get_id(doc1);
}


	// __pvui_app_set_style();

int main (int argc, char **argv){
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (window, 900,700);

	EtEtaion *current_state = et_etaion_init();
	if(NULL == current_state){
		et_bug("");
		return -1;
	}

	g_signal_connect(G_OBJECT(window), "key-press-event",
			G_CALLBACK(cb_key_press), NULL);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vbox), box1, true, true, 1);

	GtkWidget *statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, TRUE, 0);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), 1, "Hello World");
	status_bar = statusbar;


	GtkWidget *hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(box1), hpaned, true, true, 3);

	GtkWidget *notebook = gtk_notebook_new();
	gtk_paned_pack1 (GTK_PANED (hpaned), notebook, TRUE, FALSE);

	GtkWidget *box_dock_work = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
	gtk_paned_pack2 (GTK_PANED (hpaned), box_dock_work, FALSE, FALSE);

	EtLayerView *layer_view = et_layer_view_init();
	if(NULL == layer_view){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_set_slot_change_state(
				et_layer_view_slot_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_dock_work),
			et_layer_view_get_widget_frame(layer_view),
			true, true, 3);

	GtkWidget *frame_thumb_canvas = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (frame_thumb_canvas), "Thumbnail");
	gtk_frame_set_shadow_type (GTK_FRAME (frame_thumb_canvas), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(box_dock_work), frame_thumb_canvas,
					false, true, 3);

	EtCanvas *canvas_thumbnail = et_canvas_new();
	GtkWidget *canvas_thumbnail_widget = et_canvas_get_widget_frame(canvas_thumbnail);
	if(NULL == canvas_thumbnail_widget){
		et_bug("");
		return -1;
	}
	gtk_container_add(GTK_CONTAINER(frame_thumb_canvas), canvas_thumbnail_widget);

	EtPointingManager *pointing_manager = et_pointing_manager_init();
	if(NULL == pointing_manager){
		et_error("");
		return -1;
	}

	EtDocManager *doc_manager = et_doc_manager_init();
	if(NULL == doc_manager){
		et_error("");
		return -1;
	}
	if(!et_pointing_manager_set_slot_mouse_action(
				et_etaion_slot_mouse_action)){
		et_error("");
		return -1;
	}

	if(0 > et_canvas_set_slot_mouse_action(canvas_thumbnail,
				et_pointing_manager_slot_mouse_action, NULL)){
		et_error("");
		return -1;
	}

	EtDocId doc_id = _open_doc_new(notebook, canvas_thumbnail, layer_view);
	if(0 > doc_id){
		et_error("");
		return -1;
	}

	// ** (開発デバッグ用)ドキュメントにsvgを貼り付ける
	const char *pathImageFile = NULL;
	pathImageFile = "./library/23.svg";
	if(!et_doc_set_image_from_file(et_doc_manager_get_doc_from_id(doc_id), pathImageFile)){
		et_error("");
		return false;
	}

	if(!et_doc_signal_update_from_id(doc_id)){
		et_error("");
		return false;
	}


	GThread* thread;
	thread = g_thread_new ("", worker_func, NULL);
	if(NULL == thread){
		et_critical("");
		return -1;
	}

	gtk_widget_show_all(window);
	gtk_main();

	isStop = 1;
	g_thread_join(thread);

	return 0;
}


void __pvui_app_set_style(){
	GtkCssProvider *provider;
	provider = gtk_css_provider_new ();

	GdkDisplay *display;
	GdkScreen *screen;
	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen (display);
	gtk_style_context_add_provider_for_screen (screen,
			GTK_STYLE_PROVIDER (provider),
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER(provider),
			"GtkWindow {\n"
			"   background-color: rgb (103, 103, 103);\n"
			"}\n"
			//"GtkWidget {\n"
			"#canvasGtk {\n"
			"   background-color: rgb (50, 50, 50);\n"
			"   border-style: solid;\n"
			"   border-width: 1px;\n"
			"   border-color: rgb (255, 255, 10);\n"
			"   margin: 0px;\n"
			"   padding: 0px;\n"
			"}\n"
			"#canvasNanoSVG {\n"
			"   background-color: rgb (50, 50, 50);\n"
			"   border-style: solid;\n"
			"   border-width: 1px;\n"
			"   border-color: rgb (10, 255, 255);\n"
			"   margin: 0px;\n"
			"   padding: 0px;\n"
			"}\n"
			"", -1, NULL);
	g_object_unref (provider);
}
