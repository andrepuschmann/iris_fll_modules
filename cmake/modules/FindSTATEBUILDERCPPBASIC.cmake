#
# Copyright 2012-2013 The Iris Project Developers. See the
# COPYRIGHT file at the top-level directory of this distribution
# and at http://www.softwareradiosystems.com/iris/copyright.html.
#
# This file is part of the Iris Project.
#
# Iris is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# Iris is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# A copy of the GNU Lesser General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

# Try to find StateBuilderCpp
# Note that this is only an initial test in order to decide whether to look
# for the _real_ CMake script provided by StateBuilderCpp itself
# Once done this will define
#  STATEBUILDERCPPBASIC_FOUND, If false, don't try to use the state machine builder

FIND_PROGRAM(STATEBUILDERCPP_EXECUTABLE
  NAMES 
    StateBuilderCpp.exe
    StateBuilderCpp.sh
  PATHS 
    $ENV{STATEBUILDERCPP_HOME}/bin
    $ENV{HOME}/StateBuilderCpp/bin
    $ENV{HOME}/AppData/Local/StateForge/StateBuilderCpp/bin
  NO_DEFAULT_PATH
)

IF(STATEBUILDERCPP_EXECUTABLE)
  SET(STATEBUILDERCPPBASIC_FOUND "YES")
  MESSAGE(STATUS "    StateBuilderCpp is installed.") 
ELSE() 
  SET(STATEBUILDERCPPBASIC_FOUND "NO")
  MESSAGE(STATUS "    Couldn't find StateBuilderCpp.") 
ENDIF()
