
#include "gui.h"
#include "gui_resources.h"
#include "debug.h"


#ifdef __amigaos4__
	#define ACQUIRE_IFACE(n)  I##n = (struct n##IFace *)IExec->GetInterface(gui->n##Base, "main", 1L, NULL);
	#define DISPOSE_IFACE(n)  { IExec->DropInterface( (struct Interface *)I##n ); I##n = NULL; }

	#define ACQUIRE_CLASSES(n,s,v)      { gui->n##Base=open_ResourceClass(gui, s, v, &(n##Class)); if (gui->n##Base == NULL) { return 0; } }
	#define ACQUIRE_CLASSES_LIB(n,s,v)  { gui->n##Base=(struct Library *)open_ResourceClass(gui, s, v, &(n##Class)); if (gui->n##Base == NULL) { return 0; } }
#else 
	#define ACQUIRE_IFACE(n)
	#define DISPOSE_IFACE(n)
	#define ACQUIRE_CLASSES(n,s,v)
	#define ACQUIRE_CLASSES_LIB(n,s,v)
#endif


/*
 * Open gadget VERSION
 * and shows (timed) requester if fails.
 */
static struct ClassLibrary* open_ResourceClass(struct APTGUI *gui, CONST_STRPTR name, uint32 ver, Class **className) {
	struct ClassLibrary *base;
DBUG("%s()\n",__FUNCTION__);
	base = IIntuition->OpenClass(name, ver, className);
	if (base == NULL) {
		IDOS->TimedDosRequesterTags(TDR_Timeout, 20,
		                            TDR_FormatString, "Can't load required class",//GetString(&Li,MSG_ERROR_LIB_REQUIRED),
		                            TDR_GadgetString, "OK",//GetString(&Li,MSG_OK_BTN),
		                            TDR_ImageType, TDRIMAGE_ERROR,
		                            TDR_Arg1, name,
		                            TDR_Arg2, ver,
		                            TDR_Arg3, 0,
		                           TAG_END);
	}

	return base;
}


/*
 * Open library VERSION.REVISON
 * and shows (timed) requester if fails.
 */
static struct Library* open_Resource(struct APTGUI *gui, CONST_STRPTR name, uint32 ver, uint32 rev) {
	struct Library *base;
DBUG("%s() LIB_IS_AT_LEAST(%ld.%ld)\n",__FUNCTION__,ver,rev);
	base = IExec->OpenLibrary(name, ver);
	if ( base==NULL  ||  !LIB_IS_AT_LEAST(base,ver,rev) ) {
		IDOS->TimedDosRequesterTags(TDR_Timeout, 20,
		                            TDR_FormatString, "Can't find required library",//GetString(&Li,MSG_ERROR_LIB_REQUIRED),
		                            TDR_GadgetString, "OK",//GetString(&Li,MSG_OK_BTN),
		                            TDR_ImageType, TDRIMAGE_ERROR,
		                            TDR_Arg1, name,
		                            TDR_Arg2, ver,
		                            TDR_Arg3, rev,
		                           TAG_END);
		IExec->CloseLibrary(base);
		return NULL;
	}

	return base;
}


static int open_Classes(struct APTGUI *gui) {
	gui->RequesterBase = open_ResourceClass(gui, "requester.class", 52, &RequesterClass); // not mandatory, see Do_Message()

	ACQUIRE_CLASSES(Space,      "gadgets/space.gadget", 52)
	//ACQUIRE_CLASSES(CheckBox,   "gadgets/checkbox.gadget", 52)
	ACQUIRE_CLASSES(Button,     "gadgets/button.gadget", 52)
	ACQUIRE_CLASSES(Label,      "images/label.image", 52)
	//ACQUIRE_CLASSES(GetFile,    "gadgets/getfile.gadget", 52)
//	ACQUIRE_CLASSES(Slider,     "gadgets/slider.gadget", 52)
	//ACQUIRE_CLASSES(TextEditor, "gadgets/texteditor.gadget", 52)
	//ACQUIRE_CLASSES(Scroller,   "gadgets/scroller.gadget", 52)
	//ACQUIRE_CLASSES(FuelGauge,  "gadgets/fuelgauge.gadget", 52)
	ACQUIRE_CLASSES(String,     "gadgets/string.gadget", 52)
	ACQUIRE_CLASSES(Window,     "window.class", 52)

	// Casted as '(struct Library *)'
	ACQUIRE_CLASSES_LIB(ListBrowser, "gadgets/listbrowser.gadget", 53)
	ACQUIRE_CLASSES_LIB(Layout,      "gadgets/layout.gadget", 52)
	//ACQUIRE_CLASSES_LIB(Chooser,     "gadgets/chooser.gadget", 52)
	//ACQUIRE_CLASSES_LIB(ClickTab,    "gadgets/clicktab.gadget", 53)

	// Open gadgets's IFaces
	ACQUIRE_IFACE(ListBrowser)
	//ACQUIRE_IFACE(Layout)
	//ACQUIRE_IFACE(Chooser)
	//ACQUIRE_IFACE(ClickTab)

	return 1;
}


/*
 * Get screen at front. Code by Fredrik Wikstrom (aka salas00)
 * -Used by Do_Message()
 */
struct Screen* FrontMost_Scr(struct APTGUI *gui) {
	struct Screen        *front_screen_address, *public_screen_address = NULL;
	struct List          *public_screen_list;
	struct PubScreenNode *public_screen_node;
	ULONG                 intuition_lock;

	intuition_lock = IIntuition->LockIBase(0L);

	front_screen_address = ((struct IntuitionBase *)gui->IntuitionBase)->FirstScreen;
	if( (front_screen_address->Flags & PUBLICSCREEN) || (front_screen_address->Flags & WBENCHSCREEN) ) {
		IIntuition->UnlockIBase(intuition_lock);

		public_screen_list = IIntuition->LockPubScreenList();
		public_screen_node = (struct PubScreenNode *)public_screen_list->lh_Head;
		while (public_screen_node != NULL) {
			if (public_screen_node->psn_Screen == front_screen_address) {
				public_screen_address = public_screen_node->psn_Screen;
				break;
			}

			public_screen_node = (struct PubScreenNode *)public_screen_node->psn_Node.ln_Succ;
		}

		IIntuition->UnlockPubScreenList();
	} else {
		IIntuition->UnlockIBase(intuition_lock);
	}

	if (public_screen_address == NULL) {
		public_screen_address = IIntuition->LockPubScreen(NULL);
		IIntuition->UnlockPubScreen(NULL, public_screen_address);
	}

//DBUG("%lx\n", (int)public_screen_address);
	return public_screen_address;
}


/*
 * Show Errors/Warnings/... in a requester and "disables" main window gagdets.
 * -Returns the number of the button user selected.
 * -The buttons are numbered 1, 2, 3, ..., 0. A single button req always returns 0.
 * -Use EasyRequestArgs() if requester.gadget not (yet) available.
 */
uint32 VARARGS68K Do_Message(struct APTGUI *gui, char reqimg, CONST_STRPTR buttons, CONST_STRPTR message, ...)
{
	va_list  args;
	APTR    *vargs = NULL;
	uint32   btn_sel;

	if (IIntuition == NULL) {
		IDOS->PutErrStr(message);
		return 0;
	}

	va_startlinear(args, message);
	vargs = va_getlinearva(args, APTR *);

	// "Disabling" main window gadgets
	//IIntuition->SetAttrs(OBJ(OID_MAIN), WA_BusyPointer,TRUE, TAG_DONE);

	if (gui->RequesterBase != NULL) {
		Object *requester;

		IUtility->SNPrintf(gui->loc_buffer, MAX_LOCBUF, "\033c%s",message); // centers text

		requester = IIntuition->NewObject(RequesterClass, NULL, //"requester.class",
		                                  REQ_Image,      reqimg,
		                                  REQ_TitleText,  "APT_GUI",//PROGNAME,
		                                  REQ_BodyText,   gui->loc_buffer,
		                                  REQ_VarArgs,    vargs,
		                                  REQ_GadgetText, buttons? buttons : "OK",//GetString(&Li,MSG_OK_BTN),
		                                  //REQ_StayOnTop, TRUE,
		                                 TAG_DONE);

		btn_sel = IIntuition->IDoMethod( requester, RM_OPENREQ, NULL, NULL, FrontMost_Scr(gui) );
		IIntuition->DisposeObject(requester);
	} else {
		struct EasyStruct es;

		es.es_StructSize   = sizeof(struct EasyStruct);
		es.es_Flags        = 0L;
		es.es_Title        = "APT_GUI",//PROGNAME;
		es.es_TextFormat   = message;
		es.es_GadgetFormat = buttons? buttons : "OK",//GetString(&Li,MSG_OK_BTN),

		btn_sel = IIntuition->EasyRequestArgs(NULL, &es, 0L, vargs);
	}

	// "Re-enabling" main window gadgets
	//IIntuition->SetAttrs(OBJ(OID_MAIN), WA_BusyPointer,FALSE, TAG_DONE);

	va_end(args);

//DBUG("  button selected = %ld\n",btn_sel)
	return btn_sel;
}


int Open_Resources(struct APTGUI *gui)
{
DBUG("%s()\n",__FUNCTION__);
	gui->DOSBase = IExec->OpenLibrary("dos.library", 52);
	ACQUIRE_IFACE(DOS)

	gui->UtilityBase = IExec->OpenLibrary("utility.library", 52);
	ACQUIRE_IFACE(Utility)

	gui->IntuitionBase = IExec->OpenLibrary("intuition.library", 52);
	ACQUIRE_IFACE(Intuition)

//	gui->ExpansionBase = IExec->OpenLibrary("expansion.library", 52);
//	ACQUIRE_IFACE(Expansion);

	/*Li.li_Catalog = NULL;
	if ( (gui->LocaleBase=IExec->OpenLibrary("locale.library", 52))
	    &&  (gui->Ilocale=(struct LocaleIFace *)IExec->GetInterface(gui->LocaleBase, "main", 1, NULL)) ) {
		Li.li_ILocale = gui->Ilocale;
		Li.li_Catalog = ILocale->OpenCatalog(NULL, PROGNAME".catalog",
		                                     OC_BuiltInLanguage, "english",
		                                     OC_PreferExternal, TRUE,
		                                    TAG_END);
	} else {
		IDOS->PutErrStr("Failed to use catalog system. Using built-in strings.\n");
	}*/

//	gui->AslBase = open_Resource(gui, "asl.library", 53, 54);
//	ACQUIRE_IFACE(Asl)

	gui->IconBase = open_Resource(gui, "icon.library", 54, 6);
	ACQUIRE_IFACE(Icon)

//	gui->ApplicationBase = open_Resource(gui, "application.library", 53, 32);
	//ACQUIRE_IFACE(Application)
//	gui->Iprefsobjects = (struct PrefsObjectsIFace *)IExec->GetInterface(gui->ApplicationBase, "prefsobjects", 2, NULL);

//	gui->GraphicsBase = open_Resource(gui, "graphics.library", 54, 0);
//	ACQUIRE_IFACE(Graphics)

//	gui->NVBase = open_Resource(gui, "nonvolatile.library", 52, 0);
//	ACQUIRE_IFACE(NV)

//	gui->DataTypesBase = open_Resource(gui, "datatypes.library", 54, 0);
//	ACQUIRE_IFACE(DataTypes)

	if (gui->DOSBase==NULL  ||  gui->UtilityBase==NULL  ||  gui->IntuitionBase==NULL
	    ||  gui->IconBase==NULL  ||  open_Classes(gui)==0) {
		//IExec->Alert(AT_Recovery|AG_OpenLib);
		//Do_Message( gui, REQIMAGE_ERROR, GetString(&Li,MSG_OK_BTN), GetString(&Li,MSG_ERROR_RESOURCES_REQUIRED) );
		Do_Message( gui, REQIMAGE_ERROR, NULL, "Can't find required resources");//GetString(&Li,MSG_ERROR_RESOURCES_REQUIRED) );
		return 0;
	}

	return 1;
}


void Close_Resources(struct APTGUI *gui)
{
DBUG("%s()\n",__FUNCTION__);
	if (IIntuition != NULL) {
		IIntuition->CloseClass(gui->RequesterBase);
		IIntuition->CloseClass(gui->SpaceBase);
		IIntuition->CloseClass(gui->LabelBase);
		//IIntuition->CloseClass(gui->GetFileBase);
//		IIntuition->CloseClass(gui->SliderBase);
		//IIntuition->CloseClass(gui->TextEditorBase);
		//IIntuition->CloseClass(gui->ScrollerBase);
//		IIntuition->CloseClass(gui->FuelGaugeBase);
		IIntuition->CloseClass(gui->StringBase);
		IIntuition->CloseClass(gui->ButtonBase);
//		IIntuition->CloseClass(gui->CheckBoxBase);
		IIntuition->CloseClass(gui->WindowBase);

//		DISPOSE_IFACE(ClickTab)
//		IIntuition->CloseClass( (struct ClassLibrary *)gui->ClickTabBase );

		//DISPOSE_IFACE(Chooser)
		//IIntuition->CloseClass( (struct ClassLibrary *)gui->ChooserBase );

		//DISPOSE_IFACE(Layout)
		IIntuition->CloseClass( (struct ClassLibrary *)gui->LayoutBase );

		DISPOSE_IFACE(ListBrowser)
		IIntuition->CloseClass( (struct ClassLibrary *)gui->ListBrowserBase );
	}
/*
	if (ILocale != NULL) {
		ILocale->CloseCatalog(Li.li_Catalog);
		DISPOSE_IFACE(Locale)
	}
	IExec->CloseLibrary( (struct Library *)gui->LocaleBase );
*/
	//finish_TimerDevice(gui);

//	DISPOSE_IFACE(DataTypes)
//	IExec->CloseLibrary(gui->DataTypesBase);

	//DISPOSE_IFACE(Graphics)
	//IExec->CloseLibrary(gui->GraphicsBase);

//	DISPOSE_IFACE(PrefsObjects)
//	IExec->CloseLibrary(gui->ApplicationBase);

	DISPOSE_IFACE(Icon)
	IExec->CloseLibrary(gui->IconBase);

	//DISPOSE_IFACE(Asl)
	//IExec->CloseLibrary(gui->AslBase);

//	DISPOSE_IFACE(Expansion)
//	IExec->CloseLibrary(gui->ExpansionBase);

	DISPOSE_IFACE(Intuition)
	IExec->CloseLibrary(gui->IntuitionBase);

	DISPOSE_IFACE(Utility)
	IExec->CloseLibrary(gui->UtilityBase);

	DISPOSE_IFACE(DOS)
	IExec->CloseLibrary(gui->DOSBase);
}
