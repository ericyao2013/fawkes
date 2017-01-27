#*****************************************************************************
#         Makefile Build System for Fawkes: robot-memory-eval Plugin
#                            -------------------
#   Created on Thu Jan 26 15:54:26 2017
 
#   Copyright (C) 2017 by Frederik Zwilling
#
#*****************************************************************************
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#*****************************************************************************

BASEDIR = ../../../..
include $(BASEDIR)/etc/buildsys/config.mk
include $(BASEDIR)/src/plugins/mongodb/mongodb.mk

LIBS_robot_memory_eval = m fawkescore fawkesutils fawkesaspects fawkesbaseapp \
                      fawkesblackboard fawkesinterface fawkesrobotmemory fawkesmongodbaspect

OBJS_robot_memory_eval = robot-memory-eval_plugin.o robot-memory-eval_thread.o
OBJS_all = $(OBJS_robot-memory-eval)

ifeq ($(HAVE_CPP11)$(HAVE_MONGODB),11)
  CFLAGS  += $(CFLAGS_CPP11)

  PLUGINS_all = $(PLUGINDIR)/robot-memory-eval.$(SOEXT)
else
  ifneq ($(HAVE_CPP11),1)
    WARN_TARGETS += warning_cpp11
  endif
  ifneq ($(HAVE_MONGODB),1)
    WARN_TARGETS = warning_mongodb
  endif
endif

ifeq ($(OBJSSUBMAKE),1)
all: $(WARN_TARGETS)

.PHONY: warning_mongodb warning_cpp11
warning_mongodb:
	$(SILENT)echo -e "$(INDENT_PRINT)--> $(TRED)Omitting clips-robot-memory plugin$(TNORMAL) (mongodb not available)"
warning_cpp11:
	$(SILENT)echo -e "$(INDENT_PRINT)--> $(TRED)Cannot build clips-robot-memory plugin$(TNORMAL) (C++11 not supported)"

endif

include $(BUILDSYSDIR)/base.mk