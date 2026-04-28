#include <gtk/gtk.h>
#include <vte/vte.h>

//APP DATA 

typedef struct {
    gchar *shell;
    gchar *font;
    gchar *theme;
    gdouble transparency;
    gchar *mode;

    gchar *cursor;   

    GtkWidget *window;
} AppData;

//config

void load_config(AppData *app) {
    GKeyFile *kf = g_key_file_new();
    gchar *path = g_build_filename(g_get_home_dir(), ".config/pitty/pitty.conf", NULL);

    app->shell = g_strdup("/bin/bash");
    app->font  = g_strdup("Monospace 12");
    app->theme = g_strdup("dark");
    app->transparency = 1.0;
    app->mode = g_strdup("normal");
    app->cursor = g_strdup("block"); 

    if (g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, NULL)) {

        app->shell = g_key_file_get_string(kf, "main", "shell", NULL);
        app->font  = g_key_file_get_string(kf, "main", "font", NULL);
        app->theme = g_key_file_get_string(kf, "main", "theme", NULL);
        app->mode  = g_key_file_get_string(kf, "main", "mode", NULL);

        app->cursor = g_key_file_get_string(kf, "main", "cursor", NULL);
        if (!app->cursor)
            app->cursor = g_strdup("block");

        GError *err = NULL;
        app->transparency = g_key_file_get_double(kf, "main", "transparency", &err);
        if (err) {
            g_clear_error(&err);
            app->transparency = 1.0;
        }

        if (!app->shell) app->shell = g_strdup("/bin/bash");
        if (!app->font)  app->font  = g_strdup("Monospace 12");
        if (!app->theme) app->theme = g_strdup("dark");
        if (!app->mode)  app->mode  = g_strdup("normal");
    }

    g_key_file_free(kf);
    g_free(path);
}

//exit

void on_child_exit(VteTerminal *term, gint status, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_window_close(GTK_WINDOW(window));
}

//zoom

gboolean on_key(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    VteTerminal *term = VTE_TERMINAL(widget);

    if ((event->state & GDK_CONTROL_MASK) &&
        (event->state & GDK_SHIFT_MASK)) {

        double scale = vte_terminal_get_font_scale(term);

        switch (event->keyval) {

            case GDK_KEY_plus:
            case GDK_KEY_equal:
            case GDK_KEY_KP_Add:
                scale += 0.1;
                break;

            case GDK_KEY_minus:
            case GDK_KEY_underscore:
            case GDK_KEY_KP_Subtract:
                scale -= 0.1;
                break;

            case GDK_KEY_0:
                scale = 1.0;
                break;

            default:
                return FALSE;
        }

        if (scale < 0.5) scale = 0.5;
        if (scale > 3.0) scale = 3.0;

        vte_terminal_set_font_scale(term, scale);
        return TRUE;
    }

    return FALSE;
}

//terminal

GtkWidget* create_terminal(AppData *app) {
    GtkWidget *term = vte_terminal_new();

    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term), 10000);

    PangoFontDescription *desc = pango_font_description_from_string(app->font);
    vte_terminal_set_font(VTE_TERMINAL(term), desc);
    pango_font_description_free(desc);

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

    // cursor

    if (g_strcmp0(app->cursor, "beam") == 0) {
        vte_terminal_set_cursor_shape(VTE_TERMINAL(term),
            VTE_CURSOR_SHAPE_IBEAM);
    } else {
        vte_terminal_set_cursor_shape(VTE_TERMINAL(term),
            VTE_CURSOR_SHAPE_BLOCK);
    }

    g_signal_connect(term, "child-exited",
                     G_CALLBACK(on_child_exit), app->window);

    g_signal_connect(term, "key-press-event",
                     G_CALLBACK(on_key), NULL);

    return term;
}

//style

void apply_style(AppData *app) {
    gboolean dark = (g_strcmp0(app->theme, "dark") == 0);

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings,
        "gtk-application-prefer-dark-theme", dark,
        NULL);

    gtk_widget_set_opacity(app->window, app->transparency);
}

//ui

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    AppData *app = user_data;

    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 600);

    gboolean minimal = (g_strcmp0(app->mode, "minimal") == 0);

    if (!minimal) {
        GtkWidget *header = gtk_header_bar_new();
        gtk_header_bar_set_title(GTK_HEADER_BAR(header), "pitty");
        gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
        gtk_window_set_titlebar(GTK_WINDOW(app->window), header);
    } else {
        gtk_window_set_decorated(GTK_WINDOW(app->window), FALSE);
    }

    apply_style(app);

    GtkWidget *term = create_terminal(app);
    gtk_container_add(GTK_CONTAINER(app->window), term);

    gtk_widget_show_all(app->window);
}

//main

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
    g_free(data.theme);
    g_free(data.mode);

    g_free(data.cursor);

    return status;
}
