#include <gtk/gtk.h>

#include <librcc.h>
#include <librccui.h>

static rcc_class classes[] = {
    { "id3", RCC_CLASS_STANDARD, NULL, NULL, "ID3 Encoding", 0 },
    { "pl", RCC_CLASS_STANDARD, "id3", NULL, "PlayList Title Encoding", 0},
    { "plfs", RCC_CLASS_STANDARD, "pl", NULL, "PlayList File Encoding", 0 },
    { "fs", RCC_CLASS_FS, "LC_CTYPE", NULL, "FileSystem Encoding", 0 },
    { "oem", RCC_CLASS_STANDARD, NULL, NULL, "Zip OEM Encoding", 0 },
    { "iso", RCC_CLASS_STANDARD, NULL, NULL, "Zip ISO Encoding", 0 },
    { "out", RCC_CLASS_STANDARD, "LC_CTYPE", NULL, NULL, 0 },
    { NULL }
};

static char *config;
static rcc_context ctx;
static rcc_ui_context uictx;

static void apply_cb(GtkWidget * w, gpointer data)
{
    rccUiUpdate(uictx);
    rccSave(ctx, config);
}
 
 
int main (int argc, char *argv[])
{
    GtkWidget *window1;
    GtkWidget *save, *close, *hbox;
    GtkWidget *box;
    
    
    if (argc<1) config = argv[0];
    else config = argv[1];

    gtk_set_locale ();
    gtk_init (&argc, &argv);

    rccInit();
    rccUiInit();
    ctx = rccCreateContext(NULL, 0, 0, classes, 0);
    rccLoad(ctx, config);
    uictx = rccUiCreateContext(ctx);

    window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_policy(GTK_WINDOW (window1), FALSE, FALSE, TRUE);
    gtk_window_set_title (GTK_WINDOW (window1), "LibRCC Config");
    gtk_widget_show(window1);
    
    box = rccUiGetPage(uictx, NULL);
    gtk_widget_show (box);
    gtk_container_add (GTK_CONTAINER (window1), box);

    hbox = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

    close = gtk_button_new_with_label("Close");
    gtk_widget_show (close);
    gtk_signal_connect (GTK_OBJECT (window1), "destroy", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (close), "clicked", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), close, FALSE, FALSE, 0);

    save = gtk_button_new_with_label("Save");
    gtk_widget_show (save);
    gtk_signal_connect(GTK_OBJECT(save), "clicked", GTK_SIGNAL_FUNC(apply_cb), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), save, FALSE, FALSE, 0);

    gtk_main ();

    rccUiFreeContext(uictx);
    rccFreeContext(ctx);
    rccUiFree();
    rccFree();

    return 0;
}
