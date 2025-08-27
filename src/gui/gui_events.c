
#include "gui.h"
#include "gui_events.h"
//#include "gui_resources.h"
#include "gui_build.h"
#include "gui_menu.h"
#include "debug.h"


static void refresh_ModulesCounter(struct APTGUI *gui, uint32 status) {
DBUG("%s() %ld\n",__FUNCTION__,status);
 if (status == LBRE_CHECKED) {
  ++gui->counter[CNT_ACT];
 } else {
  --gui->counter[CNT_ACT];
 }

 IIntuition->SetGadgetAttrs(GAD(OID_ENTRIES_CNT), gui->win[WID_MAIN], NULL,
                            BUTTON_VarArgs,gui->counter, TAG_DONE);
}


static void events_GadgetUp(struct APTGUI *gui, uint32 res) {
 uint32 res_value;
DBUG("%s() 0x%08lx\n",__FUNCTION__,res);
 switch (res) {
  case OID_APTLIST:
   IIntuition->GetAttr(LISTBROWSER_RelEvent, OBJ(OID_APTLIST), &res_value);
//DBUG("  [OID_APTLIST] event=0x%08lx\n",res_value);
   if (res_value==LBRE_CHECKED || res_value==LBRE_UNCHECKED) {
    refresh_ModulesCounter(gui, res_value);
    IIntuition->RefreshSetGadgetAttrs(GAD(OID_BUTTONSRIGHT_GROUP), gui->win[WID_MAIN], NULL,
                                      GA_Disabled,gui->counter[CNT_ACT]? FALSE: TRUE, TAG_DONE);
   }
   break;

  case OID_SEARCH_STR:
DBUG(" [SEARCH]\n",NULL);
   break;
//OID_BUTTONSRIGHT_GROUP
  case OID_INSTALL:
DBUG(" [INSTALL]\n",NULL);
   break;
  case OID_UNINSTALL:
DBUG(" [UNINSTALL]\n",NULL);
   break;
  case OID_REMOVE:
DBUG(" [REMOVE]\n",NULL);
   break;
//OID_BUTTONSDOWN_GROUP
  case OID_UPDATE:
DBUG(" [UPDATE]\n",NULL);
   break;
  case OID_UPGRADE:
DBUG(" [UPGRADE]\n",NULL);
   break;

  default:
   break;
 }
}


/*
 * Process MAIN GUI window
 * events.
 */
int Process_GUI(struct APTGUI *gui) {
 int    done      = 1;
 uint16 code      = 0;
 uint32 result    = WMHI_LASTMSG,
        siggot    = 0,
        main_sigmask; // MAIN window
//DBUG("%s()\n",__FUNCTION__);
 IIntuition->GetAttr(WINDOW_SigMask, OBJ(OID_MAIN), &main_sigmask);
 siggot = IExec->Wait(main_sigmask | SIGBREAKF_CTRL_C);

 if (siggot & SIGBREAKF_CTRL_C) {
  return 0;
 }

 // MAIN window events
 if (siggot & main_sigmask) {
  while ( (result=IIntuition->IDoMethod(OBJ(OID_MAIN), WM_HANDLEINPUT, &code)) != WMHI_LASTMSG ) {
//DBUG("result=0x%lx\n",result);
   switch (result & WMHI_CLASSMASK) {
    case WMHI_CLOSEWINDOW:
     done = 0;
     break;

    case WMHI_ICONIFY:
//DBUG("WMHI_ICONIFY (win[WID_MAIN]=%p)\n",gui->win[WID_MAIN]);
     if ( IIntuition->IDoMethod(OBJ(OID_MAIN), WM_ICONIFY) ) {
      gui->win[WID_MAIN] = NULL;
     }
     break;

    case WMHI_UNICONIFY:
     gui->win[WID_MAIN] = (struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL);
     if (gui->win[WID_MAIN] != NULL) {
//DBUG("WMHI_UNICONIFY (win[WID_MAIN]=%p)\n",gui->win[WID_MAIN]);
      gui->screen = gui->win[WID_MAIN]->WScreen;
      IIntuition->ScreenToFront(gui->screen);
     } else {
      done = 0;
     }
     break;

    case WMHI_MENUPICK:
     done = Handle_MenuEvents(gui);
     break;

    case WMHI_GADGETUP:
//DBUG("[WMHI_GADGETUP] code = %ld (0x%08lx)\n",code,code);
      events_GadgetUp(gui, (result & WMHI_GADGETMASK));
     break;

    default:
     break;
   } // END of WMHI_CLASSMASK
  }
	} // END of if(siggot & main_sigmask)

	return done;
}
