#ifndef GST_MY_H
#define GST_MY_H

void gst_my_init(int *argc,
              char **argv[]);

guint64 gst_query_duration ();
guint64 gst_query_position ();
void gst_seek_absolute (guint64 value);
void gst_set_window (guintptr window);
void gst_reset (void);
void gst_pause (void);
void gst_resume (void);
void gst_stop(void);
void my_play_start(const gchar* uri, int isRtmp);
gchar* my_file (const gchar * name);
#endif /* GST_MY_H */
