# ----------------------------
# Set NAME to the program name
# Set DEBUGMODE to "DEBUG" to use debug functions
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ** Add all shared library names to L **
# ----------------------------

NAME        ?= FLOWCE
DEBUGMODE   ?= NDEBUG
COMPRESSED  ?= YES
ICON        ?= icon.png
DESCRIPTION ?= "Connect dots, fill spaces."

L ?= graphx fileioc keypadc

# ----------------------------
# Specify source and output locations
# ----------------------------

SRCDIR ?= src
OBJDIR ?= obj
BINDIR ?= bin
GFXDIR ?= src/gfx

# ----------------------------
# Use OS helper functions (Advanced)
# ----------------------------

USE_FLASH_FUNCTIONS ?= YES

include $(CEDEV)/include/.makefile
