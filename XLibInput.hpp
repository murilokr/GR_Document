#if __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <string>

enum Action{
    KeyLeft = 0,
    KeyRight = 1,
    KeyZoomIn = 2,
    KeyZoomOut = 3
};



/**
 * getWindowName
 * Função: Retorna o nome da janela win
 * 
 * In: Display *disp (O "Monitor")
 * In: Window win (A Window que a função irá extrair o nome)
 * Out: char* list (O nome da window)
 */
char *getWindowName(Display *disp, Window win) {
    Atom prop = XInternAtom(disp,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;


    if (XGetWindowProperty(disp,win,prop,0,1024,False,AnyPropertyType,
                &type,&form,&len,&remain,&list) != Success) { // XA_STRING

        return NULL;
    }

    return (char*)list;
}

/**
 * createKeyEvent
 * Função: Cria uma estrutura de KeyEvent para ser passado no buffer de inputs do S.O
 * 
 * In: Display *display (A janela que irá passar os inputs)
 * In: Window &win (Referência à janela que irá receber os inputs)
 * In: bool press (Determina se o input é um KeyUp ou um KeyDown)
 * In: int keycode (Código do input a ser passado)
 * In: int modifier (Modificadores do input Ex: (Ctrl + tecla, Shift + tecla, etc...))
 * 
 * Out: XKeyEvent event (A estrutura criada do input)
 */
XKeyEvent createKeyEvent(Display *display, Window &win,
                           Window &winRoot, bool press,
                           int keycode, int modifiers)
{
   XKeyEvent event;

   event.display     = display;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = True;
   event.keycode     = XKeysymToKeycode(display, keycode);
   event.state       = modifiers;


   if(press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}


/**
 * SendInput
 * Função: Envia o input para o programa que tem foco
 * 
 * In: Action ac (Enumerador que determina qual tecla será gerada)
 */ 
void SendInput(Action ac){
    int modifier = 0;

    KeySym KEYCODE;
    switch (ac){
        case KeyLeft:
            KEYCODE = XK_Left;
            break;
        case KeyRight:
            KEYCODE = XK_Right;
            break;
        case KeyZoomIn:
            KEYCODE = XK_KP_Add;
            modifier = ControlMask;
            break;
        case KeyZoomOut:
            KEYCODE = XK_KP_Subtract;
            modifier = ControlMask;
            break;
        default:
            KEYCODE = XK_Escape;
            break;
    }

    Display *display = XOpenDisplay(0);
    if(display == NULL)
        return;

    Window winRoot = XDefaultRootWindow(display);

    Window winFocus;
    int    revert;
    XGetInputFocus(display, &winFocus, &revert);

    char *winName;
    winName = getWindowName(display, winFocus);
    cout << "Sending " << ac << " to " << winName << " window." << endl;

    XKeyEvent event = createKeyEvent(display, winFocus, winRoot, true, KEYCODE, modifier);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

    XCloseDisplay(display);
}


#endif //__linux__