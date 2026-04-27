#include <gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    gchar *shell;
    gchar *font;
    GtkWidget *notebook;
} AppData;

/* ---------------- CONFIG ---------------- */

void load_config(AppData *app) {
    GKeyFile *keyfile = g_key_file_new();
    gchar *path = g_build_filename(g_get_home_dir(), ".simple_terminal.conf", NULL);

    if (!g_key_file_load_from_file(keyfile, path, G_KEY_FILE_NONE, NULL)) {
        app->shell = g_strdup(g_getenv("SHELL") ? g_getenv("SHELL") : "/bin/bash");
        app->font  = g_strdup("Monospace 12");
    } else {
        app->shell = g_key_file_get_string(keyfile, "main", "shell", NULL);
        app->font  = g_key_file_get_string(keyfile, "main", "font", NULL);

        if (!app->shell) app->shell = g_strdup("/bin/bash");
        if (!app->font)  app->font  = g_strdup("Monospace 12");
    }

    g_key_file_free(keyfile);
    g_free(path);
}

/* ---------------- EXIT HANDLER ---------------- */

void on_child_exit(VteTerminal *term, gint status, gpointer user_data) {
    GtkWidget *notebook = GTK_WIDGET(user_data);

    int page = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), GTK_WIDGET(term));

    if (page != -1)
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), page);

    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) == 0) {
        GtkWidget *win = gtk_widget_get_toplevel(notebook);
        gtk_window_close(GTK_WINDOW(win));
    }
}

/* ---------------- TERMINAL ---------------- */

GtkWidget* create_terminal(AppData *app) {
    GtkWidget *term = vte_terminal_new();

    /* SCROLLBACK */
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term), 10000);

    /* ---------------- FONT FIX SERIO ---------------- */
    PangoFontDescription *desc = pango_font_description_from_string(app->font);

    if (!desc || !pango_font_description_get_family(desc)) {
        g_warning("Font non valido: %s, uso Monospace", app->font);
        if (desc) pango_font_description_free(desc);
        desc = pango_font_description_from_string("Monospace 12");
    }

    vte_terminal_set_font(VTE_TERMINAL(term), NULL);
    vte_terminal_set_font(VTE_TERMINAL(term), desc);
    pango_font_description_free(desc);

    /* ---------------- SHELL ---------------- */
    char *argv[] = { app->shell, NULL };

    vte_terminal_spawn_async(
        VTE_TERMINAL(term),
        VTE_PTY_DEFAULT,
        NULL,
        argv,
        NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        NULL,
        -1,
        NULL,
        NULL, NULL
    );

    g_signal_connect(term, "child-exited", G_CALLBACK(on_child_exit), app->notebook);

    return term;
}

/* ---------------- TAB ---------------- */

void new_tab(AppData *app) {
    GtkWidget *term = create_terminal(app);
    GtkWidget *label = gtk_label_new("Terminal");

    int page = gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), term, label);
    gtk_widget_show_all(term);

    gtk_notebook_set_current_page(GTK_NOTEBOOK(app->notebook), page);
}

void close_tab(AppData *app) {
    int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    if (page != -1)
        gtk_notebook_remove_page(GTK_NOTEBOOK(app->notebook), page);
}

/* ---------------- SHORTCUTS ---------------- */

gboolean on_key(GtkWidget *w, GdkEventKey *e, gpointer data) {
    AppData *app = data;

    if ((e->state & GDK_CONTROL_MASK) && (e->state & GDK_SHIFT_MASK)) {
        if (e->keyval == GDK_KEY_T) {
            new_tab(app);
            return TRUE;
        }
        if (e->keyval == GDK_KEY_W) {
            close_tab(app);
            return TRUE;
        }
    }

    return FALSE;
}

/* ---------------- UI ---------------- */

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    AppData *app = user_data;

    GtkWidget *win = gtk_application_window_new(gtk_app);
    gtk_window_set_default_size(GTK_WINDOW(win), 900, 600);

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "pitty");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(win), header);

    GtkWidget *btn = gtk_button_new_with_label("+");
    g_signal_connect_swapped(btn, "clicked", G_CALLBACK(new_tab), app);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn);

    app->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(app->notebook), TRUE);

    gtk_container_add(GTK_CONTAINER(win), app->notebook);

    new_tab(app);

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key), app);

    gtk_widget_show_all(win);
}

/* ---------------- MAIN ---------------- */

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    AppData data;
    load_config(&data);

    app = gtk_application_new(
        "com.pitty.terminal",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(activate), &data);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    g_free(data.shell);
    g_free(data.font);

    return status;
}
