/*
 * Copyright (C) 2020 ezzzehxx@gmail.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtk/gtk.h>
#include <string.h>
#include "gst-my.h"
#include <gdk/gdkx.h>

static guint64 duration;
static GtkWidget *scale;
static GtkWidget *controls;
static GtkWidget *curLabel;

#define DURATION_IS_VALID(x) (x != 0 && x != (guint64) -1)

struct AppData {
  GtkWidget *video_window;
  GtkWidget *app_window;

  int isRtmp;
  char **argv;
};

static void on_window_main_destroy()
{
    gtk_main_quit();
}

static void
video_widget_realize_cb (GtkWidget * widget, gpointer data)
{
  
  GdkWindow *window;
  window = gtk_widget_get_window (widget);
  // d->window_handle = GDK_WINDOW_XID (window);
  gst_set_window (GDK_WINDOW_XID (window));
}

static void
playing_clicked_cb (GtkButton *button, struct AppData * d)
{
  gst_resume();
}

static void
paused_clicked_cb (GtkButton *button, struct AppData * d)
{
  gst_pause();
}

static void
null_clicked_cb (GtkButton *button, struct AppData * d)
{
  gst_stop();
  on_window_main_destroy();
}

static void
seek_cb (GtkRange *range,
         GtkScrollType scroll,
         gdouble value,
         gpointer data)
{
    guint64 to_seek;

    if (!DURATION_IS_VALID (duration))
        duration = gst_query_duration ();

    if (!DURATION_IS_VALID (duration))
        return;

    to_seek = (value / 100) * duration;

#if 0
    g_print ("value: %f\n", value);
    g_print ("duration: %llu\n", duration);
    g_print ("seek: %llu\n", to_seek);
#endif
    gst_seek_absolute (to_seek);
}

static gboolean
timeout (gpointer data)
{
    guint64 pos;

    pos = gst_query_position ();
    if (!DURATION_IS_VALID (duration))
        duration = gst_query_duration ();

    if (!DURATION_IS_VALID (duration))
        return TRUE;

#if 0
    g_debug ("duration=%f", duration / ((double) 60 * 1000 * 1000 * 1000));
    g_debug ("position=%llu", pos);
#endif

    /** @todo use events for seeking instead of checking for bad positions. */
    if (pos != 0)
    {
        double value;
        gint64 scend = duration / 1000000000;
        int h = scend / 3600;
        int m = (scend / 60) % 60;
        int s = scend  % 60;


        gint64 scend_s = pos / 1000000000;
        int hs = scend_s / 3600;
        int ms = (scend_s / 60) % 60;
        int ss = scend_s  % 60;

        value = (pos * (((double) 100) / duration));
        gtk_range_set_value (GTK_RANGE (scale), value);
        gtk_label_set_text((GtkLabel *) curLabel, g_strdup_printf("%u:%02u:%02u / %u:%02u:%02u", hs, ms, ss, h, m, s));
    }

    return TRUE;
}

static gboolean
key_press (GtkWidget *widget,
           GdkEventKey *event,
           gpointer data)
{
    switch (event->keyval)
    {
        default:
            break;
    }

    return TRUE;
}

static gboolean hideCursor(gpointer data)
{
     struct AppData* d = data;

     GdkCursor* blank_c = gdk_cursor_new(GDK_BLANK_CURSOR);

     GdkWindow* win = gtk_widget_get_window(d->app_window);
     gdk_window_set_cursor(win, blank_c);

     return FALSE;
}

static void
build_window (struct AppData * d)
{
  GtkBuilder *builder;
  GtkWidget *button;
  gchar *window_ui;
  GError *error = NULL;

  builder = gtk_builder_new ();
  window_ui = my_file ("window.ui");
  if (!gtk_builder_add_from_file (builder, window_ui, &error)) {
    g_error ("Failed to load window.ui: %s", error->message);
    g_error_free (error);
    goto exit;
  }

  d->app_window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  g_object_ref (d->app_window);

  g_signal_connect (d->app_window, "destroy",
      G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect (G_OBJECT (d->app_window), "key-press-event",
                      G_CALLBACK (key_press), NULL);

  g_object_set (d->app_window, "title",
      "Cdesk video redirect",
      NULL);

  d->video_window = GTK_WIDGET (gtk_builder_get_object (builder, "videoarea"));

  g_signal_connect (d->video_window, "realize",
      G_CALLBACK (video_widget_realize_cb), d);

  gtk_widget_add_events(d->app_window, GDK_KEY_PRESS_MASK);

  gtk_widget_show_all(d->app_window);

  GtkWidget *boxgroup = GTK_WIDGET (gtk_builder_get_object (builder, "vbox"));

  if (d->isRtmp == 0) {  
    button = GTK_WIDGET (gtk_builder_get_object (builder, "button_playing"));
    g_signal_connect (button, "clicked", G_CALLBACK (playing_clicked_cb), d);

    button = GTK_WIDGET (gtk_builder_get_object (builder, "button_paused"));
    g_signal_connect (button, "clicked", G_CALLBACK (paused_clicked_cb), d);

    button = GTK_WIDGET (gtk_builder_get_object (builder, "button_null"));
    g_signal_connect (button, "clicked", G_CALLBACK (null_clicked_cb), d);

    controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    gtk_box_pack_end (GTK_BOX (boxgroup), controls, TRUE, TRUE, 2);

    {
          GtkAdjustment * adj;
          adj = gtk_adjustment_new (0, 0, 101, 1, 5, 1);

          scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (adj));
          gtk_box_pack_start (GTK_BOX (controls), scale, TRUE, TRUE, 2);
          g_signal_connect (G_OBJECT (scale), "change-value",
                            G_CALLBACK (seek_cb), d);

          curLabel = gtk_label_new ("0.00 / 0.00");
          gtk_box_pack_start (GTK_BOX (controls), curLabel, FALSE, FALSE, 2);
    }
    gtk_widget_show(boxgroup);
  } else {
    g_idle_add(hideCursor, d);
    gtk_widget_hide(boxgroup);
  }

exit:
  g_free (window_ui);
  g_object_unref (builder);
}

int
main (int argc, char **argv)
{
 
  gdk_set_allowed_backends ("x11");

  gtk_init (&argc, &argv);
  gst_my_init(&argc, &argv);

  struct AppData data = {0};
  data.isRtmp = argc > 2 ? 1 : 0;
  if (argc == 2) {
    if(strstr(argv[1], "rtmp://") != NULL){
      data.isRtmp = 1;
    }
  } 
  // g_print("rtmp %d \n", data.isRtmp);
  data.argv = argv;

  // create the window
  build_window (&data);

  // show the GUI
  gtk_window_fullscreen (GTK_WINDOW(data.app_window));
  // gtk_widget_realize (data.app_window);
  if (argc > 1 && argc <= 2) {
      my_play_start(argv[1], data.isRtmp);
  }
  g_timeout_add_seconds (1, timeout, &data);
  gtk_main ();

  gst_stop();
  return 0;
}
