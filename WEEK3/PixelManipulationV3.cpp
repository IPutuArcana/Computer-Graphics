#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>
#include <cmath>
#include <utility>
#include <X11/Xutil.h> // For XLookupString and KeySym
#include <string>      // For std::string

using namespace std;

// Algorithm Selection: DAA or Bresenham
enum class DrawAlgorithm {
    BRUTE_FORCE,
    DDA,
    BRESENHAM
};

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


// --- REVISED: Flexible Brute-Force Line Drawing Algorithm ---
// This version respects the original x1,y1 -> x2,y2 direction.
void drawLineBruteForce(Display* display, Window window, GC gc, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Determine which axis has a larger range
    if (abs(dx) > abs(dy)) {
        // --- Iterate along the X-axis ---
        float m = (float)dy / (float)dx;
        int x_step = (dx > 0) ? 1 : -1;
        
        int current_x = x1;
        while (true) {
            float y = y1 + m * (current_x - x1);
            XDrawPoint(display, window, gc, current_x, round(y));
            
            if (current_x == x2) break; // Reached the end point
            current_x += x_step;
        }
    } else {
        // --- Iterate along the Y-axis ---
        // Handle vertical lines separately to avoid division by zero
        if (dy == 0) {
            if (dx == 0) { // It's a single point
                XDrawPoint(display, window, gc, x1, y1);
            }
            // If dx != 0, it's a horizontal line, handled by the other branch
            return;
        }

        float m_inv = (float)dx / (float)dy;
        int y_step = (dy > 0) ? 1 : -1;
        
        int current_y = y1;
        while (true) {
            float x = x1 + m_inv * (current_y - y1);
            XDrawPoint(display, window, gc, round(x), current_y);
            
            if (current_y == y2) break; // Reached the end point
            current_y += y_step;
        }
    }
}

// --- WEEK 3: Digital Differential Analyzer (DDA) Algorithm ---
// This version is more efficient than Brute-Force.
// It removes multiplication from the loop by incrementally adding the slope.
void drawLineDDA(Display* display, Window window, GC gc, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Determine which axis has a larger range
    if (abs(dx) > abs(dy)) {
        // --- Iterate along the X-axis (Shallow Slope) ---
        float m = (float)dy / (float)dx;
        int x_step = (dx > 0) ? 1 : -1;
        
        float y = (float)y1; // Start y as a float
        int x = x1;
        
        while (true) {
            XDrawPoint(display, window, gc, x, round(y));
            if (x == x2) break; // Reached the end point
            
            x += x_step;
            y += (m * x_step); // The core DDA step: y = y + m
        }
    } else {
        // --- Iterate along the Y-axis (Steep Slope) ---
        if (dy == 0) { // Handle horizontal lines
            if (dx == 0) { XDrawPoint(display, window, gc, x1, y1); }
            return;
        }

        float m_inv = (float)dx / (float)dy;
        int y_step = (dy > 0) ? 1 : -1;
        
        float x = (float)x1; // Start x as a float
        int y = y1;

        while (true) {
            XDrawPoint(display, window, gc, round(x), y);
            if (y == y2) break; // Reached the end point
            
            y += y_step;
            x += (m_inv * y_step); // The core DDA step: x = x + (1/m)
        }
    }
}


// --- WEEK 3: Generalized Bresenham's Line Algorithm ---
// This version is the most efficient.
// It works for all 8 octants (any slope) using only integer math.
void drawLineBresenham(Display* display, Window window, GC gc, int x1, int y1, int x2, int y2) {
    // TRICK 1: Handle "steep" lines by pretending they are "shallow".
    // A steep line is one where the change in Y is greater than the change in X.
    const bool is_steep = abs(y2 - y1) > abs(x2 - x1);
    if (is_steep) {
        // If it's steep, we swap the x and y coordinates. This reflects the line
        // across the y=x axis, turning it into a shallow line.
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    // TRICK 2: Always draw from left-to-right.
    // This simplifies our loop so we can always do `x++`.
    if (x1 > x2) {
        // If the starting point is to the right of the end point, swap them.
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    // Now, we can do the core Bresenham calculation.
    const int dx = x2 - x1;
    const int dy = abs(y2 - y1);
    
    // We need to know if y should be incrementing or decrementing.
    const int y_step = (y1 < y2) ? 1 : -1;

    // This is the "error term" or "decision parameter". It keeps track of
    // how far our pixel line has drifted from the true mathematical line.
    int error = dx / 2;
    int y = y1;

    // Loop through every x from the start to the end.
    for (int x = x1; x <= x2; x++) {
        // Draw the pixel.
        // IMPORTANT: If we swapped coordinates earlier (for a steep line),
        // we must "un-swap" them here, right before drawing.
        if (is_steep) {
            XDrawPoint(display, window, gc, y, x);
        } else {
            XDrawPoint(display, window, gc, x, y);
        }

        // Update the error term. Each step in x adds to the error.
        error -= dy;

        // Check if the error has crossed the threshold.
        if (error < 0) {
            // If it has, it's time to step in the y direction to correct it.
            y += y_step;
            // And we reset the error by adding dx back.
            error += dx;
        }
    }
}


// General 3D wireframe drawing (uses our brute-force function)
void drawEdges(Display* display, Window window, GC gc,
               const vector<Point3D>& vertices,
               const vector<pair<int, int>>& edges,
               float angle, int posX, int posY, DrawAlgorithm algo) {
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

        if (algo == DrawAlgorithm::BRESENHAM) {
            drawLineBresenham(display, window, gc, p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y);
        } else if (algo == DrawAlgorithm::DDA) {
            drawLineDDA(display, window, gc, p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y);
        } else { // Default to Brute-Force
            drawLineBruteForce(display, window, gc, p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y);
        }
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
    XSelectInput(display, window, ExposureMask | StructureNotifyMask | ButtonPressMask | KeyPressMask);
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XMapWindow(display, window);

    // --- Variables ---
    DrawAlgorithm current_algo = DrawAlgorithm::BRUTE_FORCE;
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

            if (event.type == KeyPress) {
                char buffer[10];
                KeySym keysym;
                XLookupString(&event.xkey, buffer, sizeof(buffer), &keysym, NULL);

                if (keysym == XK_f || keysym == XK_F) {
                    current_algo = DrawAlgorithm::BRUTE_FORCE;
                    cout << "Switched to Brute-Force Algorithm" << endl;
                } else if (keysym == XK_d || keysym == XK_D) {
                    current_algo = DrawAlgorithm::DDA;
                    cout << "Switched to DDA Algorithm" << endl;
                } else if (keysym == XK_b || keysym == XK_B) {
                    current_algo = DrawAlgorithm::BRESENHAM;
                    cout << "Switched to Bresenham's Algorithm" << endl;
                }
            }

            if (event.type == ButtonPress) {
                if (!has_start_point) {
                    start_x = event.xbutton.x;
                    start_y = event.xbutton.y;
                    cout << "Start point set to: (" << start_x << ", " << start_y << ")" << endl;
                    has_start_point = true;
                } else {
                    int end_x = event.xbutton.x;
                    int end_y = event.xbutton.y;
                    cout << "End point set to: (" << end_x << ", " << end_y << ")" << endl;
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

        // --- Draw UI Text for current algorithm ---
        string algo_text = "Algorithm: ";
        if (current_algo == DrawAlgorithm::BRESENHAM) {
            algo_text += "Bresenham (B)";
        } else if (current_algo == DrawAlgorithm::DDA) {
            algo_text += "DDA (D)";
        } else {
            algo_text += "Brute-Force (F)";
        }
        XDrawString(display, window, gc, 10, 20, algo_text.c_str(), algo_text.length());

        // Draw user lines with our new function
        for (const auto& line : user_lines) {
            if (current_algo == DrawAlgorithm::BRESENHAM) {
                drawLineBresenham(display, window, gc, line.x1, line.y1, line.x2, line.y2);
            } else if (current_algo == DrawAlgorithm::DDA) {
                drawLineDDA(display, window, gc, line.x1, line.y1, line.x2, line.y2);
            } else {
                drawLineBruteForce(display, window, gc, line.x1, line.y1, line.x2, line.y2);
            }
        }

        // Update cube position + rotation
        angle += 0.015f;
        cube_x += cube_dx;
        cube_y += cube_dy;
        if (cube_x <= 40 || cube_x >= 560) cube_dx *= -1;
        if (cube_y <= 40 || cube_y >= 560) cube_dy *= -1;

        // Draw cube and spine
        drawEdges(display, window, gc, cube_vertices, cube_edges, angle, cube_x, cube_y, current_algo);
        drawEdges(display, window, gc, rayquaza_spine_vertices, rayquaza_spine_edges, -angle * 0.5f, spine_x, spine_y, current_algo);

        XFlush(display);
        usleep(16667); // ~60fps
    }

    // Cleanup
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}