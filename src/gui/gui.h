#ifndef GUI_H
#define GUI_H


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/icon.h>

#include <proto/listbrowser.h>


#define PROGNAME  "APT-GUI"


struct myToolTypes {
	BOOL show_hints;  // SHOW_HINTS
};
//#define DISABLE_TT "*"      // "special token" used in SaveToolTypes() to..
//#define DISABLE_TT_CHAR '*' // ..disable tooltype: tooltype[=value] -> (tooltype[=value])


#define MAX_LOCBUF  MAX_DOS_PATH+256


enum {
 WID_MAIN = 0,
 //WID_PREFS,

 LAST_WIN
};

enum {
 OID_MAIN = 0,
//MAIN WINDOW
 OID_APTLIST,
 OID_BUTTONSRIGHT_GROUP,
 OID_INSTALL,
 OID_UNINSTALL,
 OID_REMOVE,
 OID_SEARCH_STR,
 //OID_SEARCH,
 //OID_BUTTONSDOWN_GROUP,
 OID_UPDATE,
 OID_UPGRADE,
 //OID_LIST,
 OID_ENTRIES_CNT,
//MENU
 OID_MENU,

 LAST_OID
};


#define CNT_ACT 0 // used in gui->counter[]
#define CNT_TOT 1 // used in gui->counter[]


struct APTGUI {
 //struct Library  *NewlibBase;

 struct ExecBase *ExecBase;
 struct Library  *DOSBase;
 struct Library  *IntuitionBase;
 struct Library  *UtilityBase;
 //struct Library  *ExpansionBase;
 struct Library  *IconBase;
 //struct Library  *AslBase;
 struct Library  *LocaleBase;
// struct Library  *ApplicationBase;
 //struct Library  *VersionBase;
 //struct Library  *GraphicsBase;
 //struct Library  *NVBase;
 //struct Library  *DataTypesBase;
 //struct Device   *TimerBase;

 struct ExecIFace       *Iexec;
 struct DOSIFace        *Idos;
 struct IntuitionIFace  *Iintuition;
 struct UtilityIFace    *Iutility;
 //struct ExpansionIFace  *Iexpansion;
 struct IconIFace       *Iicon;
 //struct AslIFace        *Iasl;
	struct LocaleIFace     *Ilocale;
// struct PrefsObjectsIFace *Iprefsobjects;
 //struct ApplicationIFace  *Iapplication;
 //struct VersionIFace    *Iversion;
 //struct GraphicsIFace   *Igraphics;
 //struct NVIFace         *Inv;
 //struct DataTypesIFace  *Idatatypes;
 //struct TimerIFace      *Itimer;

 struct Library *ListBrowserBase, *LayoutBase, *ChooserBase;//, *ClickTabBase;
// the class library base
 struct ClassLibrary *ButtonBase, /**CheckBoxBase,*/ *LabelBase, *WindowBase,
                     *RequesterBase, *SpaceBase, *GetFileBase, /**SliderBase,*/
                     *TextEditorBase, *ScrollerBase, *StringBase/*, *FuelGaugeBase*/;
// the class pointer
 Class *ListBrowserClass, *LayoutClass, *ChooserClass, /**ClickTabClass,*/ *ButtonClass,
       /**CheckBoxClass,*/ *LabelClass, *WindowClass, *RequesterClass, *SpaceClass,
       *GetFileClass, /**SliderClass,*/ *TextEditorClass, *ScrollerClass, *StringClass/*,
       *FuelGaugeClass*/;
// some interfaces needed
 struct ListBrowserIFace *Ilistbrowser;
 //struct LayoutIFace      *Ilayout;
 //struct ChooserIFace     *Ichooser;
 //struct ClickTabIFace    *Iclicktab;

 Object *Objects[LAST_OID];

 struct Screen      *screen; // PUSBCREEN=<screen_name>
 struct MsgPort     *AppPort;
 struct Window      *win[LAST_WIN];
 struct myToolTypes  myTT;
 struct DiskObject  *micon;
 struct WBStartup   *wbs;
 STRPTR              loc_buffer; // LOCale buffer
 // listbrowser
 struct ColumnInfo *apt_ColInfo;
 struct List       *apt_list;
 int32              counter[2]; // selected / total

 uint32 userdata;
};

//extern struct Interface *INewlib;
//#define IExec         gui->Iexec
#define IDOS          gui->Idos
#define IIntuition    gui->Iintuition
#define IUtility      gui->Iutility
//#define IExpansion    gui->Iexpansion
#define IIcon         gui->Iicon
//#define IAsl          gui->Iasl
#define ILocale       gui->Ilocale
//#define IPrefsObjects gui->Iprefsobjects
//#define IApplication  gui->Iapplication
//#define IVersion      gui->Iversion
//#define IGraphics     gui->Igraphics
//#define INV           gui->Inv
//#define IDataTypes    gui->Idatatypes
//#define ITimer        gui->Itimer

#define IListBrowser  gui->Ilistbrowser
//#define ILayout       gui->Ilayout
//#define IChooser      gui->Ichooser
//#define IClickTab     gui->Iclicktab

#define ListBrowserClass gui->ListBrowserClass
#define ButtonClass      gui->ButtonClass
#define LabelClass       gui->LabelClass
//#define CheckBoxClass    gui->CheckBoxClass
//#define ChooserClass     gui->ChooserClass
#define LayoutClass      gui->LayoutClass
#define WindowClass      gui->WindowClass
#define RequesterClass   gui->RequesterClass
#define SpaceClass       gui->SpaceClass
//#define GetFileClass     gui->GetFileClass
//#define SliderClass      gui->SliderClass
#define TextEditorClass  gui->TextEditorClass
#define ScrollerClass    gui->ScrollerClass
#define StringClass      gui->StringClass
//#define ClickTabClass    gui->ClickTabClass
//#define FuelGaugeClass   gui->FuelGaugeClass


#endif
