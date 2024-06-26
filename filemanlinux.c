#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define COLUMN_NAME 0
#define COLUMN_TYPE 1
#define NUM_COLUMNS 2
//gcc main.c -o directory_list_view `pkg-config --cflags --libs gtk+-3.0`
void open_selected_file(GtkTreeView *treeview);
void create_new_folder(GtkWidget *widget, gpointer data);
void refresh_list_view(GtkWidget *treeview);
gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Directory List View");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GdkRGBA yellow = {1.0, 1.0, 0.0, 1.0};
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &yellow);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *treeview = gtk_tree_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), treeview, TRUE, TRUE, 0);

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COLUMN_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", COLUMN_TYPE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    GtkListStore *store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);

    refresh_list_view(treeview);

    GtkWidget *button = gtk_button_new_with_label("Create New Folder");
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(create_new_folder), treeview);
    g_signal_connect(treeview, "button-press-event", G_CALLBACK(on_button_press), treeview);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

void refresh_list_view(GtkWidget *treeview) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    gtk_list_store_clear(store);

    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);

        if (entry->d_type == DT_DIR) {
            gtk_list_store_set(store, &iter, COLUMN_NAME, entry->d_name, COLUMN_TYPE, "Folder", -1);
        } else {
            gtk_list_store_set(store, &iter, COLUMN_NAME, entry->d_name, COLUMN_TYPE, "File", -1);
        }
    }

    closedir(dir);
}

void open_selected_file(GtkTreeView *treeview) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *name;
        gtk_tree_model_get(model, &iter, COLUMN_NAME, &name, -1);

        gchar *cmd = g_strdup_printf("xdg-open '%s'", name);
        system(cmd);
        g_free(cmd);
        g_free(name);
    }
}

void create_new_folder(GtkWidget *widget, gpointer data) {
    const char *base_name = "NewFolder";
    char folder_name[256];
    int i = 0;

    while (1) {
        if (i == 0) {
            snprintf(folder_name, sizeof(folder_name), "%s", base_name);
        } else {
            snprintf(folder_name, sizeof(folder_name), "%s%d", base_name, i);
        }

        struct stat st;
        if (stat(folder_name, &st) != 0) {
            if (mkdir(folder_name, 0755) == 0) {
                refresh_list_view(GTK_WIDGET(data));
                return;
            } else {
                perror("mkdir");
                return;
            }
        }
        i++;
    }
}

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
        open_selected_file(GTK_TREE_VIEW(widget));
        return TRUE;
    }
    return FALSE;
}

