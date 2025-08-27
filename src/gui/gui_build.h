#ifndef GUI_BUILD__H
#define GUI_BUILD__H


//#include <dos/startup.h>
#include <workbench/icon.h>
//#include <workbench/startup.h>
//#include <libraries/keymap.h> // RAWKEY_#? codes
#include <classes/window.h>
//#include <gadgets/clicktab.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
//#include <gadgets/checkbox.h>
//#include <gadgets/chooser.h>
//#include <gadgets/string.h>
#include <gadgets/space.h>
#include <gadgets/listbrowser.h>
//#include <gadgets/fuelgauge.h>
//#include <gadgets/integer.h>
//#include <gadgets/getfile.h>
//#include <gadgets/slider.h>
//#include <gadgets/texteditor.h>
//#include <gadgets/scroller.h>
#include <images/label.h>
//#include <images/bitmap.h>


#define OBJ(x) gui->Objects[x]
#define GAD(x) (struct Gadget *)gui->Objects[x]


enum {
 COL_APT_CHK = 0, // enable/disbale checkbox
 COL_APT_NAM,
 COL_APT_VER,
 COL_APT_STA,
 COL_APT_DES,

 APT_TOT_COLS
};


void Free_ListBrowser(struct APTGUI *, struct List *);
void Launch_GUI(struct APTGUI *);


#endif
