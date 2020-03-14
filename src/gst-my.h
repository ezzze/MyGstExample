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
