/*#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo Li;*/


#include "gui.h"
#include "gui_menu.h"
#include "gui_resources.h"
#include "gui_build.h"
//#include "gui_events.h"
#include "debug.h"
#include "gui_rev.h"


/*********************/
//uint16 MenuImgSize = 24; // temporal, should be a tooltype
/*********************/

/*
 * To use in menu for adding an (AISS) image to option/entry
 * Needs 'MenuImgSize' (above) variable for setting size (defaults 24px)
 * -Example: MA_Image, menu_Image("open",screen),
 */
/*static struct Image *menu_Image(STRPTR name, struct Screen *scr)
{
    struct Image *i = NULL;
    APTR prev_win;
    BPTR dir, prev_dir;
    STRPTR name_s, name_g;
    uint32 len, totlen;

    len = Strlen(name);
    totlen = 2*len + 6;

    name_s = AllocVecTags(totlen, TAG_END); // name_s_LENGTH + name_s_LENGTH + 6
    if(name_s)
    {
        name_g = name_s + len + 3; // name_g "stored" at name_s_LENGTH + 3

        Strlcpy(name_s, name, totlen);
        Strlcat(name_s, "_s", totlen);

        Strlcpy(name_g, name, totlen);
        Strlcat(name_g, "_g", totlen);

        prev_win = SetProcWindow( (APTR)-1 ); // Disable requesters
        dir = Lock("TBIMAGES:", SHARED_LOCK);
        SetProcWindow(prev_win);              // Re-enable requesters
        if(dir != ZERO)
        {
            prev_dir = SetCurrentDir(dir);

            i = (struct Image *)NewObject(BitMapClass, NULL, //"bitmap.image",
                        (MenuImgSize!=24)? IA_Scalable : TAG_IGNORE, TRUE,
                        //IA_Width,24, IA_Height,24+2,
                        BITMAP_SourceFile,         name,
                        BITMAP_SelectSourceFile,   name_s,
                        BITMAP_DisabledSourceFile, name_g,
                        BITMAP_Screen, scr,//FrontMost_Scr(),
                        BITMAP_Masking, TRUE,
                       TAG_END);
            if(i)
            {
                SetAttrs( (Object *)i, IA_Height,MenuImgSize+2, IA_Width,MenuImgSize, TAG_END);
            }

            SetCurrentDir(prev_dir);
            UnLock(dir);
        }

        FreeVec(name_s);
    }

    return(i);
}*/


/* Build the menu object. */
void Create_Menu(struct APTGUI *gui)
{
DBUG("%s()\n",__FUNCTION__);
    OBJ(OID_MENU) = IIntuition->NewObject(NULL, "menuclass",
//PROJECT
     MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,"Project",//GetString(&Li,MSG_MENU_PROJECT),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,"Iconify",//GetString(&Li,MSG_MENU_ICONIFY),
        MA_ID,    MID_ICONIFY,
        MA_Key,   "I",
        //MA_Image, MenuImage("iconify"),
      TAG_END),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,"About...",//GetString(&Li,MSG_MENU_ABOUT),
        MA_ID,    MID_ABOUT,
        MA_Key,   "?",
        //MA_Image, MenuImage("info",gui->scr),
      TAG_END),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass", MA_Type,T_ITEM, MA_Separator,TRUE, TAG_END),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,"Quit",//GetString(&Li,MSG_MENU_QUIT),
        MA_ID,    MID_QUIT,
        MA_Key,   "Q",
        //MA_Image, MenuImage("quit",gui->scr),
      TAG_END),
     TAG_END),
//EDIT
     /*MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,GetString(&Li,MSG_MENU_EDIT),//"Edit",
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetString(&Li,MSG_COPY),//"Copy",
        MA_ID,    MID_COPY,
        MA_Key,   "C",
        //MA_Image, MenuImage("copy",gui->scr),
      TAG_END),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetString(&Li,MSG_CUT),//"Cut",
        MA_ID,    MID_CUT,
        MA_Key,   "X",
        //MA_Image, MenuImage("cut",gui->scr),
      TAG_END),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,GetString(&Li,MSG_PASTE),//"Paste",
        MA_ID,    MID_PASTE,
        MA_Key,   "V",
        //MA_Image, MenuImage("paste",gui->scr),
      TAG_END),
     TAG_END),*/
//SETTINGS
     MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
      MA_Type,T_MENU, MA_Label,"Settings",//GetString(&Li,MSG_MENU_SETTINGS),
      MA_AddChild, IIntuition->NewObject(NULL, "menuclass",
        MA_Type,T_ITEM, MA_Label,"Show hints",//GetString(&Li,MSG_MENU_SHOWHINTS),
        MA_ID,    MID_SHOWHINTS,
        MA_Key,   "H",
        //MA_Image, MenuImage("helpbubble",gui->scr),
        MA_Toggle,   TRUE,
        MA_Selected, gui->myTT.show_hints,
      TAG_END),
     TAG_END),
//
    TAG_END);
}


/* Free previously created menu object. */
void Free_Menu(struct APTGUI *gui)
{
DBUG("%s() 0x%08lx\n",__FUNCTION__,OBJ(OID_MENU));
	IIntuition->DisposeObject( OBJ(OID_MENU) );
	OBJ(OID_MENU) = NULL;
}


/*
 * Menu object handling events.
 * Returns BOOL to continue/stop main gui process 'Process_GUI()'
 */
int Handle_MenuEvents(struct APTGUI *gui)
{
	int    processGUI = 1;
	uint32 mitem      = NO_MENU_ID;

	while ( (mitem=IIntuition->IDoMethod(OBJ(OID_MENU),MM_NEXTSELECT,0,mitem)) != NO_MENU_ID ) {
DBUG("%s() %04ld\n",__FUNCTION__,mitem);
		//Object *obj = (Object *)IIntuition->IDoMethod(OBJ(OID_MENU), MM_FINDID, 0, mitem);

		switch (mitem) {
// PROJECT
			case MID_ICONIFY:
				if ( IIntuition->IDoMethod(OBJ(OID_MAIN), WM_ICONIFY) ) {
					gui->win[WID_MAIN] = NULL;
				}
				break;
			case MID_ABOUT:
				Do_Message(gui, REQIMAGE_INFO, NULL, "\033b%s (%s)",VERS,DATE);//GetString(&Li,MSG_ABOUTWIN_TEXT),VERS,DATE);
				break;
			case MID_QUIT:
				processGUI = 0;
				break;
// SETTINGS
			case MID_SHOWHINTS:
				//gui->myTT.show_hints = !gui->myTT.show_hints;
				//IIntuition->SetAttrs(OBJ(OID_MAIN), WINDOW_GadgetHelp,gui->myTT.show_hints, TAG_DONE);
				break;
			default:
				break;
		}

	}

	return processGUI;
}
