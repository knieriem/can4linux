#
#
# can4linux -- LINUX CAN device driver Makefile
# 
# Copyright (c) 2001/2/3 port GmbH Halle/Saale
# (c) 2001/2/3 Heinz-J�rgen Oertel (oe@port.de)
#
# to compile the can4linux device driver,
# please select some configuration variables.
# eg. the target hardware                       TARGET=
# with or without compiled debug-support        DEBUG= 
#
# Modified and extended to support the esd elctronic system
# design gmbh PC/104-CAN Card (www.esd-electronics.com)
# by Jean-Jacques Tchouto (tchouto@fokus.fraunhofer.de) 
#
# SSV TRM/816 support by Sven Geggus <geggus@iitb.fraunhofer.de>
#
# $Id: Makefile,v 1.1.1.1 2003/02/17 17:44:45 jjt Exp $ 
# 
#

# Used release tag for this software version
VERSION=3
REL=1
RELEASE=CAN4LINUX-$(VERSION)_$(REL)
DVERSION=$(VERSION).$(REL)

# be prepared for RTLinux
LINUXTARGET=LINUXOS
#LINUXTARGET=RTLinux

KVERSION= $(shell uname -r)
CONFIG := $(shell uname -n)

CTAGS =	ctags --c-types=dtvf 
CTAGS =	elvtags -tsevl

TITLE = CAN driver can4linux

#
# The CAN driver major device number
# development starts with major=63
#  (LOCAL/EXPERIMENTAL USE)
# The new linux/Documentation/devices.txt defines major=91
CAN_MAJOR=	63
CAN_MAJOR=	91

# definitions for the hardware target
#########################################################################
# Only AT-CAN-MINI can be compiled for 82c200 or PeliCAN mode.
# All other Targets are assuming to have an SJA1000
# CPC_PCI  implies PeliCAN
## 
## Supported TARGETS= IME_SLIMLINE ATCANMINI_PELICAN CPC_PCI IXXAT_PCI03 
##        PCM3680 PC104_200
## 
## compile DigiTec FC-CAN as ATCANMINI_PELICAN
##
#TARGET=IXXAT_PCI03
#TARGET=IME_SLIMLINE
#TARGET=PCM3680
#TARGET=PC104_200
TARGET=ATCANMINI_PELICAN
#TARGET=CPC_PCI
#TARGET=TRM816


TARGET_MATCHED = false
# location of the compiled objects and the final driver module 
OBJDIR = obj

# Debugging Code within the driver
# to use the Debugging option
# and the Debugging control via /proc/sys/Can/DbgMask
# the Makefile in subdir Can must called with the DEBUG set to
# DEBUG=1
# else
# NODEBUG
# doesn't compile any debug code into the driver
DEBUG=NODEBUG
DEBUG=DEBUG=1

# all definitions for compiling the sources
# CAN_PORT_IO		- use port I/O instead of memory I/O
# CAN_INDEXED_PORT_IO   - CAN registers adressed by a pair of registers
#			  one is selcting the register the other one does i/O
#			  used eg. on Elan CPUs
# CAN4LINUX_PCI
# IODEBUG               - all register write accesses are logged
# CONFIG_TIME_MEASURE=1 - enable Time measurement at parallel port
#



ifeq "$(TARGET)" "CPC_PCI"
# CPC-PCI PeliCAN  PCI (only with SJA1000) ------------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR)\
	-DCAN4LINUX_PCI \
	-DCAN_SYSCLK=8

	#-DIODEBUG

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "ATCANMINI_PELICAN"
# AT-CAN-MINI PeliCAN ISA (only with SJA1000) --------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO \
	-DCAN_SYSCLK=8
	#-DCONFIG_TIME_MEASURE=1

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IXXAT_PCI03"
# IXXAT PC-I 03 board ISA (only with SJA1000) ---------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "PCM3680"
# Advantech PCM3680 board ISA (only with SJA1000) ----------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "TRM816"
# TRM816 Onboard CAN-Controller (only with SJA1000) --------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_INDEXED_PORT_IO \
	-DCAN_SYSCLK=10

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "PC104_200"
# ESD PC104-200 PC104 board (with SJA1000) ----------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO -DPC104 \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IME_SLIMLINE"
# I+ME  PcSlimline ISA (only with SJA1000) -----------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(LINUXTARGET)" "LINUXOS"
#use normal Linux OS
LIBS   =
CAN_MODULE = Can.o
PROJECTHOME=$(shell pwd)
INSTALLHOME=/usr/src
endif
 


# TARGET=ELIMA selcts the powerpc-linux crosscompiler

# which sets defines like PPC, __PPC__, __PPC

ifeq "$(TARGET)" "ELIMA"
CC:=		powerpc-linux-gcc 
CFLAGS=		-v -s -O9 -fforce-addr -fforce-mem -ffast-math	\
		-fomit-frame-pointer -funroll-loops		\
		-DLOOPS=300000 -DTIMES -DHZ=100
DDLFLAGS = -I./ddllib -p powerpc-linux-

TARGET_MATCHED = true
else
CC:=		gcc
CFLAGS =  
endif

###########################################################################
ifneq "$(TARGET_MATCHED)" "true"
.DEFAULT: all ; @$(MAKE) all
all:	
	@echo "You didn't select a supported TARGET"
	@echo "select one of: ATCANMINI_PELICAN, CPC_PCI, IME_SLIMLINE, IXXAT_PCI03, PCM3680, PC104_200, TRM816"
else
###########################################################################
# select the compiler toolchain
###########################################################################
TOOLS=powerpc-linux-gcc 
TOOLS=arm-uclinux-
TOOLS=/usr/local/armtools_glibc/bin/arm-uclinux-
TOOLS=

BOLD		= "\033[1m"
BOLD		= "\033[0;31m"
NBOLD		= "\033[0m"

ECHO		= /bin/echo -e

COMPILE	= $(ECHO) "--- Compiling "$(BOLD)$<$(NBOLD)" for $(TARGET) on $(LINUXTARGET) ..." ; \
	  $(TOOLS)gcc
DEPEND	= $(ECHO) "--- Checking dependencies..." ; $(TOOLS)$(CPP)
RLINK	= $(ECHO) "--- Linking (relocatable) "$(BOLD)$@$(NBOLD)"..." ;\
	  $(TOOLS)ld -r
LINK	= $(ECHO) "--- Linking "$(BOLD)$@$(NBOLD)"..." ; $(TOOLS)gcc
YACC	= $(ECHO) --- Running bison on $(BOLD)$<$(NBOLD)...; bison -d -y
LEX	= $(ECHO) --- Running flex on $(BOLD)$<$(NBOLD)...; flex 

CC	= $(COMPILE)

all: $(CAN_MODULE) #examples 


# !! should be for all Kernels > 2.2.17 ???
# for each kernel ther is a set of kernel specific headers in 
# /lib/modules/`uname -r`/build/include
#
ifeq "$(LINUXTARGET)" "LINUXOS"
ifeq "$(findstring 2.4., $(KVERSION))" ""
 INCLUDES = -Isrc
 TEST = Nein
else
 INCLUDES = -Isrc -I/lib/modules/`uname -r`/build/include
 #INCLUDES = -Isrc -I/home/geg/kernel/linux-2.4.22-586/include
 TEST = Ja
endif
endif

ifeq "$(LINUXTARGET)" "LINUXOS"
# That are the finally used flags for compiling the sources
CFLAGS = -Wall -D__KERNEL__ -DLINUX -O2 -Wstrict-prototypes -fomit-frame-pointer $(DEFS) $(OPTIONS) $(INCLUDES) -DVERSION=\"$(DVERSION)_$(TARGET)\"

else
#define for RTlLinux
MYCFLAGS = -O2	 -Wall
include rtl.mk	

endif

VPATH=src
# all the files to be compiled into object code
OBJS	=	\
	    can_core.o		\
	    can_open.o		\
	    can_read.o		\
	    can_write.o		\
	    can_ioctl.o		\
	    can_select.o	\
	    can_close.o		\
	    Can_debug.o		\
	    Can_error.o		\
	    can_util.o		\
	    can_sysctl.o	\

# include Chip specific object files
ifeq "$(TARGET)" "CPC_PCI"
OBJS += can_sja1000funcs.o
endif
ifeq "$(TARGET)" "ATCANMINI_PELICAN"
OBJS += can_sja1000funcs.o
endif
ifeq "$(TARGET)" "IXXAT_PCI03"
OBJS += can_sja1000funcs.o
endif
ifeq "$(TARGET)" "PCM3680"
OBJS += can_sja1000funcs.o
endif
ifeq "$(TARGET)" "TRM816"
OBJS += can_sja1000funcs.o
endif
ifeq "$(TARGET)" "PC104_200"
OBJS += can_mcf5282funcs.o
endif
ifeq "$(TARGET)" "IME_SLIMLINE"
OBJS += can_sja1000funcs.o
endif


$(CAN_MODULE):  $(addprefix $(OBJDIR)/,$(OBJS)) $(OBJDIR)
	@$(RLINK) -o $@ $(addprefix $(OBJDIR)/,$(OBJS))

$(OBJDIR)/can_core.o: can_core.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_open.o: can_open.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_read.o: can_read.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_write.o: can_write.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_ioctl.o: can_ioctl.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_select.o: can_select.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS)  $(INCLUDES) -o $@ $<
$(OBJDIR)/can_close.o: can_close.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_sja1000funcs.o: can_sja1000funcs.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_util.o: can_util.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_sysctl.o: can_sysctl.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/Can_error.o: Can_error.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/Can_debug.o: Can_debug.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<


# load host specific CAN configuration
load:
	$(ECHO) ">>> " Loading Driver Module to Kernel
	/sbin/insmod $(CAN_MODULE) 
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 0 >/proc/sys/Can/dbgMask

# load host configuration and set dbgMask
dload:
	$(ECHO) ">>> " Loading Driver Module to Kernel
	/sbin/insmod $(CAN_MODULE) 
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 7 >/proc/sys/Can/dbgMask
	echo "125 125 125 125" >/proc/sys/Can/Baud


# unload the CAN driver module
unload:
	$(ECHO) ">>> " Removing Driver Module from Kernel
	-/sbin/rmmod $(CAN_MODULE:.o=)


.PHONY:examples
examples:
	(cd examples;make)

clean:
	-rm -f tags
	-rm -f obj/*.o
	-rm -f Can.o
	(cd examples;make clean)

distclean: clean
	cd examples; make clean
	cd trm816; make clean


inodes:
	-mknod /dev/can0 c $(CAN_MAJOR) 0
	-mknod /dev/can1 c $(CAN_MAJOR) 1
	-mknod /dev/can2 c $(CAN_MAJOR) 2
	-mknod /dev/can3 c $(CAN_MAJOR) 3
	-mknod /dev/can4 c $(CAN_MAJOR) 4
	-mknod /dev/can5 c $(CAN_MAJOR) 5
	-mknod /dev/can6 c $(CAN_MAJOR) 6
	-mknod /dev/can7 c $(CAN_MAJOR) 7
	chmod 666 /dev/can[0-7]



ctags:
	$(CTAGS)  src/*.[ch] /usr/include/linux/pci.h

############################################################################
#              V e r s i o n  C o n t r o l
#
#
############################################################################
# commit changes of all files to the cvs repository
commit:
	cvs commit -F commitfile

# tag all files in the current module
tag:
	cvs tag $(RELEASE)

#### HTML Manual section. #################################
man:    port_footer.html
	doxygen

showman:
	netscape -raise -remote 'openURL(file:$(PROJECTHOME)/man/html/index.html)'

# Standardfooter f�r manual pages sollte irgendwo im pms 00340
# stehen, dito das port.gif bild
# 
port_footer.html:       Makefile
	cat ft.html | sed 's/TITLE/$(TITLE)/' \
		    | sed 's/DATE/$(shell date)/' \
		    > $@


archive:	distclean
	tar  zcvf can4linux.$(VERSION).$(REL).tgz -h\
		Makefile Doxyfile README* \
	        INSTALL.t2 INSTALL.pdf INSTALL-g.pdf CHANGELOG \
		etc \
		man \
		src \
		obj \
		examples \
		utils \
		ft.html \
		debug \
		trm816


install:
	-mkdir $(INSTALLHOME)/can4linux
	-mkdir $(INSTALLHOME)/can4linux/obj
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/etc .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/examples .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/src .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/utils .)
	cp Makefile $(INSTALLHOME)/can4linux
	(cd $(INSTALLHOME)/can4linux; make)

endif
# Help Text
## 
## make targets:
##  Can.o      - compile the driver sources to Can.o
##  examples   - compile examples in examples directory
##  man        - manual pages using Doxygen
##  archive    - create a *tgz
## 
## only as super user:
##  inodes     - create device entries  in /dev
##  load       - load the driver and use actual "host"-configuration
##  dload      - load with debugMask set to "debug"
##  unload     - unload the CAN driver module
## 
##  install    - install the driver in /usr/src with references to
##               this actual source tree


.PHONY:help
help:
	@echo -e "\nMakefile for the can4linux CAN driver module"
	@echo "Actual Release Tag is set to RELEASE=$(RELEASE)."
	@sed -n '/^##/s/^## //p' Makefile

