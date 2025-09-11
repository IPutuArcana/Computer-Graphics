#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>
#include <cmath>
#include <utility>

using namespace std;

struct Line {
    int x1, y1, x2, y2;
};

struct Point3D {
    float x, y, z;
};

struct EdgeBuilder {
    static vector<pair<int, int>> build(const vector<Point3D>& vertices) {
        vector<pair<int, int>> edges;
        for (size_t i = 0; i < vertices.size() - 1; ++i) {
            edges.push_back({static_cast<int>(i), static_cast<int>(i + 1)});
        }
        return edges;
    }
};

// Cube definition
const vector<Point3D> cube_vertices = {
    {-20, -20, -20}, {20, -20, -20}, {20, 20, -20}, {-20, 20, -20},
    {-20, -20, 20},  {20, -20, 20},  {20, 20, 20},  {-20, 20, 20}
};

const vector<pair<int, int>> cube_edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Sides
};

// Example Rayquaza spine (currently unused)
const vector<Point3D> rayquaza_spine_vertices = {
    {  0,   0,   0}, { 20,   5, -10}, { 30,  15, -20},
    { 25,  30, -30}, { 10,  40, -40}, {-10,  35, -50},
    {-20,  20, -60}, {-15,   5, -70}, {  0,   0, -80},
    { 10,  -5, -90}, { 20, -10, -100}, { 15, -20, -110},
    {  0, -25, -120}, {-10, -20, -130}, {-20, -15, -140}
};

const vector<pair<int, int>> rayquaza_spine_edges =
    EdgeBuilder::build(rayquaza_spine_vertices);

// Circle drawing function (placeholder)
void drawCircle(Display* display, Window window, GC gc, int centerX, int centerY, int radius) {
    // TODO: implement Bresenham / Midpoint circle if needed
}

int main() {
    // --- X11 Setup ---
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        cerr << "Cannot open display" << endl;
        return 1;
    }
    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                        10, 10, 400, 400, 1,
                                        BlackPixel(display, screen), WhitePixel(display, screen));
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delWindow, 1);
    XSelectInput(display, window, ExposureMask | StructureNotifyMask | ButtonPressMask);
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XMapWindow(display, window);

    // --- Variables ---
    vector<Line> user_lines; 
    bool has_start_point = false;
    int start_x = 0, start_y = 0;

    float angle = 0.0f;
    int cube_x = 200, cube_y = 200;
    int cube_dx = 1, cube_dy = 1;
    bool running = true;

    // --- Main Loop ---
    while (running) {
        // Handle input
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == ButtonPress) {
                if (!has_start_point) {
                    start_x = event.xbutton.x;
                    start_y = event.xbutton.y;
                    cout << "Start Point: (" << start_x << ", " << start_y << ")" << endl;
                    has_start_point = true;
                } else {
                    int end_x = event.xbutton.x;
                    int end_y = event.xbutton.y;
                    user_lines.push_back({start_x, start_y, end_x, end_y});
                    cout << "End Point: (" << end_x << ", " << end_y << ")" << endl;
                    has_start_point = false;
                }
            }
            if (event.type == ClientMessage &&
                (Atom)event.xclient.data.l[0] == delWindow) {
                running = false;
            }
        }

        // Clear window
        XClearWindow(display, window);

        // Draw user lines
        for (const auto& line : user_lines) {
            XDrawLine(display, window, gc, line.x1, line.y1, line.x2, line.y2);
        }

        // Update cube position + rotation
        angle += 0.015f;
        cube_x += cube_dx;
        cube_y += cube_dy;

        if (cube_x <= 20 || cube_x >= 380) cube_dx *= -1;
        if (cube_y <= 20 || cube_y >= 380) cube_dy *= -1;

        // Draw cube
        for (const auto& edge : cube_edges) {
            Point3D p1 = cube_vertices[edge.first];
            Point3D p2 = cube_vertices[edge.second];

            float p1_rot_x = p1.x * cos(angle) - p1.z * sin(angle);
            float p1_rot_y = p1.y;
            float p2_rot_x = p2.x * cos(angle) - p2.z * sin(angle);
            float p2_rot_y = p2.y;

            int p1_screen_x = static_cast<int>(p1_rot_x + cube_x);
            int p1_screen_y = static_cast<int>(p1_rot_y + cube_y);
            int p2_screen_x = static_cast<int>(p2_rot_x + cube_x);
            int p2_screen_y = static_cast<int>(p2_rot_y + cube_y);

            XDrawLine(display, window, gc, p1_screen_x, p1_screen_y,
                      p2_screen_x, p2_screen_y);
        }

        XFlush(display);
        usleep(16667); // ~60fps
    }

    // Cleanup
    cout << "Closing window." << endl;
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
