#include <iostream>
#include <X11/Xlib.h>

using namespace std;

int main()
{
    Display *display;
    Window window;
    XEvent event;
    GC gc;

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
                                10, 10, 200, 200, 1,
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
    
    while(1)
    {
        XNextEvent(display, &event);
        if(event.type == Expose)
        {
           
        }

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
                has_start_point = false;
            }
        }

        if(event.type == ClientMessage)
        {
            if((Atom)event.xclient.data.l[0] == delWindow)
            {
                break;
            }
        }
    }

    cout<<"KELAS JALAN BROOO"<< endl;

    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}