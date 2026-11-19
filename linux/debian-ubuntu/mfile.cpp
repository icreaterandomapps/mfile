#include <gtk/gtk.h>
#include <gio/gio.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {

enum FileCols {
    COL_ICON = 0,
    COL_NAME,
    COL_MODIFIED,
    COL_TYPE,
    COL_SIZE_TEXT,
    COL_PATH,
    COL_IS_DIR,
    COL_SIZE_BYTES,
    COL_MTIME,
    COL_COUNT
};

enum SideCols {
    SCOL_ICON = 0,
    SCOL_NAME,
    SCOL_PATH,
    SCOL_IS_COMPUTER,
    SCOL_COUNT
};

struct App {
    GtkWidget *window = nullptr;
    GtkWidget *path_entry = nullptr;
    GtkWidget *search_entry = nullptr;
    GtkWidget *file_view = nullptr;
    GtkWidget *side_view = nullptr;
    GtkWidget *status_label = nullptr;
    GtkWidget *show_hidden_check = nullptr;
    GtkListStore *file_store = nullptr;
    GtkTreeStore *side_store = nullptr;

    std::string current_path;
    std::vector<std::string> back_history;
    std::vector<std::string> forward_history;
    bool show_hidden = false;
    bool suppress_nav = false;
};

App g;

std::string path_join(const std::string &a, const std::string &b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a == "/") return "/" + b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

std::string dirname_of(const std::string &p) {
    if (p.empty() || p == "/") return "/";
    std::string s = p;
    while (s.size() > 1 && s.back() == '/') s.pop_back();
    size_t pos = s.find_last_of('/');
    if (pos == std::string::npos) return "/";
    if (pos == 0) return "/";
    return s.substr(0, pos);
}

std::string basename_of(const std::string &p) {
    if (p == "/") return "/";
    std::string s = p;
    while (s.size() > 1 && s.back() == '/') s.pop_back();
    size_t pos = s.find_last_of('/');
    if (pos == std::string::npos) return s;
    return s.substr(pos + 1);
}

bool is_dir_path(const std::string &p) {
    struct stat st;
    if (stat(p.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

std::string format_size(guint64 bytes, bool is_dir) {
    if (is_dir) return "";
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double v = (double)bytes;
    int u = 0;
    while (v >= 1024.0 && u < 4) { v /= 1024.0; ++u; }
    char buf[64];
    if (u == 0) std::snprintf(buf, sizeof(buf), "%llu %s", (unsigned long long)bytes, units[u]);
    else std::snprintf(buf, sizeof(buf), "%.1f %s", v, units[u]);
    return buf;
}

std::string format_time(time_t t) {
    if (t <= 0) return "";
    struct tm lt;
    if (!localtime_r(&t, &lt)) return "";
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &lt);
    return buf;
}

std::string lower_ascii(std::string s) {
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'A' && c <= 'Z') s[i] = (char)(c - 'A' + 'a');
    }
    return s;
}

void set_status(const std::string &msg) {
    if (g.status_label) gtk_label_set_text(GTK_LABEL(g.status_label), msg.c_str());
}

bool path_is_computer(const std::string &p) {
    return p == "computer:///";
}

std::string type_for_entry(const std::string &full, bool is_dir) {
    if (is_dir) return "Folder";
    gchar *ctype = g_content_type_guess(full.c_str(), nullptr, 0, nullptr);
    std::string out = "File";
    if (ctype) {
        gchar *desc = g_content_type_get_description(ctype);
        if (desc) { out = desc; g_free(desc); }
        else { out = ctype; }
        g_free(ctype);
    }
    return out;
}

GIcon *icon_for_entry(const std::string &full, bool is_dir) {
    if (is_dir) {
        return g_themed_icon_new("folder");
    }
    gchar *ctype = g_content_type_guess(full.c_str(), nullptr, 0, nullptr);
    GIcon *icon = nullptr;
    if (ctype) {
        icon = g_content_type_get_icon(ctype);
        g_free(ctype);
    }
    if (!icon) icon = g_themed_icon_new("text-x-generic");
    else g_object_ref(icon);
    return icon;
}

void clear_file_list() {
    if (g.file_store) gtk_list_store_clear(g.file_store);
}

void update_window_title() {
    if (!g.window) return;
    std::string title = "mFile - " + (path_is_computer(g.current_path) ? std::string("Computer") : g.current_path);
    gtk_window_set_title(GTK_WINDOW(g.window), title.c_str());
}

std::vector<std::string> selected_paths() {
    std::vector<std::string> out;
    if (!g.file_view) return out;
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g.file_view));
    GList *rows = gtk_tree_selection_get_selected_rows(sel, nullptr);
    for (GList *l = rows; l; l = l->next) {
        GtkTreePath *tp = (GtkTreePath*)l->data;
        GtkTreeIter it;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(g.file_store), &it, tp)) {
            gchar *p = nullptr;
            gtk_tree_model_get(GTK_TREE_MODEL(g.file_store), &it, COL_PATH, &p, -1);
            if (p) { out.push_back(p); g_free(p); }
        }
    }
    g_list_free_full(rows, (GDestroyNotify)gtk_tree_path_free);
    return out;
}

void add_file_row(const std::string &name, const std::string &full, bool is_dir, guint64 size, time_t mtime) {
    if (!g.file_store) return;
    const gchar *filter = gtk_entry_get_text(GTK_ENTRY(g.search_entry));
    if (filter && *filter) {
        std::string fn = lower_ascii(name);
        std::string ff = lower_ascii(filter);
        if (fn.find(ff) == std::string::npos) return;
    }

    GtkTreeIter it;
    gtk_list_store_append(g.file_store, &it);
    GIcon *icon = icon_for_entry(full, is_dir);
    std::string mstr = format_time(mtime);
    std::string tstr = type_for_entry(full, is_dir);
    std::string sstr = format_size(size, is_dir);
    gtk_list_store_set(g.file_store, &it,
        COL_ICON, icon,
        COL_NAME, name.c_str(),
        COL_MODIFIED, mstr.c_str(),
        COL_TYPE, tstr.c_str(),
        COL_SIZE_TEXT, sstr.c_str(),
        COL_PATH, full.c_str(),
        COL_IS_DIR, is_dir ? TRUE : FALSE,
        COL_SIZE_BYTES, (guint64)size,
        COL_MTIME, (gint64)mtime,
        -1);
    if (icon) g_object_unref(icon);
}

void load_computer() {
    clear_file_list();
    // Root filesystem
    add_file_row("File System (/)", "/", true, 0, 0);

    // Home
    const char *home = g_get_home_dir();
    if (home && *home) add_file_row("Home", home, true, 0, 0);

    // Mounts from /proc/mounts (best-effort)
    FILE *f = std::fopen("/proc/mounts", "r");
    if (f) {
        char dev[512], mnt[1024], fstype[128];
        while (std::fscanf(f, "%511s %1023s %127s", dev, mnt, fstype) == 3) {
            // skip rest of line
            int ch;
            while ((ch = std::fgetc(f)) != '\n' && ch != EOF) {}
            std::string mp = mnt;
            if (mp == "/" || mp == "/proc" || mp == "/sys" || mp == "/dev" || mp == "/run") continue;
            if (mp.rfind("/proc/", 0) == 0 || mp.rfind("/sys/", 0) == 0 || mp.rfind("/dev/", 0) == 0 || mp.rfind("/run/user/", 0) == 0) continue;
            if (mp.rfind("/snap", 0) == 0) continue;
            if (is_dir_path(mp)) {
                std::string name = basename_of(mp);
                if (name.empty()) name = mp;
                add_file_row(name + " (" + mp + ")", mp, true, 0, 0);
            }
        }
        std::fclose(f);
    }

    char status[256];
    std::snprintf(status, sizeof(status), "Computer");
    set_status(status);
}

void load_directory(const std::string &path) {
    clear_file_list();
    if (path_is_computer(path)) {
        load_computer();
        return;
    }

    GDir *dir = nullptr;
    GError *err = nullptr;
    dir = g_dir_open(path.c_str(), 0, &err);
    if (!dir) {
        std::string msg = "Cannot open folder";
        if (err && err->message) { msg += ": "; msg += err->message; }
        if (err) g_error_free(err);
        set_status(msg);
        return;
    }

    const gchar *name;
    std::vector<std::string> names;
    while ((name = g_dir_read_name(dir)) != nullptr) {
        if (!g.show_hidden && name[0] == '.') continue;
        names.push_back(name);
    }
    g_dir_close(dir);

    int count = 0;
    for (size_t i = 0; i < names.size(); ++i) {
        std::string full = path_join(path, names[i]);
        struct stat st;
        if (lstat(full.c_str(), &st) != 0) continue;
        bool is_dir = S_ISDIR(st.st_mode);
        guint64 size = S_ISREG(st.st_mode) ? (guint64)st.st_size : 0;
        time_t mtime = st.st_mtime;
        add_file_row(names[i], full, is_dir, size, mtime);
        ++count;
    }

    char status[256];
    std::snprintf(status, sizeof(status), "%d items", count);
    set_status(status);
}

void navigate_to(const std::string &path, bool push_history = true) {
    std::string target = path;
    if (target.empty()) return;
    if (!path_is_computer(target)) {
        // normalize trailing slash (except root)
        while (target.size() > 1 && target.back() == '/') target.pop_back();
        if (!is_dir_path(target)) {
            set_status("Not a folder: " + target);
            return;
        }
    }

    if (push_history && !g.current_path.empty() && g.current_path != target) {
        g.back_history.push_back(g.current_path);
        g.forward_history.clear();
    }

    g.current_path = target;
    if (g.path_entry) {
        g.suppress_nav = true;
        gtk_entry_set_text(GTK_ENTRY(g.path_entry), path_is_computer(target) ? "computer:///" : target.c_str());
        g.suppress_nav = false;
    }
    update_window_title();
    load_directory(target);
}

void go_up() {
    if (path_is_computer(g.current_path)) return;
    if (g.current_path == "/") { navigate_to("computer:///"); return; }
    navigate_to(dirname_of(g.current_path));
}

void go_back() {
    if (g.back_history.empty()) return;
    std::string target = g.back_history.back();
    g.back_history.pop_back();
    g.forward_history.push_back(g.current_path);
    navigate_to(target, false);
}

void go_forward() {
    if (g.forward_history.empty()) return;
    std::string target = g.forward_history.back();
    g.forward_history.pop_back();
    g.back_history.push_back(g.current_path);
    navigate_to(target, false);
}

void open_path(const std::string &path) {
    if (path.empty()) return;
    if (path_is_computer(path) || is_dir_path(path)) {
        navigate_to(path);
        return;
    }
    gchar *q = g_shell_quote(path.c_str());
    std::string cmd = std::string("xdg-open ") + q + " >/dev/null 2>&1 &";
    g_free(q);
    (void)std::system(cmd.c_str());
}

void on_row_activated(GtkTreeView *, GtkTreePath *tp, GtkTreeViewColumn *, gpointer) {
    GtkTreeIter it;
    if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(g.file_store), &it, tp)) return;
    gchar *path = nullptr;
    gtk_tree_model_get(GTK_TREE_MODEL(g.file_store), &it, COL_PATH, &path, -1);
    if (path) { open_path(path); g_free(path); }
}

void on_sidebar_cursor_changed(GtkTreeSelection *, gpointer) {
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g.side_view));
    GtkTreeIter it; GtkTreeModel *m;
    if (!gtk_tree_selection_get_selected(sel, &m, &it)) return;
    gchar *path = nullptr; gboolean is_comp = FALSE;
    gtk_tree_model_get(m, &it, SCOL_PATH, &path, SCOL_IS_COMPUTER, &is_comp, -1);
    if (path) {
        navigate_to(is_comp ? "computer:///" : std::string(path));
        g_free(path);
    } else if (is_comp) {
        navigate_to("computer:///");
    }
}

void on_path_activate(GtkEntry *, gpointer) {
    if (g.suppress_nav) return;
    const gchar *t = gtk_entry_get_text(GTK_ENTRY(g.path_entry));
    if (!t) return;
    std::string p = t;
    if (p == "computer:///" || p == "Computer" || p == "computer") navigate_to("computer:///");
    else navigate_to(p);
}

void on_search_changed(GtkEditable *, gpointer) {
    load_directory(g.current_path);
}

void on_show_hidden_toggled(GtkToggleButton *btn, gpointer) {
    g.show_hidden = gtk_toggle_button_get_active(btn);
    load_directory(g.current_path);
}

void on_back(GtkButton*, gpointer){ go_back(); }
void on_forward(GtkButton*, gpointer){ go_forward(); }
void on_up(GtkButton*, gpointer){ go_up(); }
void on_refresh(GtkButton*, gpointer){ load_directory(g.current_path); }

void do_new_folder(GtkWidget*, gpointer) {
    if (path_is_computer(g.current_path)) return;
    GtkWidget *dlg = gtk_dialog_new_with_buttons("New Folder", GTK_WINDOW(g.window),
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        "_Cancel", GTK_RESPONSE_CANCEL, "_Create", GTK_RESPONSE_OK, nullptr);
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "New Folder");
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Folder name:"), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 4);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(entry));
        if (name && *name) {
            std::string full = path_join(g.current_path, name);
            if (g_mkdir_with_parents(full.c_str(), 0755) != 0) {
                set_status(std::string("Failed to create folder: ") + g_strerror(errno));
            } else {
                load_directory(g.current_path);
            }
        }
    }
    gtk_widget_destroy(dlg);
}

void do_delete(GtkWidget*, gpointer) {
    auto sel = selected_paths();
    if (sel.empty()) return;
    GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(g.window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
        "Delete %zu item(s)?", sel.size());
    gtk_dialog_add_buttons(GTK_DIALOG(dlg), "_Cancel", GTK_RESPONSE_CANCEL, "_Delete", GTK_RESPONSE_OK, nullptr);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), "Items will be deleted (moved to Trash if available).");
    int resp = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    if (resp != GTK_RESPONSE_OK) return;

    for (auto &p : sel) {
        gchar *q = g_shell_quote(p.c_str());
        std::string cmd = std::string("gio trash ") + q + " >/dev/null 2>&1";
        g_free(q);
        int rc = std::system(cmd.c_str());
        if (rc != 0) {
            // fallback: remove
            if (is_dir_path(p)) {
                gchar *q2 = g_shell_quote(p.c_str());
                std::string cmd3 = std::string("rm -rf ") + q2;
                g_free(q2);
                (void)std::system(cmd3.c_str());
            } else {
                std::remove(p.c_str());
            }
        }
    }
    load_directory(g.current_path);
}

void do_rename(GtkWidget*, gpointer) {
    auto sel = selected_paths();
    if (sel.size() != 1) return;
    std::string oldp = sel[0];
    std::string base = basename_of(oldp);
    GtkWidget *dlg = gtk_dialog_new_with_buttons("Rename", GTK_WINDOW(g.window),
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        "_Cancel", GTK_RESPONSE_CANCEL, "_Rename", GTK_RESPONSE_OK, nullptr);
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), base.c_str());
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("New name:"), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 4);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(entry));
        if (name && *name) {
            std::string newp = path_join(dirname_of(oldp), name);
            if (std::rename(oldp.c_str(), newp.c_str()) != 0) {
                set_status(std::string("Rename failed: ") + g_strerror(errno));
            } else {
                load_directory(g.current_path);
            }
        }
    }
    gtk_widget_destroy(dlg);
}

void do_copy_path(GtkWidget*, gpointer) {
    auto sel = selected_paths();
    if (sel.empty()) return;
    std::string text;
    for (size_t i = 0; i < sel.size(); ++i) { if (i) text += "\n"; text += sel[i]; }
    GtkClipboard *cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, text.c_str(), -1);
    set_status("Path copied");
}

void do_open_selected(GtkWidget*, gpointer) {
    auto sel = selected_paths();
    for (auto &p : sel) open_path(p);
}

gboolean on_file_button_press(GtkWidget *view, GdkEventButton *ev, gpointer) {
    if (ev->type == GDK_BUTTON_PRESS && ev->button == 3) {
        GtkTreePath *path = nullptr;
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view), (gint)ev->x, (gint)ev->y, &path, nullptr, nullptr, nullptr)) {
            GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
            if (!gtk_tree_selection_path_is_selected(sel, path)) {
                gtk_tree_selection_unselect_all(sel);
                gtk_tree_selection_select_path(sel, path);
            }
            gtk_tree_path_free(path);
        } else {
            gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)));
        }

        GtkWidget *menu = gtk_menu_new();
        auto add_item = [&](const char *label, GCallback cb) {
            GtkWidget *it = gtk_menu_item_new_with_label(label);
            g_signal_connect(it, "activate", cb, nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), it);
        };
        add_item("Open", G_CALLBACK(do_open_selected));
        add_item("Copy Path", G_CALLBACK(do_copy_path));
        add_item("Rename", G_CALLBACK(do_rename));
        add_item("Delete", G_CALLBACK(do_delete));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
        add_item("New Folder", G_CALLBACK(do_new_folder));
        add_item("Refresh", G_CALLBACK(on_refresh));
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)ev);
        return TRUE;
    }
    return FALSE;
}

void add_sidebar_place(const char *label, const char *icon_name, const std::string &path, bool is_computer=false) {
    GtkTreeIter it;
    gtk_tree_store_append(g.side_store, &it, nullptr);
    GIcon *icon = g_themed_icon_new(icon_name);
    gtk_tree_store_set(g.side_store, &it,
        SCOL_ICON, icon,
        SCOL_NAME, label,
        SCOL_PATH, path.c_str(),
        SCOL_IS_COMPUTER, is_computer ? TRUE : FALSE,
        -1);
    if (icon) g_object_unref(icon);
}

void build_sidebar() {
    g.side_store = gtk_tree_store_new(SCOL_COUNT, G_TYPE_ICON, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
    g.side_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(g.side_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(g.side_view), FALSE);

    GtkCellRenderer *pr = gtk_cell_renderer_pixbuf_new();
    GtkCellRenderer *tr = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col = gtk_tree_view_column_new();
    gtk_tree_view_column_pack_start(col, pr, FALSE);
    gtk_tree_view_column_pack_start(col, tr, TRUE);
    gtk_tree_view_column_add_attribute(col, pr, "gicon", SCOL_ICON);
    gtk_tree_view_column_add_attribute(col, tr, "text", SCOL_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g.side_view), col);

    add_sidebar_place("Computer", "computer", "computer:///", true);
    const char *home = g_get_home_dir();
    if (home) add_sidebar_place("Home", "user-home", home);
    const char *desk = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    if (desk) add_sidebar_place("Desktop", "user-desktop", desk);
    const char *docs = g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS);
    if (docs) add_sidebar_place("Documents", "folder-documents", docs);
    const char *dl = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (dl) add_sidebar_place("Downloads", "folder-download", dl);
    const char *mus = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
    if (mus) add_sidebar_place("Music", "folder-music", mus);
    const char *pic = g_get_user_special_dir(G_USER_DIRECTORY_PICTURES);
    if (pic) add_sidebar_place("Pictures", "folder-pictures", pic);
    const char *vid = g_get_user_special_dir(G_USER_DIRECTORY_VIDEOS);
    if (vid) add_sidebar_place("Videos", "folder-videos", vid);
    add_sidebar_place("File System", "drive-harddisk", "/");

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g.side_view));
    g_signal_connect(sel, "changed", G_CALLBACK(on_sidebar_cursor_changed), nullptr);
}

void build_file_view() {
    g.file_store = gtk_list_store_new(COL_COUNT,
        G_TYPE_ICON, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_UINT64, G_TYPE_INT64);
    g.file_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(g.file_store));
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(g.file_view), TRUE);

    auto add_col = [&](const char *title, int col_idx, int sort_col, bool icon_first=false) {
        GtkTreeViewColumn *col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, title);
        gtk_tree_view_column_set_resizable(col, TRUE);
        gtk_tree_view_column_set_sort_column_id(col, sort_col);
        if (icon_first) {
            GtkCellRenderer *pr = gtk_cell_renderer_pixbuf_new();
            gtk_tree_view_column_pack_start(col, pr, FALSE);
            gtk_tree_view_column_add_attribute(col, pr, "gicon", COL_ICON);
        }
        GtkCellRenderer *tr = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, tr, TRUE);
        gtk_tree_view_column_add_attribute(col, tr, "text", col_idx);
        gtk_tree_view_append_column(GTK_TREE_VIEW(g.file_view), col);
    };

    add_col("Name", COL_NAME, COL_NAME, true);
    add_col("Date modified", COL_MODIFIED, COL_MTIME, false);
    add_col("Type", COL_TYPE, COL_TYPE, false);
    add_col("Size", COL_SIZE_TEXT, COL_SIZE_BYTES, false);

    gtk_tree_view_column_set_expand(gtk_tree_view_get_column(GTK_TREE_VIEW(g.file_view), 0), TRUE);

    g_signal_connect(g.file_view, "row-activated", G_CALLBACK(on_row_activated), nullptr);
    g_signal_connect(g.file_view, "button-press-event", G_CALLBACK(on_file_button_press), nullptr);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g.file_view));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
}

/* (UI constructed in main for simple gtk_init/gtk_main flow) */

} // namespace

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    g.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(g.window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
    gtk_window_set_default_size(GTK_WINDOW(g.window), 1100, 700);
    gtk_window_set_title(GTK_WINDOW(g.window), "mFile");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(g.window), vbox);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 4);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    auto mk_btn = [&](const char *label, GCallback cb) {
        GtkWidget *b = gtk_button_new_with_label(label);
        g_signal_connect(b, "clicked", cb, nullptr);
        gtk_box_pack_start(GTK_BOX(toolbar), b, FALSE, FALSE, 0);
        return b;
    };
    mk_btn("Back", G_CALLBACK(on_back));
    mk_btn("Forward", G_CALLBACK(on_forward));
    mk_btn("Up", G_CALLBACK(on_up));
    mk_btn("Refresh", G_CALLBACK(on_refresh));

    g.path_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(toolbar), g.path_entry, TRUE, TRUE, 4);
    g_signal_connect(g.path_entry, "activate", G_CALLBACK(on_path_activate), nullptr);

    gtk_box_pack_start(GTK_BOX(toolbar), gtk_label_new("Search:"), FALSE, FALSE, 0);
    g.search_entry = gtk_search_entry_new();
    gtk_widget_set_size_request(g.search_entry, 220, -1);
    gtk_box_pack_start(GTK_BOX(toolbar), g.search_entry, FALSE, FALSE, 0);
    g_signal_connect(g.search_entry, "search-changed", G_CALLBACK(on_search_changed), nullptr);

    g.show_hidden_check = gtk_check_button_new_with_label("Show hidden");
    gtk_box_pack_start(GTK_BOX(toolbar), g.show_hidden_check, FALSE, FALSE, 0);
    g_signal_connect(g.show_hidden_check, "toggled", G_CALLBACK(on_show_hidden_toggled), nullptr);

    GtkWidget *newbtn = gtk_button_new_with_label("New Folder");
    g_signal_connect(newbtn, "clicked", G_CALLBACK(do_new_folder), nullptr);
    gtk_box_pack_start(GTK_BOX(toolbar), newbtn, FALSE, FALSE, 0);

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 240);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);

    build_sidebar();
    GtkWidget *side_scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(side_scroll), g.side_view);
    gtk_paned_add1(GTK_PANED(paned), side_scroll);

    build_file_view();
    GtkWidget *file_scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(file_scroll), g.file_view);
    gtk_paned_add2(GTK_PANED(paned), file_scroll);

    g.status_label = gtk_label_new("Ready");
    gtk_label_set_xalign(GTK_LABEL(g.status_label), 0.0f);
    gtk_widget_set_margin_start(g.status_label, 6);
    gtk_widget_set_margin_end(g.status_label, 6);
    gtk_widget_set_margin_top(g.status_label, 2);
    gtk_widget_set_margin_bottom(g.status_label, 2);
    gtk_box_pack_start(GTK_BOX(vbox), g.status_label, FALSE, FALSE, 0);

    gtk_widget_show_all(g.window);

    navigate_to("computer:///", false);

    gtk_main();
    return 0;
}
