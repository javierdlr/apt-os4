#ifndef GUI_RESOURCES_H
#define GUI_RESOURCES_H


#include <classes/requester.h>

#include <stdarg.h> // used in  Do_Message()


int Open_Resources(struct APTGUI *);
void Close_Resources(struct APTGUI *);
uint32 VARARGS68K Do_Message(struct APTGUI *, char reqimg, CONST_STRPTR buttons, CONST_STRPTR message, ...);
struct Screen* FrontMost_Scr(struct APTGUI *);


#endif
