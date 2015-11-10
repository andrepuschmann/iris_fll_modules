#
# Copyright 2012-2014 The Iris Project Developers. See the
# COPYRIGHT file at the top-level directory of this distribution
# and at http://www.softwareradiosystems.com/iris/copyright.html.
#
# This file is part of the Iris Project.
#
# Iris is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# Iris is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# A copy of the GNU General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

MESSAGE(STATUS "  Processing fll.")

########################################################################
# Add includes and dependencies
########################################################################
find_package(STATEBUILDERCPPBASIC REQUIRED) # Look for the Finite State Machine Compiler
FIND_PACKAGE( OSPECORR QUIET )

IF (STATEBUILDERCPPBASIC_FOUND)
  LIST(APPEND CMAKE_MODULE_PATH $ENV{STATEBUILDERCPP_HOME}/share/cmake/)
  SET(STATEBUILDERCPP_USE_BOOST TRUE)
  find_package(STATEBUILDERCPP QUIET)
  # Automatically and includes, lib, lib dirs and definition
  include(${STATEBUILDERCPP_USE_FILE})

  INcLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
  ADD_SUBDIRECTORY(fsms)

  SET(sources
      FllController.cpp
      FllChannelManager.cpp
      FllFsmObserver.cpp
      FllFsmBase.cpp
      FllOspecorrHelper.h
  )

  ADD_LIBRARY(controller_fll SHARED ${sources} ${fsm_sources})
  TARGET_LINK_LIBRARIES(controller_fll fll_fsm_agilereceiver)
  TARGET_LINK_LIBRARIES(controller_fll fll_fsm_spectrummobility)
  SET_TARGET_PROPERTIES(controller_fll PROPERTIES OUTPUT_NAME "fll")
  SET_TARGET_PROPERTIES(controller_fll PROPERTIES DEFINE_SYMBOL "IRIS_CORE_DLL_EXPORTS")
  IF (OSPECORR_FOUND)
    ADD_DEFINITIONS(-DOSPECORR)
    TARGET_LINK_LIBRARIES (controller_fll ${OSPECORR_LIBS})
  ENDIF (OSPECORR_FOUND)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  IRIS_INSTALL(controller_fll)
  IRIS_APPEND_INSTALL_LIST("fll")

ELSE (STATEBUILDERCPPBASIC_FOUND)
  IRIS_APPEND_NOINSTALL_LIST(controller_fll)

ENDIF (STATEBUILDERCPPBASIC_FOUND)
