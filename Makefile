#------------------------------------------------------------------------------ ---------------------------------------
SHELL = /bin/sh

#------------------------------------------------------------------------------
# Choose! Are you making an exe, an archive (static library), or a DLL (dynamic library)
#
BLDTYPE=exe
#BLDTYPE=arc
#BLDTYPE=dll # TODO : Add support for dynamic libraries

#------------------------------------------------------------------------------
# Give your tool a name!
# All file extensions are appended later
#
TOOLNAME = tictac
TESTPRAM = -6 -W -b +78

#------------------------------------------------------------------------------
# Commands (for future cross platform support)
#
GETDEPS = gcc -M
LINK    = gcc -o
MAKE    = make
DEL     = rm -f
COPY    = cp
CAT     = cat
MD      = mkdir -p
RMF     = rm -f
RMRF    = rm -rf
ECHO    = /bin/echo -e
AR      = ar -crfvs
TAR     = tar -cvf
TARGZ   = tar -cvzf
TARBZ   = tar -cvjf
SEDI    = sed -i
READELF = readelf
SVNVER  = svnversion . -n | sed s/^[0-9]*://
SVNTMP  = ~svntmp.tmp

UNAME_A = uname -a
UNAME_M = uname -m
OSREL   = lsb_release -crid
WHO     = id -un
WHERE   = hostname -A
WHEN    = date +%Y-%m-%d@%H:%M:%S

#------------------------------------------------------------------------------
# ANSI colours
#
aBLK=\e[30m
aRED=\e[31m
aGRN=\e[32m
aYEL=\e[33m
aBLU=\e[34m
aMAG=\e[35m
aCYN=\e[36m
aWHT=\e[37m

aONBLK=\e[40m
aONRED=\e[41m
aONGRN=\e[42m
aONYEL=\e[43m
aONBLU=\e[44m
aONMAG=\e[45m
aONCYN=\e[46m
aONWHT=\e[47m

aNRM=\e[0m
aBRT=\e[1m
aUNL=\e[4m
aFLS=\e[5m
aINV=\e[7m
aHID=\e[8m

tPRE =$(aNRM)$(aBRT):-----[
tPOST=$(aNRM)$(aBRT)]-----:$(aNRM)

#------------------------------------------------------------------------------
# MACROs used by the source code, generated at compile time
#
VER_RCS  = $(shell $(SVNVER))
VER_DAT  = $(shell $(DATE))

CCDEF    = -DTOOLNAME="$(TOOLNAME)"  -DVER_SVN="$(VER_RCS)" -DVER_DAT="$(VER_DAT)"

#------------------------------------------------------------------------------
# Files, directories & libraries in use
#
ARCH   = $(shell $(UNAME_M))

# Make sure ALL of these are ALWAYS define (see 'clean' & 'tidy' rules)
BINROOT = bin
BINDIR  = $(BINROOT)/$(ARCH)
LIBDIR  = $(BINDIR)/lib
INCDIR  = $(BINDIR)/inc

EXE  = $(BINDIR)/$(TOOLNAME)
ARC  = $(LIBDIR)/$(TOOLNAME).a
SRC  = $(wildcard *.c)
HDR  = $(wildcard *.h)
MISC =
LIBS = 
#-lncurses

TARBALL = $(TOOLNAME)_$(VER_RCS)_$(ARCH).tar.gz

# Build files
BLDROOT = b
BDIR    = $(BLDROOT)/$(ARCH)
OBJ     = $(SRC:%.c=$(BDIR)/%.o)
DEP     = $(SRC:%.c=$(BDIR)/%.d) $(HDR:%.h=$(BDIR)/%.hd)

# Library headers
LIBHDR = $(HDR:%=$(INCDIR)/%)

#------------------------------------------------------------------------------
# Description file
#
ifeq ($(BLDTYPE), exe)
	TARGET  = $(EXE)
	DIZDIR  = $(BINDIR)
	ELFINFO = $(READELF) -sd $(EXE)
	MOREELF = $(READELF) -sd $(EXE) | grep -e NEEDED -e FUNC.*GLOBAL.*UND

else ifeq ($(BLDTYPE), arc)
	TARGET  = $(ARC)
	DIZDIR  = $(LIBDIR)
	ELFINFO = $(READELF) -sh $(ARC)
	MOREELF = $(ELFINFO) | grep -e ^F -e FILE -e FUNC | grep -v FUNC.*LOCAL

else ifeq ($(BLDTYPE), dll)
	@$(ECHO) "Broken Makefile : Dynamically Linked Library (DLL) support not written yet."
	false

else
	@$(ECHO) "Broken Makefile : Unknown Build Type - BLDTYPE=exe|arc|dll"
	false

endif

DIZ = $(DIZDIR)/$(TOOLNAME).diz

#------------------------------------------------------------------------------
# Compiler directives
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# gcc -Q --help=warning | sed -e 's/^\s*\(\-\S*\)\s*\[\w*\]/\1 /gp;d' | tr -d '\n'
#
#CERR = -Wall -Wextra -Wformat -Werror
#CFLG = -std=c99
CMP  = -c
DBG  = $(CMP) -g
CC   = gcc $(CCDEF) $(CFLG) $(DBG) $(CERR)

#------------------------------------------------------------------------------
# Default make rule
.PHONY : default
default: help

#------------------------------------------------------------------------------
.PHONY : help
help :
	@$(ECHO) "Available make rules"
	@$(ECHO) "\thelp      : This help"
	@$(ECHO) ""
	@$(ECHO) "\tall       : Build binary to: $(BINDIR)"
	@$(ECHO) "\trun       : Test binary with: $(EXE) $(TESTPRAM)"
	@$(ECHO) ""
	@$(ECHO) "\ttidy      : Erase build directory: $(BDIR)"
	@$(ECHO) "\tclean     : Erase build and binary dir: $(BDIR), $(BINDIR)"
	@$(ECHO) ""
	@$(ECHO) "\tdiz       : Create dizcription file: $(DIZ)"
	@$(ECHO) "\ttarball   : Create archive of source & binary: $(TARBALL)"
	@$(ECHO) ""
	@$(ECHO) "\tsvnignore : Set SVN to ignore temp files"
	@$(ECHO) "\tsvnadd    : Add $(ARCH) build to svn repo"

#------------------------------------------------------------------------------
all : $(TARGET) $(DIZ)

#------------------------------------------------------------------------------
run : $(EXE)
	@$(ECHO) "$(tPRE)$(aRED) RUN: $(EXE) $(tPOST)"
	[ -x $(EXE) ] && ./$(EXE) $(TESTPRAM)

#------------------------------------------------------------------------------
.PHONY : tidy
tidy :
	@$(ECHO) "$(tPRE)$(aGRN) TIDY $(tPOST)"
	$(RMRF) $(BDIR)/

.PHONY : clean
clean :
	@$(ECHO) "$(tPRE)$(aGRN) CLEAN $(tPOST)"
	$(RMRF) $(BDIR)/ $(BINDIR)/ $(LIBDIR)/ $(INCDIR)/

#------------------------------------------------------------------------------
# Create Dizcription file (when target updates)
#
.PHONY : diz
diz $(DIZ) : $(TARGET)
	@$(ECHO) "$(tPRE)$(aCYN) DIZCRIPTION: $@ $(tPOST)"
	@ >$(DIZ) $(ECHO) -n
	@>>$(DIZ) $(ECHO) "timstamp      : $(shell $(WHEN))"
	@>>$(DIZ) $(ECHO) "whoami@host   : $(shell $(WHO))@$(shell $(WHERE))"
	@>>$(DIZ) $(ECHO) "$(UNAME_A)      : $(shell $(UNAME_A))"
	@>>$(DIZ) $(OSREL)

	@$(CAT) $(DIZ)

# This will not be helpful output during compile
	@>>$(DIZ) $(ECHO) "----------------------------------------"
	$(MOREELF) >>$(DIZ)
	@>>$(DIZ) $(ECHO) "----------------------------------------"
	$(ELFINFO) >>$(DIZ)

	@$(ECHO) "----------------------------------------"

#------------------------------------------------------------------------------
.PHONY : svnadd
svnadd :
	svn --force --parents --no-ignore add $(EXEDIR)
	svn ci $(EXEDIR) -m "Checkin of $(ARCH) compiled code"

.PHONY : svnignore
svnignore :
	$(MD) $(BDIR)
	@ > $(BDIR)/$(SVNTMP) $(ECHO) -n
	@>> $(BDIR)/$(SVNTMP) $(ECHO) $(TMP)
	@>> $(BDIR)/$(SVNTMP) $(ECHO) $(BDIR)
#	@>> $(BDIR)/$(SVNTMP) $(ECHO) $(BINDIR)
	svn propset svn:ignore -F  $(BDIR)/$(SVNTMP) .

#------------------------------------------------------------------------------
# Archve & Link rules
#
$(EXE) : $(OBJ) $(DEP)
	@$(ECHO) "$(tPRE)$(aYEL) LINK: $@ $(tPOST)"
	$(MD) $(BINDIR)
	$(LINK) $(EXE) $(OBJ) $(LIBS)

$(ARC) : $(OBJ) $(DEP) $(LIBHDR)
	@$(ECHO) "$(tPRE)$(aYEL) ARCHIVE: $@ $(tPOST)"
	$(MD) $(LIBDIR)
	$(AR) $(ARC) $(OBJ)

#$(DLL) : $(OBJ) $(DEP) $(LIBHDR)
#	@$(ECHO) "$(tPRE)$(aYEL) ARCHIVE: $@ $(tPOST)"
#	$(MD) $(LIBDIR)
#	????

#------------------------------------------------------------------------------
.PHONY : tarball
tarball : $(EXE) $(DIZ) $(ARC) $(LIBHDR) $(SRC) $(HDR) $(MISC) Makefile
	@$(ECHO) "$(tPRE)$(aWHT) TARBALL: $(TARBALL) $(tPOST)"
	$(RMF) $(TARBALL)
	$(TAR) $(TARBALL) $^

#------------------------------------------------------------------------------
# 'Install' the .h files in the lib directory (via the .hd)
#
$(INCDIR)/%.h : $(BDIR)/%.hd
	@$(ECHO) "$(tPRE)$(aYEL) HEADER: $@ $(tPOST)"
	@$(MD) $(INCDIR)
	$(COPY) $(notdir $(<:.hd=.h)) $(dir $@)

#------------------------------------------------------------------------------
# Compile .c (via the .d)
#
$(BDIR)/%.o : $(BDIR)/%.d
	@$(ECHO) "$(tPRE)$(aCYN) COMPILE: $@ $(tPOST)"
	@$(MD) $(BDIR)
	$(CC) -o $@ $(notdir $(<:.d=.c))

#------------------------------------------------------------------------------
# Create dependency from .h
#
$(BDIR)/%.hd : %.h
	@$(ECHO) "$(tPRE)$(aWHT) DEPS: $@ $(tPOST)"
	@$(MD) $(BDIR)
	@$(DEL) $@
	$(GETDEPS) $< > $@
	@$(SEDI) 's,\($*\)\(\.o\)[ :]*,$(INCDIR)/\1.h $@ : ,g'  $@

#------------------------------------------------------------------------------
# Create dependency from .c
#
$(BDIR)/%.d : %.c
	@$(ECHO) "$(tPRE)$(aWHT) DEPS: $@ $(tPOST)"
	@$(MD) $(BDIR)
	@$(DEL) $@
	$(GETDEPS) $< > $@
	@$(SEDI) 's,\($*\)\.o[ :]*,\1.o $@ : ,g'  $@

#------------------------------------------------------------------------------
ifneq (,$(DEP))
  include $(DEP)
endif
