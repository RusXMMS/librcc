/*
  LibRCC - standalone application for adjusting RCC configuration

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>

#include <librcc.h>
#include <librccui.h>

static rcc_class classes[] = {
    { "id3", RCC_CLASS_STANDARD, NULL, NULL, "ID3 Encoding", 0 },
    { "id3v2", RCC_CLASS_STANDARD, "id3", NULL, "ID3 v.2 Encoding", 0},
    { "pl", RCC_CLASS_STANDARD, "id3", NULL, "PlayList Title Encoding", 0},
    { "plfs", RCC_CLASS_STANDARD, "pl", NULL, "PlayList File Encoding", 0 },
    { "fs", RCC_CLASS_FS, "LC_CTYPE", NULL, "FileSystem Encoding", 0 },
    { "oem", RCC_CLASS_STANDARD, NULL, NULL, "Zip OEM Encoding", 0 },
    { "iso", RCC_CLASS_STANDARD, NULL, NULL, "Zip ISO Encoding", 0 },
    { "ftp", RCC_CLASS_STANDARD, NULL, NULL, "FTP Encoding", 0 },
    { "in", RCC_CLASS_STANDARD, NULL, NULL, "Input Encoding", 0 },
    { "out", RCC_CLASS_STANDARD, "LC_CTYPE", NULL, "Output Encoding", 0 },
    { NULL }
};

static char *config = NULL;
static rcc_context ctx;
static rcc_ui_context uictx;

static void apply_cb(GtkWidget * w, gpointer data)
{
    rccUiUpdate(uictx);
    rccSave(ctx, config);
}
 
 
int main (int argc, char *argv[])
{
    unsigned int i;
    unsigned char all_mode = 0;
    GtkWidget *window1;
    GtkWidget *scroll;
    GtkWidget *save, *close, *hbox;
    GtkWidget *box;
    GdkGeometry hints;
    GtkRequisition size;
    char title[128];
    
    
    for (i=1;i<argc;i++) {
	if (!strcmp(argv[i],"--all")) all_mode = 1;
	else if (!strcmp(argv[i], "--")) break;
	else if ((!config)&&(strncmp(argv[i],"-",1))) config = argv[i];
    }
    if (i==1) {
	printf("Usage: rcc-config [ --all ] [ <config name> ]\n");
	printf(" Known configs: xmms, ftp, zip\n");
    }
    
    if (config) {
        if (strlen(config) > 64) {
            fprintf(stderr, "Config name is too long...");
            exit(-1);
        }
        sprintf(title, "LibRCC Config [%s]", config);
    } else {
        sprintf(title, "LibRCC Config [default]");
    }

#if GTK_MAJOR_VERSION > 2
    setlocale(LC_ALL, "");
#else /* GTK_MAJOR_VERSION < 3 */
    gtk_set_locale ();
#endif /* GTK_MAJOR_VERSION */

    gtk_init (&argc, &argv);

    rccInit();
    rccUiInit();
    ctx = rccCreateContext(NULL, 0, 0, classes, 0);
    rccLoad(ctx, config);
    uictx = rccUiCreateContext(ctx);
    if (all_mode) rccUiUnHideOption(uictx, RCC_OPTION_ALL);

    window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#if GTK_MAJOR_VERSION > 2
//    gtk_window_set_resizable(GTK_WINDOW(window1), FALSE);
#else /* GTK_MAJOR_VERSION < 3 */
//    gtk_window_set_policy(GTK_WINDOW (window1), FALSE, FALSE, TRUE);
#endif /* GTK_MAJOR_VERSION */
    gtk_window_set_title (GTK_WINDOW (window1), title);
    gtk_window_set_wmclass (GTK_WINDOW(window1), "librcc", "libRCC");
    gtk_widget_show(window1);
    
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show(scroll);
    gtk_container_add(GTK_CONTAINER(window1), scroll);
    
    box = rccUiGetPage(uictx, NULL);
    gtk_widget_show (box);

#if ((GTK_MAJOR_VERSION < 3) || (GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION < 7) || (GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION == 7 && GTK_MICRO_VERSION < 8))
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), box);
#else
    gtk_container_add (GTK_CONTAINER (scroll), box);
#endif

#if GTK_MAJOR_VERSION > 2
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
#else /* GTK_MAJOR_VERSION < 3 */
    hbox = gtk_hbox_new (TRUE, 0);
#endif /* GTK_MAJOR_VERSION */

    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

    close = gtk_button_new_with_label("Close");
    gtk_widget_show (close);
#if GTK_MAJOR_VERSION > 2
    g_signal_connect (G_OBJECT (window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (close), "clicked", G_CALLBACK(gtk_main_quit), NULL);
#else /* GTK_MAJOR_VERSION < 3 */
    gtk_signal_connect (GTK_OBJECT (window1), "destroy", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (close), "clicked", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
#endif /* GTK_MAJOR_VERSION */
    gtk_box_pack_start (GTK_BOX (hbox), close, FALSE, FALSE, 0);

    save = gtk_button_new_with_label("Save");
    gtk_widget_show (save);
#if GTK_MAJOR_VERSION > 2
    g_signal_connect(G_OBJECT(save), "clicked", G_CALLBACK(apply_cb), NULL);
#else /* GTK_MAJOR_VERSION < 3 */
    gtk_signal_connect(GTK_OBJECT(save), "clicked", GTK_SIGNAL_FUNC(apply_cb), NULL);
#endif /* GTK_MAJOR_VERSION */
    gtk_box_pack_start (GTK_BOX (hbox), save, FALSE, FALSE, 0);

#if GTK_MAJOR_VERSION > 2
    gtk_widget_get_preferred_size (box, NULL, &size);
#else /* GTK_MAJOR_VERSION < 3 */
    gtk_widget_size_request(box, &size);
#endif /* GTK_MAJOR_VERSION */

    hints.min_width =  size.width;
    hints.min_height = size.height;

#if GTK_MAJOR_VERSION < 3
    hints.min_width +=  25;
    hints.min_height += 25;
#endif /* GTK_MAJOR_VERSION */

    gtk_window_set_geometry_hints(GTK_WINDOW(window1), scroll, &hints, GDK_HINT_MIN_SIZE);

    gtk_main ();

    rccUiFreeContext(uictx);
    rccFreeContext(ctx);
    rccUiFree();
    rccFree();

    return 0;
}
