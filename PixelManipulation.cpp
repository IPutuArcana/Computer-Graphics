#include <iostream>
#include <X11/Xlib.h>

using namespace std;

int main()
{
    Display *display;
    Window window;
    XEvent event;

    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        cerr<<"display nya KOSONG BROOOO"<<endl;
        return 1;
    }

    int screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 200, 200, 1, BlackPixel(display, screen), WhitePixel(display, screen));
    
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delWindow, 1);

    XMapWindow(display, window);
    
    while(1)
    {
        XNextEvent(display, &event);
        if(event.type == ClientMessage)
        {
            if((Atom)event.xclient.data.l[0] == delWindow)
            {
                break;
            }
        }
    }

    cout<<"KELAS JALAN BROOO"<< endl;

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}