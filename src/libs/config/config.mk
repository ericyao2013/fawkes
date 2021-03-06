#*****************************************************************************
#               Makefile Build System for Fawkes: Config Library
#                            -------------------
#   Created on Tue Jun 21 15:04:39 2007
#   Copyright (C) 2006-2012 by Tim Niemueller, AllemaniACs RoboCup Team
#
#*****************************************************************************
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#*****************************************************************************

ifneq ($(PKGCONFIG),)
  HAVE_SQLITE    = $(if $(shell $(PKGCONFIG) --exists 'sqlite3'; echo $${?/1/}),1,0)
  HAVE_YAMLCPP   = $(if $(shell $(PKGCONFIG) --exists 'yaml-cpp'; echo $${?/1/}),1,0)
endif
ifeq ($(HAVE_SQLITE),1)
  CFLAGS_SQLITE  = -DHAVE_SQLITE $(shell $(PKGCONFIG) --cflags 'sqlite3')
  LDFLAGS_SQLITE = $(shell $(PKGCONFIG) --libs 'sqlite3')
endif
ifeq ($(HAVE_YAMLCPP),1)
  CFLAGS_YAMLCPP  = -DHAVE_YAMLCPP $(shell $(PKGCONFIG) --cflags 'yaml-cpp')
  LDFLAGS_YAMLCPP = $(shell $(PKGCONFIG) --libs 'yaml-cpp')

  HAVE_YAMLCPP_0_3 = $(if $(shell $(PKGCONFIG) --atleast-version='0.3' 'yaml-cpp'; echo $${?/1/}),1,0)
  ifeq ($(HAVE_YAMLCPP_0_3),1)
    CFLAGS_YAMLCPP += -DHAVE_YAMLCPP_ATLEAST_0_3
  endif
  HAVE_YAMLCPP_0_5 = $(if $(shell $(PKGCONFIG) --atleast-version='0.5' 'yaml-cpp'; echo $${?/1/}),1,0)
  ifeq ($(HAVE_YAMLCPP_0_5),1)
    CFLAGS_YAMLCPP += -DHAVE_YAMLCPP_0_5
  endif
endif

