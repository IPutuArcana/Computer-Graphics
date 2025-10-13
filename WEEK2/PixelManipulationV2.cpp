#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>
#include <cmath>
#include <utility>
#include <algorithm> // For std::swap

using namespace std;

// Struct definitions remain the same
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

// Object definitions remain the same
const vector<Point3D> cube_vertices = {
    {-20, -20, -20}, {20, -20, -20}, {20, 20, -20}, {-20, 20, -20},
    {-20, -20, 20},  {20, -20, 20},  {20, 20, 20},  {-20, 20, 20}
};
const vector<pair<int, int>> cube_edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};
const vector<Point3D> rayquaza_spine_vertices = {
    {  0,   0,   0}, { 20,   5, -10}, { 30,  15, -20}, { 25,  30, -30}, { 10,  40, -40},
    {-10,  35, -50}, {-20,  20, -60}, {-15,   5, -70}, {  0,   0, -80}, { 10,  -5, -90},
    { 20, -10, -100},{ 15, -20, -110},{  0, -25, -120},{-10, -20, -130},{-20, -15, -140}
};
const vector<pair<int, int>> rayquaza_spine_edges = EdgeBuilder::build(rayquaza_spine_vertices);


// --- NEW: Brute-Force Line Drawing Algorithm ---
// This function draws a line pixel by pixel using the equation y = mx + c.
void drawLineBruteForce(Display* display, Window window, GC gc, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // A simple implementation of the brute-force algorithm iterates over the x-axis.
    // This creates gaps in steep lines (where |dy| > |dx|).
    // To fix this, we check which delta is larger and iterate over that axis.
    if (abs(dx) > abs(dy)) {
        // --- Iterate over x-axis (slope is gentle, |m| <= 1) ---

        // Ensure we always draw from left to right
        if (x1 > x2) {
            swap(x1, x2);
            swap(y1, y2);
        }

        float m = (float)dy / (float)dx;
        for (int x = x1; x <= x2; ++x) {
            float y = y1 + m * (x - x1);
            XDrawPoint(display, window, gc, x, round(y));
        }
    } else {
        // --- Iterate over y-axis (slope is steep, |m| > 1) ---

        // Ensure we always draw from bottom to top (in screen coordinates)
        if (y1 > y2) {
            swap(x1, x2);
            swap(y1, y2);
        }
        
        // Handle vertical line case to avoid division by zero
        if (dy == 0) {
             if(dx == 0) { // It's a single point
                XDrawPoint(display, window, gc, x1, y1);
             }
             return; // Or handle as a horizontal line if dx !=0, but the other branch catches that
        }

        float m_inv = (float)dx / (float)dy; // Inverse slope
        for (int y = y1; y <= y2; ++y) {
            float x = x1 + m_inv * (y - y1);
            XDrawPoint(display, window, gc, round(x), y);
        }
    }
}


// General 3D wireframe drawing (now uses our brute-force function)
void drawEdges(Display* display, Window window, GC gc,
               const vector<Point3D>& vertices,
               const vector<pair<int, int>>& edges,
               float angle, int posX, int posY) {
    for (const auto& edge : edges) {
        Point3D p1 = vertices[edge.first];
        Point3D p2 = vertices[edge.second];

        // simple rotation (XZ plane)
        float p1_rot_x = p1.x * cos(angle) - p1.z * sin(angle);
        float p1_rot_y = p1.y;
        float p2_rot_x = p2.x * cos(angle) - p2.z * sin(angle);
        float p2_rot_y = p2.y;

        int p1_screen_x = static_cast<int>(p1_rot_x + posX);
        int p1_screen_y = static_cast<int>(p1_rot_y + posY);
        int p2_screen_x = static_cast<int>(p2_rot_x + posX);
        int p2_screen_y = static_cast<int>(p2_rot_y + posY);

        // --- MODIFIED: Call our new function instead of XDrawLine ---
        drawLineBruteForce(display, window, gc,
                           p1_screen_x, p1_screen_y,
                           p2_screen_x, p2_screen_y);
    }
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
                                        10, 10, 600, 600, 1,
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
    int cube_x = 200, cube_y = 200, cube_dx = 1, cube_dy = 1;
    int spine_x = 400, spine_y = 300;
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
                    has_start_point = true;
                } else {
                    int end_x = event.xbutton.x;
                    int end_y = event.xbutton.y;
                    user_lines.push_back({start_x, start_y, end_x, end_y});
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

        // --- MODIFIED: Draw user lines with our new function ---
        for (const auto& line : user_lines) {
            drawLineBruteForce(display, window, gc, line.x1, line.y1, line.x2, line.y2);
        }

        // Update cube position + rotation
        angle += 0.015f;
        cube_x += cube_dx;
        cube_y += cube_dy;
        if (cube_x <= 40 || cube_x >= 560) cube_dx *= -1;
        if (cube_y <= 40 || cube_y >= 560) cube_dy *= -1;

        // Draw cube and spine (these now use drawLineBruteForce via drawEdges)
        drawEdges(display, window, gc, cube_vertices, cube_edges, angle, cube_x, cube_y);
        drawEdges(display, window, gc, rayquaza_spine_vertices, rayquaza_spine_edges, -angle * 0.5f, spine_x, spine_y);

        XFlush(display);
        usleep(16667); // ~60fps
    }

    // Cleanup
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}