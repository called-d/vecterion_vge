#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#
prefix = /usr/local


TARGET_ARCH	:= linux

ifeq ($(OS),Windows_NT)
TARGET_ARCH	:= win
MKDIR_P		:= mkdir
else
MKDIR_P		:= mkdir -p
endif

APP_NAME	:= vecterion_vge
SOURCE_DIR	:= source
OBJECT_DIR	:= object/$(TARGET_ARCH)
BUILD_DIR	:= build/$(TARGET_ARCH)
APP_FILE	:= $(BUILD_DIR)/$(APP_NAME).exe

CC		:= gcc
PKG_CONFIG	:= pkg-config

CFLAGS		:= -W -Wall -Wextra
CFLAGS		+= -MMD -MP -g -std=c11
CFLAGS		+= -posix
CFLAGS		+= -lm
CFLAGS		+= -Werror
CFLAGS		+= -Wno-unused-parameter
CFLAGS		+= -Wunused -Wimplicit-function-declaration \
		 -Wincompatible-pointer-types \
		 -Wbad-function-cast -Wcast-align \
		 -Wdisabled-optimization -Wdouble-promotion \
		 -Wformat-y2k -Wuninitialized -Winit-self \
		 -Wlogical-op -Wmissing-include-dirs \
		 -Wshadow -Wswitch-default -Wundef \
		 -Wwrite-strings -Wunused-macros
#CFLAGS		+= -Wmissing-declarations -Wcast-qual -Wconversion -Wno-sign-conversion
#		 -Wswitch-enum -Wjump-misses-init
CFLAGS		+= $(CFLAGS_APPEND)

INCLUDE		:= -I./include
INCLUDE		+= $(shell $(PKG_CONFIG) --libs --cflags gtk+-3.0 libxml-2.0)
INCLUDE		+= $(INCLUDE_APPEND)

ifeq ($(TARGET_ARCH),win)
CFLAGS		+= -DTARGET_ARCH_WIN
endif

SOURCES		:= $(wildcard $(SOURCE_DIR)/*.c) 
OBJECTS		:= $(subst $(SOURCE_DIR),$(OBJECT_DIR),$(SOURCES:.c=.o))
DEPENDS		:= $(OBJECTS:.o=.d)



.PHONY : all run gdb clean dist_clean

all : $(APP_FILE)

$(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@


$(OBJECT_DIR)/main.o : include/version.h
$(APP_FILE) : $(OBJECTS)
	$(MKDIR_P) $(dir $@)
	bash ./version.sh $(OBJECT_DIR)
	$(CC) \
		$^ \
		$(OBJECT_DIR)/version.c \
		$(TARGET_ARCH_WIN_RESOURCE) \
		$(CFLAGS) \
		$(INCLUDE) \
		-o $(APP_FILE)

run : $(APP_FILE)
	$(MKDIR_P) $(OBJECT_DIR)/install
	make install prefix=$(OBJECT_DIR)/install
	./$(OBJECT_DIR)/install/bin/vecterion_vge -i ./library/23.svg

gdb : $(APP_FILE)
	make install prefix=$(OBJECT_DIR)/install
	gdb ./$(OBJECT_DIR)/install/bin/vecterion_vge
	#gdb ./$(APP_FILE)

clean :
	$(RM) $(APP_FILE)
	$(RM) $(OBJECTS)
	$(RM) -r object/ build/

dist_clean :
	$(MAKE) clean
	$(MAKE) test_clean

install: $(APP_FILE)
	install -D $(APP_FILE) $(DESTDIR)$(prefix)/bin/vecterion_vge
	cp -r resource/ $(DESTDIR)$(prefix)/

# test
include test/test.Makefile



-include $(DEPENDS)

