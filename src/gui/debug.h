/****************************************************************************
**  :ts=4
**
**  debug.h
**
**  Original concept and coding by Javier de las Rivas. Donostia, Spain.
**  Copyright (c)2025, All rights reserved.
**
**  Hyperion Entertainment is granted a free perpetual licence to 
**  distribute the executable and documentation files with any
**  version of Amiga OS4.
**
*****************************************************************************/

#if defined(DEBUG)
	#define DBUG(x,...) IExec->DebugPrintF("[%s:%-4ld] "x ,__FILE__,__LINE__, ##__VA_ARGS__)
	#define DBUGN(x,...) IExec->DebugPrintF(""x ,##__VA_ARGS__)
#else
	#define DBUG(x,...)
	#define DBUGN(x,...)
#endif
