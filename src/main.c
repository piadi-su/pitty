#include <gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
    gchar *shell;
    gchar *font;

    gchar *theme;
    gdouble transparency;
    gchar *mode;

    GtkWidget *window;
} AppData;

/* ---------------- CONFIG ---------------- */

void load_config(AppData *app) {
    GKeyFile *kf = g_key_file_new();
    gchar *path = g_build_filename(g_get_home_dir(), ".config/pitty/pitty.conf", NULL);

    if (!g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, NULL)) {
        app->shell = g_strdup(g_getenv("SHELL") ? g_getenv("SHELL") : "/bin/bash");
        app->font  = g_strdup("Monospace 12");

        app->theme = g_strdup("dark");
        app->transparency = 1.0;
        app->mode = g_strdup("normal");
    } else {
        app->shell = g_key_file_get_string(kf, "main", "shell", NULL);
        app->font  = g_key_file_get_string(kf, "main", "font", NULL);

        app->theme = g_key_file_get_string(kf, "main", "theme", NULL);
        app->transparency = g_key_file_get_double(kf, "main", "transparency", NULL);
        app->mode = g_key_file_get_string(kf, "main", "mode", NULL);

        if (!app->shell) app->shell = g_strdup("/bin/bash");
        if (!app->font)  app->font  = g_strdup("Monospace 12");
        if (!app->theme) app->theme = g_strdup("dark");
        if (!app->mode)  app->mode  = g_strdup("normal");
        if (app->transparency <= 0) app->transparency = 1.0;
    }

    g_key_file_free(kf);
    g_free(path);
}

/* ---------------- EXIT HANDLER ---------------- */

void on_child_exit(VteTerminal *term, gint status, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);

    gtk_window_close(GTK_WINDOW(window));
}

/* ---------------- TERMINAL ---------------- */

GtkWidget* create_terminal(AppData *app) {
    GtkWidget *term = vte_terminal_new();

    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term), 10000);

    /* FONT */
    PangoFontDescription *desc = pango_font_description_from_string(app->font);

    if (!desc || !pango_font_description_get_family(desc)) {
        g_warning("Font non valido: %s", app->font);
        if (desc) pango_font_description_free(desc);
        desc = pango_font_description_from_string("Monospace 12");
    }

    vte_terminal_set_font(VTE_TERMINAL(term), NULL);
    vte_terminal_set_font(VTE_TERMINAL(term), desc);
    pango_font_description_free(desc);

    /* SHELL */
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

    /* EXIT FIX */
    g_signal_connect(term, "child-exited", G_CALLBACK(on_child_exit), app->window);

    return term;
}

/* ---------------- STYLE ---------------- */

void apply_style(AppData *app) {
    gboolean dark = (g_strcmp0(app->theme, "dark") == 0);

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings,
        "gtk-application-prefer-dark-theme", dark,
        NULL);

    gtk_widget_set_opacity(app->window, app->transparency);

    if (g_strcmp0(app->mode, "minimal") == 0) {
        gtk_window_set_decorated(GTK_WINDOW(app->window), FALSE);
    }
}

/* ---------------- UI ---------------- */

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    AppData *app = user_data;

    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 600);

    apply_style(app);

    GtkWidget *term = create_terminal(app);
    gtk_container_add(GTK_CONTAINER(app->window), term);

    gtk_widget_show_all(app->window);
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
    g_free(data.theme);
    g_free(data.mode);

    return status;
}
