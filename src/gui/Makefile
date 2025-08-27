
CC    = SDK:gcc/bin/gcc
LD    = SDK:gcc/bin/gcc
RM    = delete
RMDIR = delete force quiet all
CAT   = type
STRIP = strip
CATCOMP = catcomp


OBJ = gui_main.o gui_resources.o gui_build.o gui_events.o gui_menu.o


BIN = gui
VERSION = 1
LASTREV = $(shell c:type $(BIN)_rev.rev)


OS := $(shell uname)

ifeq ($(strip $(OS)),AmigaOS)
	AMIGADATE = $(shell c:date LFORMAT %d.%m.%Y)
	#YEAR = $(shell c:date LFORMAT %Y)
else
	AMIGADATE = $(shell date +"%-d.%m.%Y")
	#YEAR = $(shell date +"%Y")
endif

DEBUG = -DDEBUG


INCPATH = -I. -Iincludes

CFLAGS = -mcrt=clib4 $(DEBUG) $(INCPATH) -Wall -D__AMIGADATE__=\"$(AMIGADATE)\" -gstabs

LDFLAGS = #-nostartfiles

LIBS = 
#	add any extra linker libraries you want here


# -------------------------------------------------------------
# Nothing should need changing below this line



.PHONY: all all-before all-after clean clean-custom realclean

all: all-before $(BIN) all-after

all-before: #klm_strings.h
#	You can add rules here to execute before the project is built

all-after:
#	You can add rules here to execute after the project is built


clean: clean-custom
	$(RM) $(OBJ)

realclean:
	$(RM) $(OBJ) $(BIN) $(BIN)_debug $(BIN).map
#apt_strings.h


$(BIN): $(OBJ) $(LIBS)
#	You may need to move the LDFLAGS variable in this rule depending on its contents
	@echo "Linking $(BIN)"
	@$(LD) -o $(BIN)_debug $(OBJ) $(LDFLAGS) $(LIBS) -Wl,--cref,-M,-Map=$@.map
	C:Copy $(BIN)_debug $(BIN) FORCE
#	@$(STRIP) $(BIN)_debug -o $(BIN)


.PHONY: revision
revision:
	bumprev -e si $(VERSION) $(BIN)

.PHONY: bumpdate
bumpdate:
	bumprev -e si -r $(LASTREV) $(VERSION) $(BIN)


#.PHONY: autodocs
#autodocs:
#	-$(CAT) main.c >t:autodocs
#	autodoc -C    t:autodocs >MachineInfo.doc
#	-$(RMDIR)     t:autodocs


#.PHONY: catalogs
#catalogs:
#	$(CATCOMP) apt.cd locale_source/apt_ES.ct CATALOG catalogs/spanish/apt.catalog


###################################################################
##  Standard rules
###################################################################

# A default rule to make all the objects listed below
# because we are hiding compiler commands from the output

#apt_strings.h: apt.cd
#	APPDIR:CatComp $< CFILE $@
#	make catalogs

.c.o:
	@echo "Compiling $<"
	@$(CC) -c $< -o $*.o $(CFLAGS)

gui_main.o: gui_main.c gui_resources.h gui_build.h gui.h #apt_rev.h apt_strings.h

gui_resources.o: gui_resources.c gui_resources.h gui.h #apt_strings.h

gui_build.o: gui_build.c gui_build.h gui.h #apt_rev.h apt_strings.h

gui_menu.o: gui_menu.c gui_menu.h gui_build.h gui.h #apt_rev.h apt_strings.h


###################################################################
