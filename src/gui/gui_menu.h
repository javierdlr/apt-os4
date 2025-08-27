#ifndef GUI_MENU_H
#define GUI_MENU_H


//#include <images/bitmap.h>
#include <intuition/menuclass.h>


//struct Image *Menu_Image(STRPTR, struct Screen *);
void Create_Menu(struct APTGUI *);
void Free_Menu(struct APTGUI *);
int  Handle_MenuEvents(struct APTGUI *);


enum {
	MID_DUMMY = 0,
// Project
 MID_ABOUT,
 MID_ICONIFY,
 MID_QUIT,
// Edit
// Settings
 MID_SHOWHINTS,
};


#endif
