
#include "gui.h"
#include "gui_build.h"
//#include "gui_resources.h"
#include "gui_events.h"
#include "debug.h"


static void create_GUIObjects(struct APTGUI *gui) {
	struct DiskObject *iconify;
DBUG("%s()\n",__FUNCTION__);

	gui->apt_ColInfo = IListBrowser->AllocLBColumnInfo(APT_TOT_COLS,
	                                 LBCIA_Column,COL_APT_CHK, LBCIA_Title,NULL,
	                                                           //LBCIA_HorizJustify,LCJ_CENTER,
	                                 LBCIA_Column,COL_APT_NAM, LBCIA_Title,"Package",//LBCIA_Title,GetString(&Li, MSG_GUI_COLTITLE_NAME),
	                                                           //LBCIA_AutoSort,  TRUE,
	                                                           //LBCIA_SortArrow, TRUE,
	                                 LBCIA_Column,COL_APT_VER, LBCIA_Title,"Version",//GetString(&Li, MSG_GUI_COLTITLE_NAME),
	                                                           LBCIA_HorizJustify,LCJ_CENTER,
	                                                           //LBCIA_AutoSort,  TRUE,
	                                                           //LBCIA_SortArrow, TRUE,
	                                 LBCIA_Column,COL_APT_STA, LBCIA_Title,"Status",//GetString(&Li, MSG_GUI_COLTITLE_NAME),
	                                                           LBCIA_HorizJustify,LCJ_CENTER,
	                                                           //LBCIA_AutoSort,  TRUE,
	                                                           //LBCIA_SortArrow, TRUE,
	                                 LBCIA_Column,COL_APT_DES, LBCIA_Title,"Description",//GetString(&Li, MSG_GUI_COLTITLE_NAME),
	                                                           //LBCIA_AutoSort,  TRUE,
	                                                           //LBCIA_SortArrow, TRUE,
	                                TAG_DONE);
DBUG("  apt_ColInfo = %p\n",gui->apt_ColInfo);

	// reset icon X/Y positions so it iconifies properly on Workbench
	iconify = IIcon->GetIconTags("PROGDIR:"PROGNAME, ICONGETA_FailIfUnavailable,FALSE, TAG_END);
	if (iconify != NULL) {
		iconify->do_CurrentX = NO_ICON_POSITION;
		iconify->do_CurrentY = NO_ICON_POSITION;
	}

	OBJ(OID_MAIN) = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        //WA_ScreenTitle, VERS" ("DATE")",
        WA_Title,       PROGNAME,
        //WA_PubScreen,   gui->screen,
        //WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_SizeGadget,  TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
//WA_InnerWidth,200,
WA_InnerHeight,300,
        WINDOW_IconifyGadget, TRUE,
        WINDOW_AppPort,       gui->AppPort,
        WINDOW_Icon,          iconify,
        WINDOW_Position, WPOS_CENTERSCREEN,
        WINDOW_PopupGadget, TRUE,
        WINDOW_UniqueID,    PROGNAME"_main",
        //WINDOW_MenuStrip, OBJ(OID_MENU), // menuclass
        //WINDOW_JumpScreensMenu, TRUE,
        //WINDOW_GadgetHelp, gui->myTT.show_hints,
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,  TRUE,

LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,  TRUE,
			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
				//LAYOUT_SpaceOuter,  TRUE,
				//LAYOUT_EvenSize,    TRUE,
				LAYOUT_AddChild, OBJ(OID_SEARCH_STR) = IIntuition->NewObject(StringClass, NULL,
					GA_ID,         OID_SEARCH_STR,
					GA_RelVerify,  TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					STRINGA_TextVal, NULL,
				TAG_DONE),
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL,
					LABEL_Text, "_Search:",//GetString(&Li, MSG_GUI_KL_CHOOSER),
				TAG_DONE),
				//LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
				//CHILD_WeightedWidth, 0,
			TAG_DONE),
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild, OBJ(OID_APTLIST) = IIntuition->NewObject(ListBrowserClass, NULL,
				GA_ID,        OID_APTLIST,
				GA_RelVerify, TRUE,
				//GA_ReadOnly, TRUE,
				//GA_HintInfo, GetString(&Li, MSG_GUI_APTLIST_HELP),
				LISTBROWSER_AutoFit,        TRUE,
				LISTBROWSER_Labels,         NULL,//gui->apt_list,
				LISTBROWSER_ColumnInfo,     gui->apt_ColInfo,
				LISTBROWSER_ColumnTitles,   TRUE,
				LISTBROWSER_TitleClickable, TRUE,
				//LISTBROWSER_SortColumn,     COL_APT_NAM,
				//LISTBROWSER_Selected,       -1,
				LISTBROWSER_ShowSelected,   TRUE,
				LISTBROWSER_MinVisible,     12,
				LISTBROWSER_Striping,       LBS_ROWS,
				//LISTBROWSER_HorizontalProp, TRUE,
				//LISTBROWSER_Spacing,        2,
				LISTBROWSER_AllowNoSelection, TRUE,
			TAG_DONE),
			//CHILD_MinWidth, 300,
			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
				//LAYOUT_SpaceOuter,  TRUE,
				LAYOUT_AddChild, OBJ(OID_LIST) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_LIST,
					GA_RelVerify,  TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "List",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, OBJ(OID_ENTRIES_CNT) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ReadOnly, TRUE,
					GA_Text,     "Selected: %ld of %ld",//GetString(&Li, MSG_GUI_MODULES_COUNT),
					BUTTON_BevelStyle,    BVS_NONE,
					BUTTON_Transparent,   TRUE,
					BUTTON_Justification, BCJ_RIGHT,
					BUTTON_VarArgs,       gui->counter,
				TAG_DONE),
			TAG_DONE),
			CHILD_WeightedHeight, 0,
TAG_DONE),

LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_BevelStyle, BVS_SBAR_VERT,
         LAYOUT_SpaceOuter,  TRUE,
         //LAYOUT_SpaceInner,  TRUE,
         LAYOUT_EvenSize,    TRUE,
				LAYOUT_AddChild, OBJ(OID_INSTALL) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_INSTALL,
					GA_RelVerify,  TRUE,
					GA_Disabled,   TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "Install",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
				LAYOUT_AddChild, OBJ(OID_UNINSTALL) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_UNINSTALL,
					GA_RelVerify,  TRUE,
					GA_Disabled,   TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "Uninstall",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
				CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, OBJ(OID_REMOVE) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_REMOVE,
					GA_RelVerify,  TRUE,
					GA_Disabled,   TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "Remove",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
				CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, OBJ(OID_UPDATE) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_UPDATE,
					GA_RelVerify,  TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "Update",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
				LAYOUT_AddChild, OBJ(OID_UPGRADE) = IIntuition->NewObject(ButtonClass, NULL,
					GA_ID,         OID_UPGRADE,
					GA_RelVerify,  TRUE,
					//GA_HintInfo,   GetString(&Li, MSG_GUI_VERIFY_HELP),
					GA_Text,       "Upgrade",//GetString(&Li, MSG_GUI_VERIFY),
				TAG_DONE),
TAG_DONE),
CHILD_WeightedHeight, 0,

        TAG_DONE),
	TAG_DONE);
DBUG("  OBJ(OID_MAIN) = %p\n",OBJ(OID_MAIN));
}


/* Free all GUI stuff/objects. */
static void free_GUI(struct APTGUI *gui) {
DBUG("%s()\n",__FUNCTION__);
	//IIntuition->IDoMethod(OBJ(OID_MAIN), WM_CLOSE, NULL);
	//gui->win[WID_MAIN] = NULL;
	IIntuition->DisposeObject( OBJ(OID_MAIN) ); // MAIN window
	OBJ(OID_MAIN) = NULL;
	gui->win[WID_MAIN] = NULL;

	//Free_Menu(gui);

	Free_ListBrowser(gui, gui->apt_list);
	IExec->FreeSysObject(ASOT_LIST, gui->apt_list);
	gui->apt_list = NULL;
	IListBrowser->FreeLBColumnInfo(gui->apt_ColInfo);
	gui->apt_ColInfo = NULL;

	IExec->FreeSysObject(ASOT_PORT, gui->AppPort);
	gui->AppPort = NULL;
}


/* Free list(browser) nodes. */
void Free_ListBrowser(struct APTGUI *gui, struct List *l) {
DBUG("%s() %p\n",__FUNCTION__,l);
	if ( IsListEmpty(l) ) {
//DBUG("  list was empty\n",NULL);
		return;
	}

	while ( IExec->RemHead(l) ) {};

	IListBrowser->FreeListBrowserList(l);
}

void Launch_GUI(struct APTGUI *gui) {
DBUG("%s()\n",__FUNCTION__);
	gui->AppPort = IExec->AllocSysObject(ASOT_PORT, NULL);
	gui->apt_list = IExec->AllocSysObject(ASOT_LIST, NULL);

	//Create_Menu(gui);

	create_GUIObjects(gui);


struct Node *n = IListBrowser->AllocListBrowserNode(APT_TOT_COLS,
	                                        //LBNA_UserData, (APTR)
	                                        LBNA_CheckBox, TRUE,
	                                        LBNA_Checked,  FALSE,///is_enabled,
	                                        LBNA_Column, COL_APT_NAM,
	                                          LBNCA_CopyText,TRUE, LBNCA_Text,"amigaos4-clib4",
	                                        LBNA_Column, COL_APT_VER,
	                                          LBNCA_HorizJustify,LCJ_CENTER,
	                                          LBNCA_CopyText,TRUE, LBNCA_Text,"2.0.0",
	                                        LBNA_Column, COL_APT_STA,
	                                          LBNCA_HorizJustify,LCJ_CENTER,
	                                          LBNCA_CopyText,TRUE, LBNCA_Text,"installed",
	                                        LBNA_Column, COL_APT_DES,
	                                          LBNCA_CopyText,TRUE, LBNCA_Text,"clib4 for AmigaOS4",
	                                       TAG_END);
if (n) {
	IExec->AddTail(gui->apt_list, n);
}
IIntuition->SetAttrs(GAD(OID_APTLIST), LISTBROWSER_Labels,gui->apt_list, TAG_DONE);


	gui->win[WID_MAIN] = (struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL);
	if (gui->win[WID_MAIN] != NULL) {
		//IIntuition->ScreenToFront(gui->win[WID_MAIN]->WScreen);
		while (Process_GUI(gui) != 0) {};
	} // END if(gui->win[WID_MAIN]..

	free_GUI(gui);
}
