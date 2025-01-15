#include <gtk/gtk.h>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <sstream>

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    if (userp == nullptr || contents == nullptr) {
        return 0;
    }
    try {
        userp->append(static_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    } catch (...) {
        return 0;
    }
}

std::string sendPostRequest(const std::string& url, const std::string& question) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error initializing CURL.";
    }

    std::string response;
    struct curl_slist* headers = nullptr;

    try {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!headers) {
            throw std::runtime_error("Error creating headers.");
        }

        std::ostringstream oss;
        oss << "{\"question\": \"" << question << "\"}";
        std::string payload = oss.str();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "Error: " + std::string(curl_easy_strerror(res));
        }
    } catch (const std::exception& e) {
        response = std::string("Exception: ") + e.what();
    } catch (...) {
        response = "Exception: Unknown error.";
    }

    if (headers) {
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);

    if (response.empty()) {
        response = "Error: Empty response from server.";
    }

    return response;
}

GtkWidget* create_message_bubble(const std::string& text, bool is_user) {
    GtkWidget* bubble = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget* label = gtk_label_new(text.c_str());

    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 50);

    gtk_widget_set_halign(label, is_user ? GTK_ALIGN_END : GTK_ALIGN_START);
    gtk_widget_set_margin_top(label, 5);
    gtk_widget_set_margin_bottom(label, 5);
    gtk_widget_set_margin_start(label, 10);
    gtk_widget_set_margin_end(label, 10);

    GtkCssProvider* css_provider = gtk_css_provider_new();
    const char* css_data = is_user
                                ? "label { background: #002922; color: white; border-radius: 10px; padding: 10px; border: 2px solid #001810; }"
                                : "label { background: #f5f5dc; color: black; border-radius: 10px; padding: 10px; border: 2px solid #e0e0d0; }";
    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    GtkStyleContext* context = gtk_widget_get_style_context(label);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_container_add(GTK_CONTAINER(bubble), label);
    return bubble;
}

static void on_send_clicked(GtkWidget* button, gpointer data) {
    GtkWidget** widgets = (GtkWidget**)data;
    GtkEntry* entry = GTK_ENTRY(widgets[0]);
    GtkWidget* box = GTK_WIDGET(widgets[1]);

    const gchar* user_input = gtk_entry_get_text(entry);
    if (g_strcmp0(user_input, "") == 0) {
        return;
    }

    std::string server_url = "http://192.168.1.4:5000/chat";
    std::string response = sendPostRequest(server_url, user_input);

    GtkWidget* user_bubble = create_message_bubble("You: " + std::string(user_input), true);
    gtk_box_pack_start(GTK_BOX(box), user_bubble, FALSE, FALSE, 0);

    GtkWidget* bot_bubble = create_message_bubble("Bot: " + response, false);
    gtk_box_pack_start(GTK_BOX(box), bot_bubble, FALSE, FALSE, 0);

    gtk_entry_set_text(entry, "");
    gtk_widget_show_all(GTK_WIDGET(box));
}

static void on_help_clicked(GtkWidget* button, gpointer data) {
    GtkWidget* box = GTK_WIDGET(data);
    GtkWidget* help_bubble = create_message_bubble("Help: Type a question and press Enter or click Send.", false);
    gtk_box_pack_start(GTK_BOX(box), help_bubble, FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(box));
}

static void toggle_dark_mode(GtkWidget* button, gpointer data) {
    GtkCssProvider* provider = GTK_CSS_PROVIDER(data);
    static bool dark_mode = false;
    dark_mode = !dark_mode;

    const char* css_data = dark_mode ? "* { background-color: #002922; color: #ffffff; } headerbar { background-color: #001810; color: #ffffff; } button, entry { background-color: #00332a; color: #ffffff; border-radius: 5px; }"
                                     : "* { background-color: #f5f5dc; color: #000000; } headerbar { background-color: #e0e0d0; color: #000000; } button, entry { background-color: #e0e0d0; color: #000000; border-radius: 5px; }";

    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "BEC-MM AI Interface");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

    GtkWidget* chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(scroll_window), chat_box);

    GtkWidget* entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    GtkWidget* hbox_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 0);

    GtkWidget* send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(hbox_buttons), send_button, TRUE, TRUE, 0);

    GtkWidget* help_button = gtk_button_new_with_label("Help");
    gtk_box_pack_start(GTK_BOX(hbox_buttons), help_button, TRUE, TRUE, 0);

    GtkWidget* dark_mode_button = gtk_button_new_with_label("Dark Mode");
    gtk_box_pack_start(GTK_BOX(hbox_buttons), dark_mode_button, TRUE, TRUE, 0);

    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget* widgets[2] = {entry, chat_box};
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_clicked), widgets);
    g_signal_connect(help_button, "clicked", G_CALLBACK(on_help_clicked), chat_box);
    g_signal_connect(dark_mode_button, "clicked", G_CALLBACK(toggle_dark_mode), css_provider);
    g_signal_connect(entry, "activate", G_CALLBACK(on_send_clicked), widgets);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}