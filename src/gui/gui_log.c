#include "gui.h"
#include "gui_log.h"
//#include "gui_resources.h"
#include "gui_build.h"
//#include "gui_events.h"
#include "debug.h"


/* Print text to LogWindow (texteditor.gadget) */
void Log_Message(struct CMDGUI *gui, CONST_STRPTR message) {
	IIntuition->IDoMethod(OBJ(OID_TED), GM_TEXTEDITOR_InsertText,
	                      NULL, message, GV_TEXTEDITOR_InsertText_Bottom, TAG_DONE);
	IIntuition->IDoMethod(OBJ(OID_TED), GM_TEXTEDITOR_InsertText,
	                      NULL, "\n", GV_TEXTEDITOR_InsertText_Bottom, TAG_DONE);
	if (gui->win[WID_MAIN]) {
		IIntuition->RefreshGadgets(GAD(OID_TED), gui->win[WID_MAIN], NULL);
	}
}
