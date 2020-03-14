#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_FILE "test.h264"
#define POSITION_X 100
#define POSITION_Y 100
#define SHOW_FPS_VALUE "1"
#define DELAY_VALUE 1000000
#define EXPORT_VAR "SHOW_FPS"

int
main (int argc, char *argv[])
{
  GstElement *pipeline, *source, *parser, *decoder, *sink, *fpssink;
  GstBus *bus;
  GstMessage *msg;
  gchar *fps_msg;
  guint delay_show_FPS = 0;
  gchar *export_value;
  gboolean flag_fps = 0;

  /* Check function show FPS */
  export_value = getenv(EXPORT_VAR);
  if (export_value != NULL)
    if (strcmp(export_value, SHOW_FPS_VALUE) == 0)
      flag_fps = 1;

  const gchar *input_file = INPUT_FILE;

  /* Initialization */
  gst_init (&argc, &argv);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-play");
  source = gst_element_factory_make ("filesrc", "file-source");
  parser = gst_element_factory_make ("h264parse", "h264-parser");
  decoder = gst_element_factory_make ("omxh264dec", "h264-decoder");
  sink = gst_element_factory_make ("waylandsink", "video-output");
  fpssink = gst_element_factory_make ("fpsdisplaysink", NULL);

  if (!pipeline || !source || !parser || !decoder || !sink || !fpssink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set input video file for source element */
  g_object_set (G_OBJECT (source), "location", input_file, NULL);

  /* Set position for displaying (100, 100) */
  g_object_set (G_OBJECT (sink), "position-x", POSITION_X, "position-y", POSITION_Y, NULL);

  /* Add feature FPS for video-sink */
  if (flag_fps)
    g_object_set (G_OBJECT (fpssink), "text-overlay", FALSE, "video-sink", sink, NULL);

  /* Add all elements into the pipeline */
  /* pipeline---[ file-source + h264-parser + h264-decoder + video-output ] */
  if (flag_fps)
    gst_bin_add_many (GST_BIN (pipeline), source, parser, decoder, fpssink, NULL);
  else
    gst_bin_add_many (GST_BIN (pipeline), source, parser, decoder, sink, NULL);

  /* Link the elements together */
  /* file-source -> h264-parser -> h264-decoder -> video-output */
  if (flag_fps) {
    if (gst_element_link_many (source, parser, decoder, fpssink, NULL) != TRUE) {
      g_printerr ("Elements could not be linked.\n");
      gst_object_unref (pipeline);
      return -1;
    }
  } else {
    if (gst_element_link_many (source, parser, decoder, sink, NULL) != TRUE) {
      g_printerr ("Elements could not be linked.\n");
      gst_object_unref (pipeline);
      return -1;
    }    
  }

  /* Set the pipeline to "playing" state */
  g_print ("Now playing: %s\n", input_file);
  if (gst_element_set_state (pipeline,
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  g_print ("Running...\n");

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  while(1) {
    msg = gst_bus_pop (bus);

    /* Note that because input timeout is GST_CLOCK_TIME_NONE, 
       the gst_bus_timed_pop_filtered() function will block forever until a 
       matching message was posted on the bus (GST_MESSAGE_ERROR or 
       GST_MESSAGE_EOS). */
    if (msg != NULL) {
      GError *err;
      gchar *debug_info;
      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &err, &debug_info);
          g_printerr ("Error received from element %s: %s.\n",
            GST_OBJECT_NAME (msg->src), err->message);
          g_printerr ("Debugging information: %s.\n",
            debug_info ? debug_info : "none");
          g_clear_error (&err);
          g_free (debug_info);
          goto stop_pipeline;
        case GST_MESSAGE_EOS:
          g_print ("End-Of-Stream reached.\n");
          goto stop_pipeline;
      }
      gst_message_unref (msg);
    }

    /* Display information FPS to console */
    if (flag_fps) {
      g_object_get (G_OBJECT (fpssink), "last-message", &fps_msg, NULL);
      delay_show_FPS++;
      if (fps_msg != NULL) {
        if ((delay_show_FPS % DELAY_VALUE) == 0) {
          g_print ("Frame info: %s\n", fps_msg);
          delay_show_FPS = 0;
        }
      }
    }
  }

  /* Free resources and change state to NULL */
  stop_pipeline:
  gst_object_unref (bus);
  g_print ("Returned, stopping playback...\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Freeing pipeline...\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_print ("Completed. Goodbye!\n");
  return 0;
}