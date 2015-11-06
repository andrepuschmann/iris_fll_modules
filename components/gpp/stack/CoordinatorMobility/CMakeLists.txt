#
# Copyright 2012 The Iris Project Developers. See the
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

MESSAGE(STATUS "  Processing coordinatormobility.")

########################################################################
# Add includes and dependencies
########################################################################

FIND_PACKAGE( Protobuf REQUIRED)
FIND_PACKAGE( OSPECORR QUIET )

########################################################################
# Build the library from source files
########################################################################
SET(sources
  CoordinatorMobilityComponent.cpp
  coordinatormobility.pb.cc
)

SET(headers
    coordinatormobility.proto
)

# Targets must be globally unique for cmake 
ADD_LIBRARY(comp_gpp_stack_coordinatormobility SHARED ${sources} ${headers})
TARGET_LINK_LIBRARIES(comp_gpp_stack_coordinatormobility ${PROTOBUF_LIBRARIES})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${IRIS_MODULES_SOURCE_DIR}/lib/generic/) # for utility/StackHelper.h
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/controllers/FlexibleLinkLayer/) # for FllStackComponent.h
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS coordinatormobility.proto)
IF (OSPECORR_FOUND)
  ADD_DEFINITIONS(-DOSPECORR)
  TARGET_LINK_LIBRARIES (comp_gpp_stack_coordinatormobility ${OSPECORR_LIBS})
ENDIF (OSPECORR_FOUND)
SET_TARGET_PROPERTIES(comp_gpp_stack_coordinatormobility PROPERTIES OUTPUT_NAME "coordinatormobility")
IRIS_INSTALL(comp_gpp_stack_coordinatormobility)
IRIS_APPEND_INSTALL_LIST("coordinatormobility")
