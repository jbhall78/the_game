OS_TYPE = UNIX

# MS-DOS/OpenWatcom specific options
ifeq ($(OS_TYPE),DOS)
CC      = wcc -q
COMPILE = $(CC)
OPT_OUTPUT = -fo=
LINK    = wfl -q
DEBUG   = -DDEBUG
OPTIONS = -DUSE_INT10
CFLAGS  = $(DEBUG) $(OPTIONS)
LDFLAGS = 
LIB_PRE = 
LIB_EXT = lib
OBJ_EXT = obj
DEL_CMD = del
endif
# Unix/GNU specific options
ifeq ($(OS_TYPE),UNIX)
CC      = gcc
COMPILE = $(CC) -c
OPT_OUTPUT = -o
LINK    = $(CC)
DEBUG   = -ggdb -DDEBUG
OPTIONS = -DUSE_CURSES -DUSE_TTY -DUSE_EVENT -DUSE_REALTIME
CFLAGS  = $(DEBUG) -Wall -D_GNU_SOURCE=1 -D_REENTRANT $(OPTIONS)
LDFLAGS = -lrt -lncursesw
LIB_PRE = lib
LIB_EXT = a 
OBJ_EXT = o
DEL_CMD = rm -f
endif

# set the name of the library the linker wants to see
LIB_NAME = game
LIB  = $(LIB_PRE)$(LIB_NAME).$(LIB_EXT)
ifeq ($(OS_TYPE),DOS)
LIB_LINK = $(LIB_NAME)
endif
ifeq ($(OS_TYPE),UNIX)
LIB_LINK = $(LIB)
endif
GAME = game
EDIT = edit

# edit program files
EDIT_O_FILES = edit.$(OBJ_EXT)
EDIT_C_FILES = edit.c
EDIT_H_FILES = edit.h

# game program files
GAME_O_FILES = game.$(OBJ_EXT)
GAME_C_FILES = game.c
GAME_H_FILES = game.h

# library object files
O_FILES = \
	anim.$(OBJ_EXT) \
	clock.$(OBJ_EXT) \
	dbg.$(OBJ_EXT) \
	die.$(OBJ_EXT) \
	input.$(OBJ_EXT) \
	mkstr.$(OBJ_EXT) \
	random.$(OBJ_EXT) \
	screen.$(OBJ_EXT) \
	slist.$(OBJ_EXT) \
	terminal.$(OBJ_EXT) \
	utf8.$(OBJ_EXT) \
	widget.$(OBJ_EXT)

# source files
C_FILES = anim.c clock.c dbg.c die.c input.c mkstr.c random.c slist.c \
	  screen.c terminal.c utf8.c widget.c

# header files
H_FILES = anim.h clock.h dbg.h die.h input.h mkstr.h random.h slist.h \
	  screen.h terminal.h utf8.h widget.h

# all files (to extract tags from)
FILES   = $(C_FILES) $(H_FILES)

all: $(LIB) $(GAME) $(EDIT) tags

ctags: tags

tags: $(FILES)
	ctags -o tags $(FILES)

# link target for DOS
# make sure game.lbc exists and contains "+ obj1 obj2 ..."
# we have to specify the list of object files in a seperate
# file because of command line argument restrictions on DOS
ifeq ($(OS_TYPE),DOS)
$(LIB): $(O_FILES) $(H_FILES)
	$(DEL_CMD) $(LIB)
	wlib -q $(LIB_LINK) @game.lbc
endif

# link target for UNIX
ifeq ($(OS_TYPE),UNIX)
$(LIB): $(O_FILES) $(H_FILES)
	ar r $(LIB) $(O_FILES)
endif

$(GAME): $(LIB) $(GAME_O_FILES)
	$(CC) $(CFLAGS) -o $(GAME) $(GAME_O_FILES) $(LIB) $(LDFLAGS)

$(EDIT): $(LIB) $(EDIT_O_FILES)
	$(CC) $(CFLAGS) -o $(EDIT) $(EDIT_O_FILES) $(LIB) $(LDFLAGS)

clean:
	$(DEL_CMD) $(LIB) $(O_FILES) *.err *.log
	$(DEL_CMD) $(GAME) $(GAME_O_FILES)
	$(DEL_CMD) $(EDIT) $(EDIT_O_FILES)

run: all
	./$(GAME)

# remote GDB debugging, to connect:
# target remote localhost:1234
# I should make a .gdbinit which automatically
# does this
ddd-edit:
	ddd edit &

# remote GDB debugging, run this first
debug-edit:
	gdbserver localhost:1234 ./edit
	
%.obj: %.c $(H_FILES)
	$(COMPILE) $(CFLAGS) $(OPT_OUTPUT)$@ $<
