#include <limits.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h> 
#define MAX_CITIES 50
#define INF INT_MAX
typedef struct {
    int distance;
    int time;
} Edge;

typedef struct {
    char cityNames[MAX_CITIES][50];
    Edge adjMatrix[MAX_CITIES][MAX_CITIES];
    int numVertices;
} Graph;
typedef struct {
    Graph graph;
    gboolean graphLoaded;
    GtkWidget *sourceEntry;
    GtkWidget *destEntry;
    GtkWidget *resultTextView;
    GtkWidget *window;
    GtkWidget *distRadio; 
    GtkWidget *timeRadio;
} AppData;
typedef struct {
    GtkWidget *userEntry;
    GtkWidget *passEntry;
    GtkWidget *loginWindow;
    AppData *appData;
    gulong destroy_handler_id; 
} LoginData;

GtkWidget* create_main_application_window(AppData *data);
void show_error_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "Error: %s",
        message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_info_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
void set_result_text(GtkTextView *textView, const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(textView);
    gtk_text_buffer_set_text(buffer, text, -1);
}
gboolean loadGraphFromFile(Graph *graph, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return FALSE;
    int n, m;
    char line[256];
    char city1[50], city2[50];
    int distance, time; 
    
    if (!fgets(line, sizeof(line), file) || sscanf(line, "%d", &n) != 1) {
        fclose(file); return FALSE;
    }
    graph->numVertices = n;
    if (n > MAX_CITIES) {
        fclose(file); return FALSE;
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            graph->adjMatrix[i][j].distance = (i == j) ? 0 : INF;
            graph->adjMatrix[i][j].time = (i == j) ? 0 : INF;
        }
    }
    for (int i = 0; i < n; i++) {
        if (!fgets(line, sizeof(line), file)) {
            fclose(file); return FALSE;
        }
        line[strcspn(line, "\r\n")] = 0;
        
        char* city_name_key = line;
        if (strncmp(city_name_key, "\xEF\xBB\xBF", 3) == 0) {
            city_name_key += 3; 
        }
        strcpy(graph->cityNames[i], city_name_key);
    }
    if (!fgets(line, sizeof(line), file) || sscanf(line, "%d", &m) != 1) {
        fclose(file); return FALSE;
    }

    for (int i = 0; i < m; i++) {
        if (!fgets(line, sizeof(line), file)) {
            if(feof(file)) break;
            fclose(file); return FALSE; 
        }
        
        char *token = strtok(line, ",\n\r");
        if (!token) continue;
        strcpy(city1, token);
        
        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        strcpy(city2, token);
        
        token = strtok(NULL, ",\n\r");
        if (!token || sscanf(token, "%d", &distance) != 1) continue;
        
        token = strtok(NULL, ",\n\r");
        if (!token || sscanf(token, "%d", &time) != 1) continue;
        
        int idx1 = -1, idx2 = -1;
        for (int j = 0; j < n; j++) {
            if (strcasecmp(graph->cityNames[j], city1) == 0) idx1 = j;
            if (strcasecmp(graph->cityNames[j], city2) == 0) idx2 = j;
        }
        if (idx1 != -1 && idx2 != -1) {
            graph->adjMatrix[idx1][idx2].distance = distance;
            graph->adjMatrix[idx1][idx2].time = time;
            graph->adjMatrix[idx2][idx1].distance = distance;
            graph->adjMatrix[idx2][idx1].time = time;
        }
    }
    fclose(file);
    return TRUE;
}
int findCityIndex(const Graph *graph, const char *name) {
    for (int i = 0; i < graph->numVertices; i++) {
        if (strcasecmp(graph->cityNames[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void dijkstra(const Graph *graph, int startNodeIndex, int distance[MAX_CITIES], int parent[MAX_CITIES], const char* mode) {
    int n = graph->numVertices;
    int visited[MAX_CITIES];
    for (int i = 0; i < n; i++) {
        distance[i] = INF;
        parent[i] = -1;
        visited[i] = 0;
    }
    distance[startNodeIndex] = 0;
    for (int count = 0; count < n - 1; count++) {
        int minDistance = INF;
        int u = -1;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && distance[i] < minDistance) {
                minDistance = distance[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = 1;
        for (int v = 0; v < n; v++) {
            int weight;
            if (strcmp(mode, "time") == 0) {
                weight = graph->adjMatrix[u][v].time;
            } else {
                weight = graph->adjMatrix[u][v].distance;
            }
            
            if (!visited[v] && weight != INF && distance[u] != INF && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                parent[v] = u;
            }
        }
    }
}
void getPathRecursive(const Graph *graph, const int parent[MAX_CITIES], int j, char *buffer) {
    if (parent[j] == -1) {
        sprintf(buffer + strlen(buffer), "%s", graph->cityNames[j]);
        return;
    }
    getPathRecursive(graph, parent, parent[j], buffer);
    sprintf(buffer + strlen(buffer), " -> %s", graph->cityNames[j]);
}

void getPathString(const Graph *graph, const int parent[MAX_CITIES], int destIndex, char *buffer) {
    buffer[0] = '\0';
    getPathRecursive(graph, parent, destIndex, buffer);
}

void getFloydWarshallString(const Graph *graph, char *buffer, const char* mode) {
    int n = graph->numVertices;
    int dist[MAX_CITIES][MAX_CITIES];
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                dist[i][j] = 0;
            } else if (strcmp(mode, "time") == 0) {
                dist[i][j] = graph->adjMatrix[i][j].time;
            } else {
                dist[i][j] = graph->adjMatrix[i][j].distance;
            }
        }
    }
    
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
    buffer[0] = '\0';
    if (strcmp(mode, "time") == 0) {
        sprintf(buffer, "All-Pairs Fastest Time Matrix (in minutes):\n\n");
    } else {
        sprintf(buffer, "All-Pairs Shortest Distance Matrix (in miles):\n\n");
    }
    
    sprintf(buffer + strlen(buffer), "%-10s", "");
    for(int i=0; i<n; i++) {
        sprintf(buffer + strlen(buffer), "%-7.7s ", graph->cityNames[i]);
    }
    sprintf(buffer + strlen(buffer), "\n");
    
    for (int i = 0; i < n; i++) {
        sprintf(buffer + strlen(buffer), "%-10.10s", graph->cityNames[i]);
        for (int j = 0; j < n; j++) {
            if (dist[i][j] == INF) {
                sprintf(buffer + strlen(buffer), "%-7s ", "INF");
            } else {
                sprintf(buffer + strlen(buffer), "%-7d ", dist[i][j]);
            }
        }
        sprintf(buffer + strlen(buffer), "\n");
    }
}

static void on_load_map_clicked(GtkButton *button, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Map File",
        GTK_WINDOW(data->window),
        action,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);
        
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (loadGraphFromFile(&data->graph, filename)) {
            data->graphLoaded = TRUE;
            show_info_dialog(GTK_WINDOW(data->window), "Map loaded successfully!");
            set_result_text(GTK_TEXT_VIEW(data->resultTextView), "Map loaded. Ready to find routes.");
        } else {
            data->graphLoaded = FALSE;
            show_error_dialog(GTK_WINDOW(data->window), "Failed to load or parse map file.\nCheck file format.");
            set_result_text(GTK_TEXT_VIEW(data->resultTextView), "Error: Could not load map.");
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_find_path_clicked(GtkButton *button, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    if (!data->graphLoaded) {
        show_error_dialog(GTK_WINDOW(data->window), "Please load a map file first.");
        return;
    }
    const char *sourceCity = gtk_entry_get_text(GTK_ENTRY(data->sourceEntry));
    const char *destCity = gtk_entry_get_text(GTK_ENTRY(data->destEntry));
    if (strlen(sourceCity) == 0 || strlen(destCity) == 0) {
        show_error_dialog(GTK_WINDOW(data->window), "Please enter both a source and a destination city.");
        return;
    }
    int sourceIndex = findCityIndex(&data->graph, sourceCity);
    int destIndex = findCityIndex(&data->graph, destCity);
    if (sourceIndex == -1 || destIndex == -1) {
        show_error_dialog(GTK_WINDOW(data->window), "One or both cities not found in the map.");
        return;
    }

    const char* mode;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->timeRadio))) {
        mode = "time";
    } else {
        mode = "distance";
    }

    int distance[MAX_CITIES];
    int parent[MAX_CITIES];
    dijkstra(&data->graph, sourceIndex, distance, parent, mode);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->resultTextView));
    gtk_text_buffer_set_text(buffer, "", -1); 
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    char temp[512]; 
    if (distance[destIndex] == INF) {

        gtk_text_buffer_insert(buffer, &iter, "No route found between ", -1);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, data->graph.cityNames[sourceIndex], -1, "bold", NULL);
        gtk_text_buffer_insert(buffer, &iter, " and ", -1);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, data->graph.cityNames[destIndex], -1, "bold", NULL);
    } else {
        char path_buffer[4096];
        getPathString(&data->graph, parent, destIndex, path_buffer);

        int total_dist = 0;
        int total_time = 0;
        int curr = destIndex;
        while(parent[curr] != -1) {
            int prev = parent[curr];
            total_dist += data->graph.adjMatrix[prev][curr].distance;
            total_time += data->graph.adjMatrix[prev][curr].time;
            curr = prev;
        }
        
        int hours = total_time / 60;
        int minutes = total_time % 60;
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "--- Route Optimization Results ---\n", -1, "heading", NULL);
        if (strcmp(mode, "time") == 0) {
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "(Optimized for Fastest Time)\n\n", -1, "bold", NULL);
        } else {
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "(Optimized for Shortest Distance)\n\n", -1, "bold", NULL);
        }
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "From: ", -1, "bold", NULL);
        sprintf(temp, "%s\n", data->graph.cityNames[sourceIndex]);
        gtk_text_buffer_insert(buffer, &iter, temp, -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "To: ", -1, "bold", NULL);
        sprintf(temp, "%s\n\n", data->graph.cityNames[destIndex]);
        gtk_text_buffer_insert(buffer, &iter, temp, -1);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "Total Distance: ", -1, "bold", NULL);
        sprintf(temp, "%d Miles\n", total_dist);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, temp, -1, (strcmp(mode, "distance") == 0 ? "highlight" : "normal"), NULL);
        
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "Total Time: ", -1, "bold", NULL);
        sprintf(temp, "%d hours, %d minutes\n\n", hours, minutes);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, temp, -1, (strcmp(mode, "time") == 0 ? "highlight" : "normal"), NULL);

        gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, "Optimal Route: ", -1, "bold", NULL);
        gtk_text_buffer_insert(buffer, &iter, path_buffer, -1);
    }
}
static void on_show_all_pairs_clicked(GtkButton *button, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    if (!data->graphLoaded) {
        show_error_dialog(GTK_WINDOW(data->window), "Please load a map file first.");
        return;
    }

    const char* mode;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->timeRadio))) {
        mode = "time";
    } else {
        mode = "distance";
    }
    char *matrix_buffer = (char *)malloc(MAX_CITIES * 10 + MAX_CITIES * MAX_CITIES * 8 + 1024);
    if (!matrix_buffer) {
        show_error_dialog(GTK_WINDOW(data->window), "Failed to allocate memory for matrix display.");
        return;
    }
    
    getFloydWarshallString(&data->graph, matrix_buffer, mode);
    
    set_result_text(GTK_TEXT_VIEW(data->resultTextView), matrix_buffer);
    free(matrix_buffer);
}
static void on_list_cities_clicked(GtkButton *button, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    if (!data->graphLoaded) {
        show_error_dialog(GTK_WINDOW(data->window), "Please load a map file first.");
        return;
    }
    char *cities_buffer = (char *)malloc(data->graph.numVertices * 52 + 100); 
    if (!cities_buffer) {
        show_error_dialog(GTK_WINDOW(data->window), "Failed to allocate memory for city list.");
        return;
    }
    char *buffer_ptr = cities_buffer;
    buffer_ptr += sprintf(buffer_ptr, "--- All %d Loaded Cities ---\n\n", data->graph.numVertices);
    for (int i = 0; i < data->graph.numVertices; i++) {
        buffer_ptr += sprintf(buffer_ptr, "%s\n", data->graph.cityNames[i]);
    }
    set_result_text(GTK_TEXT_VIEW(data->resultTextView), cities_buffer);
    free(cities_buffer);
}
static void on_list_routes_clicked(GtkButton *button, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    if (!data->graphLoaded) {
        show_error_dialog(GTK_WINDOW(data->window), "Please load a map file first.");
        return;
    }
    
    long buffer_size = (data->graph.numVertices * (data->graph.numVertices - 1) / 2) * 120 + 1024;
    if (buffer_size < 1024) buffer_size = 1024;

    char *routes_buffer = (char *)malloc(buffer_size); 
    if (!routes_buffer) {
        show_error_dialog(GTK_WINDOW(data->window), "Failed to allocate memory for route list.");
        return;
    }

    char *buffer_ptr = routes_buffer;
    buffer_ptr[0] = '\0'; 
    int route_count = 0;
    
    for (int i = 0; i < data->graph.numVertices; i++) {
        for (int j = i + 1; j < data->graph.numVertices; j++) {
            if (data->graph.adjMatrix[i][j].distance != INF) {
                sprintf(buffer_ptr + strlen(buffer_ptr), "%s <-> %s (%d Miles, %d min)\n", 
                        data->graph.cityNames[i], 
                        data->graph.cityNames[j],
                        data->graph.adjMatrix[i][j].distance,
                        data->graph.adjMatrix[i][j].time);
                route_count++;
            }
        }
    }

    char *final_output = (char *)malloc(strlen(routes_buffer) + 200);
    if (!final_output) {
        show_error_dialog(GTK_WINDOW(data->window), "Failed to allocate final buffer.");
        free(routes_buffer);
        return;
    }
    
    sprintf(final_output, "--- All %d Available Direct Routes ---\n\n%s", route_count, routes_buffer);

    set_result_text(GTK_TEXT_VIEW(data->resultTextView), final_output);
    
    free(routes_buffer); 
    free(final_output);  
}


gboolean check_credentials(const char *username, const char *password) {
    if (g_strcmp0(username, "admin") == 0 && g_strcmp0(password, "pass123") == 0) {
        return TRUE;
    }
    if (g_strcmp0(username, "user") == 0 && g_strcmp0(password, "1234") == 0) {
        return TRUE;
    }
    return FALSE;
}

static void on_login_clicked(GtkButton *button, gpointer user_data) {
    LoginData *loginData = (LoginData *)user_data;
    const char *user = gtk_entry_get_text(GTK_ENTRY(loginData->userEntry));
    const char *pass = gtk_entry_get_text(GTK_ENTRY(loginData->passEntry));
    
    if (check_credentials(user, pass)) {
        GtkWidget *main_window = create_main_application_window(loginData->appData);
        gtk_widget_show_all(main_window);
        g_signal_handler_disconnect(loginData->loginWindow, loginData->destroy_handler_id);
        gtk_widget_destroy(loginData->loginWindow);
        g_free(loginData); 
    } else {
        show_error_dialog(GTK_WINDOW(loginData->loginWindow), "Invalid username or password.");
        gtk_entry_set_text(GTK_ENTRY(loginData->passEntry), "");
    }
}

static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    GtkWidget *button_to_activate = GTK_WIDGET(user_data);
    gtk_widget_activate(button_to_activate);
}

GtkWidget* create_login_window(AppData *appData) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Login - Smart Travel Planner");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    
    LoginData *loginData = g_new0(LoginData, 1);
    loginData->destroy_handler_id = g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(window), grid);
    
    loginData->appData = appData;
    loginData->loginWindow = window;
    
    GtkWidget *userLabel = gtk_label_new("Username:");
    loginData->userEntry = gtk_entry_new();
    gtk_widget_set_hexpand(loginData->userEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), userLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), loginData->userEntry, 1, 0, 1, 1);
    
    GtkWidget *passLabel = gtk_label_new("Password:");
    loginData->passEntry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(loginData->passEntry), FALSE); 
    gtk_widget_set_hexpand(loginData->passEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), passLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), loginData->passEntry, 1, 1, 1, 1);
    
    GtkWidget *loginButton = gtk_button_new_with_label("Login");
    g_signal_connect(loginButton, "clicked", G_CALLBACK(on_login_clicked), loginData);
    gtk_grid_attach(GTK_GRID(grid), loginButton, 0, 2, 2, 1);
    gtk_widget_set_can_default(loginButton, TRUE);
    gtk_widget_grab_default(loginButton);
    
    g_signal_connect(loginData->userEntry, "activate", G_CALLBACK(on_entry_activate), loginButton);
    g_signal_connect(loginData->passEntry, "activate", G_CALLBACK(on_entry_activate), loginButton);
    
    return window;
}

GtkWidget* create_main_application_window(AppData *data) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Smart Travel Route Planner");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    data->window = window;
    
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Smart Travel Route Planner");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    
    GtkWidget *loadButton = gtk_button_new_with_label("Load Map File...");
    gtk_button_set_image(GTK_BUTTON(loadButton),
        gtk_image_new_from_icon_name("document-open-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), loadButton);
    g_signal_connect(loadButton, "clicked", G_CALLBACK(on_load_map_clicked), data);
    
    GtkWidget *controlsFrame = gtk_frame_new("Route Finder");
    gtk_frame_set_label_align(GTK_FRAME(controlsFrame), 0.02, 0.5);
    gtk_container_set_border_width(GTK_CONTAINER(controlsFrame), 5);
    gtk_box_pack_start(GTK_BOX(vbox), controlsFrame, FALSE, FALSE, 5);
    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 5);
    gtk_container_add(GTK_CONTAINER(controlsFrame), grid);
    
    GtkWidget *sourceLabel = gtk_label_new("Source City:");
    gtk_widget_set_halign(sourceLabel, GTK_ALIGN_END);
    data->sourceEntry = gtk_entry_new();
    gtk_widget_set_hexpand(data->sourceEntry, TRUE);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(data->sourceEntry),
        GTK_ENTRY_ICON_PRIMARY, "location-symbolic");
    gtk_grid_attach(GTK_GRID(grid), sourceLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), data->sourceEntry, 1, 0, 1, 1);
    
    GtkWidget *destLabel = gtk_label_new("Destination City:");
    gtk_widget_set_halign(destLabel, GTK_ALIGN_END);
    data->destEntry = gtk_entry_new();
    gtk_widget_set_hexpand(data->destEntry, TRUE);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(data->destEntry),
        GTK_ENTRY_ICON_PRIMARY, "flag-symbolic");
    gtk_grid_attach(GTK_GRID(grid), destLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), data->destEntry, 1, 1, 1, 1);

    GtkWidget *modeLabel = gtk_label_new("Optimize for:");
    gtk_widget_set_halign(modeLabel, GTK_ALIGN_END);
    data->distRadio = gtk_radio_button_new_with_label(NULL, "Shortest Distance (Cost)");
    data->timeRadio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(data->distRadio), "Fastest Time");
    
    GtkWidget *radioBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(radioBox), data->distRadio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radioBox), data->timeRadio, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), modeLabel, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radioBox, 1, 2, 1, 1);

    GtkWidget *hbox_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(grid), hbox_buttons, 0, 3, 2, 1); 
    
    GtkWidget *findPathButton = gtk_button_new_with_label("Find Shortest Path");
    gtk_button_set_image(GTK_BUTTON(findPathButton),
        gtk_image_new_from_icon_name("system-search-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbox_buttons), findPathButton, TRUE, TRUE, 0);
    g_signal_connect(findPathButton, "clicked", G_CALLBACK(on_find_path_clicked), data);
    GtkStyleContext *context = gtk_widget_get_style_context(findPathButton);
    gtk_style_context_add_class(context, "suggested-action"); 
    
    GtkWidget *allPairsButton = gtk_button_new_with_label("Show All-Pairs");
    gtk_widget_set_tooltip_text(allPairsButton, "Show All-Pairs Matrix (Floyd-Warshall)");
    gtk_button_set_image(GTK_BUTTON(allPairsButton),
        gtk_image_new_from_icon_name("view-grid-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbox_buttons), allPairsButton, TRUE, TRUE, 0);
    g_signal_connect(allPairsButton, "clicked", G_CALLBACK(on_show_all_pairs_clicked), data);
    
    GtkWidget *listCitiesButton = gtk_button_new_with_label("List All Cities");
    gtk_button_set_image(GTK_BUTTON(listCitiesButton),
        gtk_image_new_from_icon_name("view-list-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbox_buttons), listCitiesButton, TRUE, TRUE, 0);
    g_signal_connect(listCitiesButton, "clicked", G_CALLBACK(on_list_cities_clicked), data);

    GtkWidget *listRoutesButton = gtk_button_new_with_label("Show All Routes");
    gtk_button_set_image(GTK_BUTTON(listRoutesButton),
        gtk_image_new_from_icon_name("network-wired-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(hbox_buttons), listRoutesButton, TRUE, TRUE, 0);
    g_signal_connect(listRoutesButton, "clicked", G_CALLBACK(on_list_routes_clicked), data);
    
    GtkWidget *resultsFrame = gtk_frame_new("Results");
    gtk_frame_set_label_align(GTK_FRAME(resultsFrame), 0.02, 0.5);
    gtk_container_set_border_width(GTK_CONTAINER(resultsFrame), 5);
    gtk_box_pack_start(GTK_BOX(vbox), resultsFrame, TRUE, TRUE, 5);
    
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledWindow),
        GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(resultsFrame), scrolledWindow);
    
    data->resultTextView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(data->resultTextView), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(data->resultTextView), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(data->resultTextView), GTK_WRAP_WORD);
    
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->resultTextView));
    gtk_text_buffer_create_tag(text_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(text_buffer, "heading", "weight", PANGO_WEIGHT_BOLD, "scale", 1.2, NULL);
    gtk_text_buffer_create_tag(text_buffer, "highlight", "background", "yellow", "foreground", "black", NULL);
    gtk_text_buffer_create_tag(text_buffer, "normal", "weight", PANGO_WEIGHT_NORMAL, "background", NULL, "foreground", NULL, NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "textview {"
        "  font-family: Monospace;"
        "  font-size: 10pt;"
        "}",
        -1, NULL);
    GtkStyleContext *css_context = gtk_widget_get_style_context(data->resultTextView);
    gtk_style_context_add_provider(css_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
    
    gtk_container_add(GTK_CONTAINER(scrolledWindow), data->resultTextView);
    set_result_text(GTK_TEXT_VIEW(data->resultTextView), "Welcome! Please load a map file to begin.");
    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    AppData *data = g_new0(AppData, 1);
    data->graphLoaded = FALSE;
    
    GtkWidget *login_window = create_login_window(data);
    gtk_widget_show_all(login_window);
    
    gtk_main();
    g_free(data);
    return 0;
}