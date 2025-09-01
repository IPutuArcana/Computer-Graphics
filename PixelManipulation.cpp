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
    cube_x += cube_dx;
    cube_y += cube_dy;

    // --- Random Number Setup ---
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 100); // For timing the change
    uniform_int_distribution<> dir_distrib(-1, 1); // For picking a new direction

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

                // --- New Tapered 3D Drawing Logic ---
        // Loop through each VERTEX using an index 'i'
        for (size_t i = 0; i < rayquaza_spine_vertices.size(); ++i) {
            const auto& vertex = rayquaza_spine_vertices[i];

            // ... (The rotation and projection logic is exactly the same as before)
            float rotated_x = vertex.x * std::cos(angle) - vertex.z * std::sin(angle);
            float rotated_y = vertex.y;
            float rotated_z = vertex.x * std::sin(angle) + vertex.z * std::cos(angle);
            int screen_x = static_cast<int>(rotated_x + 200);
            int screen_y = static_cast<int>(rotated_y + 200);

            // --- Calculate Dynamic Thickness ---
            // Calculate how far along the body we are (from 0.0 to 1.0)
            float progress = static_cast<float>(i) / (rayquaza_spine_vertices.size() - 1);
            // Use the sin function to make the thickness swell in the middle
            float sin_wave = std::sin(progress * 3.14159f); // M_PI is ~3.14
            int tube_thickness = 10 + static_cast<int>(sin_wave * 20); // Base 10px, swells up to 30px

            // Draw a filled square with the new dynamic thickness
            XFillRectangle(display, window, gc,
                        screen_x - tube_thickness / 2,
                        screen_y - tube_thickness / 2,
                        tube_thickness, tube_thickness);
        }

                // 3. Draw the cube
        // Loop through all the EDGES of our model
        for (const auto& edge : cube_edges) {
            // Get the two 3D points for this edge
            Point3D p1 = cube_vertices[edge.first];
            Point3D p2 = cube_vertices[edge.second];

            // Rotate point 1
            float p1_rotated_x = p1.x * std::cos(angle) - p1.z * std::sin(angle);
            float p1_rotated_y = p1.y;
            
            // Rotate point 2
            float p2_rotated_x = p2.x * std::cos(angle) - p2.z * std::sin(angle);
            float p2_rotated_y = p2.y;

            // Project and translate the two rotated points to the screen
            // This is the key step: we add cube_x and cube_y here
            int p1_screen_x = static_cast<int>(p1_rotated_x + cube_x);
            int p1_screen_y = static_cast<int>(p1_rotated_y + cube_y);

            int p2_screen_x = static_cast<int>(p2_rotated_x + cube_x);
            int p2_screen_y = static_cast<int>(p2_rotated_y + cube_y);

            // Draw the 2D line that represents the 3D edge
            XDrawLine(display, window, gc, p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y);
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