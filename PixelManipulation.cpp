#include <iostream>
#include <vector>
#include <unistd.h>
#include <X11/Xlib.h>

using namespace std;

struct Line
{
    int x1, y1, x2, y2;
};


int main()
{
    Display *display;
    Window window;
    XEvent event;
    GC gc;
    vector<Line> lines;

    const int SPRITE_WIDTH = 5;
    const int SPRITE_HEIGHT = 5;

    // 1 = draw a pixel, 0 = empty space
    int sprite_data[SPRITE_HEIGHT][SPRITE_WIDTH] = {
        {0, 0, 1, 0, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 1, 0, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0}
};

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
    
    // Animation variables
    int square_x = 195; 
    int square_y = 195;
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

        square_x += dx;
        if(square_x <= 0 || square_x >= 390) // Bounce off the walls
        {
            dx = -dx;
        }

        // Outer loop for the rows (y)
        for (int y = 0; y < SPRITE_HEIGHT; y++) 
        {
            // Inner loop for the columns (x)
            for (int x = 0; x < SPRITE_WIDTH; x++) {
                // Check if the sprite data at this position is a 1
                if (sprite_data[y][x] == 1) {
                    // If it is, draw a point at the correct screen location
                    XDrawPoint(display, window, gc, square_x + x, square_y + y);
                }
            }
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