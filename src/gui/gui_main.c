;/*
gcc gui_main.c -o gui -gstabs -mcrt=clib4 -Wall -DDEBUG
quit

https://github.com/AmigaLabs/clib4/wiki/Recommended-C-style-and-coding-rules
*/


#include "gui.h"
#include "gui_resources.h"
#include "gui_build.h"
#include "debug.h"
#include "gui_rev.h"
//#include "../apt.h"


CONST USED TEXT version[]    = VERSTAG" apt v0.3 (" __DATE__ ")";
CONST USED TEXT stk_cookie[] = "$STACK:64000";


#ifdef _WITH_GUI_
int gui_main(struct WBStartup *wbs) {
#else
int main(int argc, char *argv[]) {
#endif
	struct APTGUI *gui;
DBUG("%s() - START\n",__FUNCTION__);

#ifndef _WITH_GUI_
	if(argc != 0) { // NOT from WB
		return RETURN_ERROR;
	}
#endif

	gui = IExec->AllocVecTags(sizeof(struct APTGUI), AVT_ClearWithValue,0,
	                          AVT_Type,MEMF_SHARED, AVT_Lock,FALSE, TAG_END);
	if(gui == NULL) {
		return RETURN_FAIL;
	}

	gui->loc_buffer = IExec->AllocVecTags(MAX_LOCBUF, AVT_ClearWithValue,NULL, TAG_END);

	if (Open_Resources(gui) != 0) {
/*#ifdef _WITH_GUI_
		gui->wbs = wbs;
#else
		gui->wbs = (struct WBStartup*)argv;
#endif*/
		Launch_GUI(gui);
	}

	Close_Resources(gui);

	IExec->FreeVec(gui->loc_buffer);
	gui->loc_buffer = NULL;

	IExec->FreeVec(gui);
	gui = NULL;

DBUG("%s() - END\n",__FUNCTION__);
	return RETURN_OK;
}
