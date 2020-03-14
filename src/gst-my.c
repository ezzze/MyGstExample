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
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include "fs-element-added-notifier.h"
#include "gst-my.h"

static GstElement *pipeline;
static GstVideoOverlay *overlay;
static guintptr window_handle;
static GstSeekFlags seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT;

#ifndef ARM_BUILD
const static char* play_dir = "/home/ezzze/cwork/my-player/src";
#else
const static char* play_dir = "/root/arm-client/rtmp"
#endif

void
gst_my_init (int *argc,
              char **argv[])
{
    gst_init (argc, argv);
}

guint64
gst_query_duration ()
{
    GstFormat format = GST_FORMAT_TIME;
    gint64 cur;
    gboolean result;
    if (!pipeline){
      return 0;
    }
    result = gst_element_query_duration (pipeline, format, &cur);
    if (!result || format != GST_FORMAT_TIME)
        return GST_CLOCK_TIME_NONE;

    return cur;
}

guint64
gst_query_position ()
{
    GstFormat format = GST_FORMAT_TIME;
    gint64 cur;
    gboolean result;

    if (!pipeline){
      return 0;
    }

    result = gst_element_query_position (pipeline, format, &cur);
    if (!result || format != GST_FORMAT_TIME)
        return GST_CLOCK_TIME_NONE;

    return cur;
}

void
gst_seek_absolute (guint64 value)
{
    if (!pipeline){
      return;
    }
    gst_element_seek (pipeline, 1.0,
                      GST_FORMAT_TIME,
                      seek_flags,
                      GST_SEEK_TYPE_SET, value,
                      GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}


void
gst_set_window (guintptr window_)
{
    window_handle = window_;
}

void
gst_stop (void)
{
    if (pipeline)
    {
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
        pipeline = NULL;
    }
}

void
gst_pause (void)
{
    if(pipeline){
        gst_element_set_state (pipeline, GST_STATE_PAUSED);
    }
    
}

void
gst_resume (void)
{
    if (pipeline) {
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
    }
}

void
gst_reset (void)
{
    gst_element_seek (pipeline, 1.0,
                      GST_FORMAT_TIME,
                      seek_flags,
                      GST_SEEK_TYPE_SET, 0,
                      GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

gchar *
my_file (const gchar * name)
{
  gchar * ret;
  ret = g_build_filename (play_dir, name, NULL);
  if (g_file_test (ret, G_FILE_TEST_EXISTS)) {
      g_print ("Found '%s' at '%s'\n", name, ret);
      return ret;
  }
  g_free (ret);

  return g_strdup (name);
}

static GstBusSyncReply
bus_sync_handler (GstBus * bus, GstMessage * message, gpointer user_data)
{
  // struct AppData *d = user_data;

  if (gst_is_video_overlay_prepare_window_handle_message (message)) {
    if (window_handle != 0) {
      /* GST_MESSAGE_SRC (message) will be the overlay object that we have to
       * use. This may be waylandsink, but it may also be playbin. In the latter
       * case, we must make sure to use playbin instead of waylandsink, because
       * playbin resets the window handle and render_rectangle after restarting
       * playback and the actual window size is lost */
      overlay = GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message));
      gst_video_overlay_set_window_handle (overlay, window_handle);

    } else {
      g_warning ("Should have obtained window_handle by now!\n");
    }

    gst_message_unref (message);
    return GST_BUS_DROP;
  }

  return GST_BUS_PASS;
}


static GstElement *get_sink (int isRtmp)
{
  GstElement *sink;
  sink = gst_element_factory_make ("xvimagesink", NULL);
  if (isRtmp > 0){
    g_object_set (sink, "sync", FALSE, NULL);
  }
  return gst_object_ref_sink (sink);
}

static void
cb_message (GstBus     *bus,
            GstMessage *message,
            gpointer    user_data)
{
  GstElement *pipeline = GST_ELEMENT (user_data);

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
      g_print ("we received an error!\n");
      break;
    case GST_MESSAGE_EOS:
      g_print ("we reached EOS\n");
      break;
    case GST_MESSAGE_APPLICATION:
    {
      if (gst_message_has_name (message, "ExPrerolled")) {
        /* it's our message */
        g_print ("we are all prerolled, do seek\n");
        gst_element_seek (pipeline,
            1.0, GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
            GST_SEEK_TYPE_SET, 2 * GST_SECOND,
            GST_SEEK_TYPE_SET, 5 * GST_SECOND);

        gst_element_set_state (pipeline, GST_STATE_PLAYING);
      }
      break;
    }
    default:
      break;
  }
}

static void
on_about_to_finish (GstElement * playbin)
{
  // if (d->argv[++d->current_uri] == NULL)
  //   d->current_uri = 1;

  // g_print ("Now playing %s\n", d->argv[d->current_uri]);
  // g_object_set (playbin, "uri", d->argv[d->current_uri], NULL);
  // on_window_main_destroy();
}


void my_play_start(const char* uri, int isRtmp){
    GstBus *bus;
    FsElementAddedNotifier *notifier;
    gchar *codec_preferences_file;
    GstElement *sink;

    pipeline = gst_element_factory_make ("playbin", NULL);
    g_object_set (pipeline, "uri", uri, NULL);

    sink = get_sink (isRtmp);
    g_object_set (pipeline, "video-sink", sink, NULL);
    gst_object_unref (sink);

    // enable looping
    g_signal_connect (pipeline, "about-to-finish",
        G_CALLBACK (on_about_to_finish), NULL);

    notifier = fs_element_added_notifier_new ();
    fs_element_added_notifier_add (notifier, GST_BIN (pipeline));

    codec_preferences_file = my_file ("codec-properties.ini");
    fs_element_added_notifier_set_properties_from_file (notifier,
        codec_preferences_file, NULL);
    g_free (codec_preferences_file);



    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    g_signal_connect (bus, "message", (GCallback) cb_message,
        pipeline);
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) bus_sync_handler, NULL,
        NULL);
    gst_object_unref (bus);
      // play
    gst_element_set_state (pipeline, GST_STATE_PLAYING);   
}