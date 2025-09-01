#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>
#include <cmath>
#include <utility>
#include <random>

using namespace std;

struct Line
{
    int x1, y1, x2, y2;
};

struct Point3D
{
    float x, y, z;
};

// rotation and position variables
float angle = 0.0f;
int cube_x = 200; // Starting X position
int cube_y = 200; // Starting Y position
int cube_dx = 1;  // Movement speed/direction on X
int cube_dy = 1;  // Movement speed/direction on Y

// The 8 corners of a cube
vector<Point3D> cube_vertices = {
    {-20, -20, -20}, {20, -20, -20}, {20, 20, -20}, {-20, 20, -20},
    {-20, -20, 20}, {20, -20, 20}, {20, 20, 20}, {-20, 20, 20}
};

// The 12 edges connecting the corners
vector<pair<int, int>> cube_edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting sides
};

   vector<Point3D> rayquaza_spine_vertices = 
    {
        {  0,   0,   0}, { 20,   5, -10}, { 30,  15, -20},
        { 25,  30, -30}, { 10,  40, -40}, {-10,  35, -50},
        {-20,  20, -60}, {-15,   5, -70}, {  0,   0, -80},
        { 10,  -5, -90}, { 20, -10, -100}, { 15, -20, -110},
        {  0, -25, -120}, {-10, -20, -130}, {-20, -15, -140}
    };

// Defines the edges connecting the vertices
    vector<pair<int, int>> rayquaza_spine_edges;

class EdgeBuilder {
public:
    EdgeBuilder() {
        for (size_t i = 0; i < rayquaza_spine_vertices.size() - 1; ++i) {
            rayquaza_spine_edges.push_back({static_cast<int>(i), static_cast<int>(i + 1)});
        }
    }
};
EdgeBuilder edge_builder;

// circle drawing function
void drawCircle(Display *display, Window window, GC gc, int centerX, int centerY, int radius)
{
    
}


int main()
{
    Display *display;
    Window window;
    XEvent event;
    GC gc;
    vector<Line> lines; // For the line drawing feature

    // --- X11 Setup (this is all correct) ---
    display = XOpenDisplay(NULL);
    if(display == NULL) {
        cerr << "Cannot open display" << endl;
        return 1;
    }
    int screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                10, 10, 400, 400, 1,
                                BlackPixel(display, screen), WhitePixel(display, screen));
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delWindow, 1);
    XSelectInput(display, window, ExposureMask | StructureNotifyMask | ButtonPressMask);
    gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XMapWindow(display, window);

    // --- Line Drawing Variables ---
    bool has_start_point = false;
    int start_x, start_y;

    // --- Animation variables (now all together) ---
    float angle = 0.0f;
    int cube_x = 200;
    int cube_y = 200;
    int cube_dx = 1;
    int cube_dy = 1;
    bool running = true;

    // --- Main Animation Loop ---
    while(running)
    {
        // Handle input events
        while(XPending(display))
        {
            XNextEvent(display, &event);
            if(event.type == ButtonPress) {
                if(!has_start_point) {
                    start_x = event.xbutton.x;
                    start_y = event.xbutton.y;
                    has_start_point = true;
                } else {
                    int end_x = event.xbutton.x;
                    int end_y = event.xbutton.y;
                    lines.push_back({start_x, start_y, end_x, end_y});
                    has_start_point = false;
                }
            }
            if(event.type == ClientMessage) {
                if((Atom)event.xclient.data.l[0] == delWindow) {
                    running = false;
                }
            }
        }

        // --- Drawing and Animation Logic ---
        XClearWindow(display, window);

        // Redraw user-drawn lines
        for (const auto& line : lines) {
            XDrawLine(display, window, gc, line.x1, line.y1, line.x2, line.y2);
        }

        // 1. Update rotation and position
        angle += 0.015f; // A little faster
        cube_x += cube_dx;
        cube_y += cube_dy;

        // 2. Boundary Checking for bouncing
        if (cube_x <= 20 || cube_x >= 380) { // 400 - 20 (half cube size)
            cube_dx *= -1;
        }
        if (cube_y <= 20 || cube_y >= 380) { // 400 - 20 (half cube size)
            cube_dy *= -1;
        }

        // 3. Draw the rotating cube at its new position
        for (const auto& edge : cube_edges) {
            Point3D p1 = cube_vertices[edge.first];
            Point3D p2 = cube_vertices[edge.second];

            float p1_rotated_x = p1.x * std::cos(angle) - p1.z * std::sin(angle);
            float p1_rotated_y = p1.y;
            float p2_rotated_x = p2.x * std::cos(angle) - p2.z * std::sin(angle);
            float p2_rotated_y = p2.y;

            int p1_screen_x = static_cast<int>(p1_rotated_x + cube_x);
            int p1_screen_y = static_cast<int>(p1_rotated_y + cube_y);
            int p2_screen_x = static_cast<int>(p2_rotated_x + cube_x);
            int p2_screen_y = static_cast<int>(p2_rotated_y + cube_y);

            XDrawLine(display, window, gc, p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y);
        }

        XFlush(display);
        usleep(16667);
    }

    // --- Cleanup ---
    cout << "Closing window." << endl;
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}