#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>
#include <cmath>
#include <utility>

using namespace std;

struct Line
{
    int x1, y1, x2, y2;
};

struct Point3D
{
    float x, y, z;
};

   std::vector<Point3D> rayquaza_spine_vertices = 
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
    vector<Line> lines;

    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        cerr<<"display nya KOSONG BROOOO"<<endl;
        return 1;
    }

    int screen = DefaultScreen(display);
    window = XCreateSimpleWindow(
                                display,
                                RootWindow(display, screen),
                                10, 10, 400, 400, 1,
                                BlackPixel(display, screen),
                                WhitePixel(display, screen));
    
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delWindow, 1);

    XSelectInput(display, window, ExposureMask | StructureNotifyMask | ButtonPressMask);

    gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));

    XMapWindow(display, window);

    bool has_start_point = false;
    int start_x, start_y, end_x, end_y;
    
    // rotation variables
    float angle = 0.0f;

    int dx = 1; //direction x
    bool running = true;

    while(running)
    {
        while(XPending(display))
        {
            XNextEvent(display, &event);
            // if(event.type == Expose)
            // {
            //    for (const auto& line : lines) {
            //        XDrawLine(display, window, gc, line.x1, line.y1, line.x2, line.y2);
            //    }
            // }

            if(event.type == ButtonPress)
            {
                if(!has_start_point)
                {
                    start_x = event.xbutton.x;
                    start_y = event.xbutton.y;
                    cout<<"Start Point: ("<<start_x<<", "<<start_y<<")"<<endl;
                    has_start_point = true;
                }
                else
                {
                    end_x = event.xbutton.x;
                    end_y = event.xbutton.y;
                    cout<<"End Point: ("<<end_x<<", "<<end_y<<")"<<endl;
                    XDrawLine(display, window, gc, start_x, start_y, end_x, end_y);
                    lines.push_back({start_x, start_y, end_x, end_y});
                    has_start_point = false;
                }
            }

            if(event.type == ClientMessage)
            {
                if((Atom)event.xclient.data.l[0] == delWindow)
                {
                    running = false;
                }
            }
        }
        XClearWindow(display, window);

        for (const auto& line : lines) {
            XDrawLine(display, window, gc, line.x1, line.y1, line.x2, line.y2);
        }

                // ADD THIS NEW BLOCK
        // --- 3D Animation Logic ---
        angle += 0.01f; // Slowly increase the rotation angle

                // --- New 3D Drawing Logic ---
        // Loop through each VERTEX of our model
        for (const auto& vertex : rayquaza_spine_vertices) {
            // Rotate the 3D point around the Y-axis
            float rotated_x = vertex.x * std::cos(angle) - vertex.z * std::sin(angle);
            float rotated_y = vertex.y; // Keep y the same for now
            float rotated_z = vertex.x * std::sin(angle) + vertex.z * std::cos(angle);

            // Project the rotated 3D point to a 2D screen coordinate
            int screen_x = static_cast<int>(rotated_x + 200);
            int screen_y = static_cast<int>(rotated_y + 200);

            // Draw a filled square at that 2D point to create the "tube"
            int tube_thickness = 20;
            XFillRectangle(display, window, gc,
                        screen_x - tube_thickness / 2,
                        screen_y - tube_thickness / 2,
                        tube_thickness, tube_thickness);
        }
        
        XFlush(display);

        usleep(16667); // Delay for 16.67 milliseconds
    }

    cout<<"KELAS JALAN BROOO"<< endl;

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}