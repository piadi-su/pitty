#include <gtk/gtk.h>
#include <vte/vte.h>

// ================= APP DATA =================

typedef struct {
    gchar *shell;
    gchar *font;

    gchar *theme;
    gdouble transparency;
    gchar *mode;

    gchar *cursor;
    gboolean cursor_blink;

    GtkWidget *window;
} AppData;

// ================= THEME =================

static gboolean parse_hex_color(const gchar *hex, GdkRGBA *color) {
    return gdk_rgba_parse(color, hex);
}

static void apply_theme(VteTerminal *term, AppData *app) {

    gchar *theme_path = NULL;

    // ================= PYWAL =================

    if (g_strcmp0(app->theme, "pywal") == 0) {

        theme_path = g_build_filename(
            g_get_home_dir(),
            ".cache",
            "wal",
            "colors",
            NULL
        );

    } else {

        // ================= CUSTOM THEMES =================

        gchar *filename = g_strdup_printf(
            "%s.conf",
            app->theme
        );

        theme_path = g_build_filename(
            g_get_home_dir(),
            ".config",
            "pitty",
            "themes",
            filename,
            NULL
        );

        g_free(filename);
    }

    FILE *fp = fopen(theme_path, "r");

    if (!fp) {
        g_free(theme_path);
        return;
    }

    gchar line[128];

    GdkRGBA fg = {0};
    GdkRGBA bg = {0};
    GdkRGBA palette[16];

    int color_index = 0;

    while (fgets(line, sizeof(line), fp)) {

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) < 7)
            continue;

        GdkRGBA color;

        if (!parse_hex_color(line, &color))
            continue;

        if (color_index == 0)
            bg = color;
        else if (color_index == 7)
            fg = color;

        if (color_index < 16)
            palette[color_index] = color;

        color_index++;
    }

    fclose(fp);

    vte_terminal_set_colors(
        term,
        &fg,
        &bg,
        palette,
        16
    );

    g_free(theme_path);
}

// ================= CONFIG =================

void load_config(AppData *app) {

    GKeyFile *kf = g_key_file_new();

    gchar *path = g_build_filename(
        g_get_home_dir(),
        ".config/pitty/pitty.conf",
        NULL
    );

    // DEFAULTS

    app->shell = g_strdup("/bin/bash");
    app->font = g_strdup("Monospace 12");

    app->theme = g_strdup("pywal");

    app->transparency = 1.0;
    app->mode = g_strdup("normal");

    app->cursor = g_strdup("block");
    app->cursor_blink = TRUE;

    if (g_key_file_load_from_file(
            kf,
            path,
            G_KEY_FILE_NONE,
            NULL
        )) {

        gchar *tmp;

        tmp = g_key_file_get_string(kf, "main", "shell", NULL);
        if (tmp) {
            g_free(app->shell);
            app->shell = tmp;
        }

        tmp = g_key_file_get_string(kf, "main", "font", NULL);
        if (tmp) {
            g_free(app->font);
            app->font = tmp;
        }

        tmp = g_key_file_get_string(kf, "main", "theme", NULL);
        if (tmp) {
            g_free(app->theme);
            app->theme = tmp;
        }

        tmp = g_key_file_get_string(kf, "main", "mode", NULL);
        if (tmp) {
            g_free(app->mode);
            app->mode = tmp;
        }

        tmp = g_key_file_get_string(kf, "main", "cursor", NULL);
        if (tmp) {
            g_free(app->cursor);
            app->cursor = tmp;
        }

        // transparency

        GError *err = NULL;

        app->transparency = g_key_file_get_double(
            kf,
            "main",
            "transparency",
            &err
        );

        if (err) {
            g_clear_error(&err);
            app->transparency = 1.0;
        }

        // cursor blink

        GError *blink_err = NULL;

        app->cursor_blink = g_key_file_get_boolean(
            kf,
            "main",
            "cursor_blink",
            &blink_err
        );

        if (blink_err) {
            g_clear_error(&blink_err);
            app->cursor_blink = TRUE;
        }
    }

    g_key_file_free(kf);
    g_free(path);
}

// ================= EXIT =================

void on_child_exit(
    VteTerminal *term,
    gint status,
    gpointer user_data
) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_window_close(GTK_WINDOW(window));
}

// ================= KEYBINDS =================

gboolean on_key(
    GtkWidget *widget,
    GdkEventKey *event,
    gpointer user_data
) {
    VteTerminal *term = VTE_TERMINAL(widget);

    if ((event->state & GDK_CONTROL_MASK) &&
        (event->state & GDK_SHIFT_MASK)) {

        double scale = vte_terminal_get_font_scale(term);

        switch (event->keyval) {

            // COPY

            case GDK_KEY_C:
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(
                    term,
                    VTE_FORMAT_TEXT
                );
                return TRUE;

            // PASTE

            case GDK_KEY_V:
            case GDK_KEY_v:
                vte_terminal_paste_clipboard(term);
                return TRUE;

            // ZOOM IN

            case GDK_KEY_plus:
            case GDK_KEY_equal:
            case GDK_KEY_KP_Add:
                scale += 0.1;
                break;

            // ZOOM OUT

            case GDK_KEY_minus:
            case GDK_KEY_underscore:
            case GDK_KEY_KP_Subtract:
                scale -= 0.1;
                break;

            // RESET

            case GDK_KEY_0:
                scale = 1.0;
                break;

            default:
                return FALSE;
        }

        if (scale < 0.5)
            scale = 0.5;

        if (scale > 3.0)
            scale = 3.0;

        vte_terminal_set_font_scale(term, scale);

        return TRUE;
    }

    return FALSE;
}

// ================= TERMINAL =================

GtkWidget* create_terminal(AppData *app) {

    GtkWidget *term = vte_terminal_new();

    vte_terminal_set_scrollback_lines(
        VTE_TERMINAL(term),
        10000
    );

    // FONT

    PangoFontDescription *desc =
        pango_font_description_from_string(app->font);

    vte_terminal_set_font(
        VTE_TERMINAL(term),
        desc
    );

    pango_font_description_free(desc);

    // SHELL

    char *argv[] = { app->shell, NULL };

    char **envv = g_get_environ();

    envv = g_environ_setenv(
        envv,
        "TERM",
        "xterm-256color",
        TRUE
    );

    vte_terminal_spawn_async(
        VTE_TERMINAL(term),
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
        argv,
        envv,
        G_SPAWN_DEFAULT,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        NULL,
        NULL
    );

    g_strfreev(envv);

    // CURSOR SHAPE

    if (g_strcmp0(app->cursor, "beam") == 0) {

        vte_terminal_set_cursor_shape(
            VTE_TERMINAL(term),
            VTE_CURSOR_SHAPE_IBEAM
        );

    } else {

        vte_terminal_set_cursor_shape(
            VTE_TERMINAL(term),
            VTE_CURSOR_SHAPE_BLOCK
        );
    }

    // CURSOR BLINK

    vte_terminal_set_cursor_blink_mode(
        VTE_TERMINAL(term),
        app->cursor_blink
            ? VTE_CURSOR_BLINK_ON
            : VTE_CURSOR_BLINK_OFF
    );

    // APPLY THEME

    apply_theme(VTE_TERMINAL(term), app);

    // SIGNALS

    g_signal_connect(
        term,
        "child-exited",
        G_CALLBACK(on_child_exit),
        app->window
    );

    g_signal_connect(
        term,
        "key-press-event",
        G_CALLBACK(on_key),
        NULL
    );

    return term;
}

// ================= STYLE =================

void apply_style(AppData *app) {

    gtk_widget_set_opacity(
        app->window,
        app->transparency
    );
}

// ================= UI =================

static void activate(
    GtkApplication *gtk_app,
    gpointer user_data
) {

    AppData *app = user_data;

    app->window =
        gtk_application_window_new(gtk_app);

    gtk_window_set_default_size(
        GTK_WINDOW(app->window),
        900,
        600
    );

    gboolean minimal =
        (g_strcmp0(app->mode, "minimal") == 0);

    if (!minimal) {

        GtkWidget *header =
            gtk_header_bar_new();

        gtk_header_bar_set_title(
            GTK_HEADER_BAR(header),
            "pitty"
        );

        gtk_header_bar_set_show_close_button(
            GTK_HEADER_BAR(header),
            TRUE
        );

        gtk_window_set_titlebar(
            GTK_WINDOW(app->window),
            header
        );

    } else {

        gtk_window_set_decorated(
            GTK_WINDOW(app->window),
            FALSE
        );
    }

    apply_style(app);

    GtkWidget *term =
        create_terminal(app);

    gtk_container_add(
        GTK_CONTAINER(app->window),
        term
    );

    gtk_widget_show_all(app->window);
}

// ================= MAIN =================

int main(int argc, char **argv) {

    GtkApplication *app;
    int status;

    AppData data;

    load_config(&data);

    app = gtk_application_new(
        "com.pitty.terminal",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        &data
    );

    status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv
    );

    g_object_unref(app);

    g_free(data.shell);
    g_free(data.font);

    g_free(data.theme);

    g_free(data.mode);

    g_free(data.cursor);

    return status;
}
