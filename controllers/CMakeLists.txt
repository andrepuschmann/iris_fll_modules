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

MESSAGE(STATUS "*****************************")
MESSAGE(STATUS "Processing Controllers.")
MESSAGE(STATUS "*****************************")

########################################################################
# Create files to cache names of installed/not installed modules
########################################################################
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/iris_inst)
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/iris_noinst)

########################################################################
# Helpful Macros
########################################################################
MACRO(IRIS_INSTALL)
    INSTALL(TARGETS ${ARGV} DESTINATION ${CONTROLLERS_DIR})
ENDMACRO(IRIS_INSTALL)

MACRO(IRIS_APPEND_INSTALL_LIST)
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/../iris_inst "\n--     ${ARGV}")
ENDMACRO(IRIS_APPEND_INSTALL_LIST)

MACRO(IRIS_APPEND_NOINSTALL_LIST)
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/../iris_noinst "\n--     ${ARGV}")
ENDMACRO(IRIS_APPEND_NOINSTALL_LIST)

########################################################################
# Adding Iris module path
########################################################################
INCLUDE_DIRECTORIES(${IRIS_MODULES_SOURCE_DIR}/lib/generic)

########################################################################
# Add the subdirectories
########################################################################
# Recurse into subdirectories. This does not actually cause another cmake 
# executable to run. The same process will walk through the project's 
# entire directory structure.
ADD_SUBDIRECTORY(FlexibleLinkLayer)
ADD_SUBDIRECTORY(GenericDisplay)
ADD_SUBDIRECTORY(MacPhy)
ADD_SUBDIRECTORY(RadioConfig)

########################################################################
# List controllers to be installed/not installed
########################################################################
FILE(READ ${CMAKE_CURRENT_BINARY_DIR}/iris_inst IRIS_INST_COMPS)
FILE(READ ${CMAKE_CURRENT_BINARY_DIR}/iris_noinst IRIS_NOINST_COMPS)
MESSAGE(STATUS)
MESSAGE(STATUS "  Building Controllers: ${IRIS_INST_COMPS}")
MESSAGE(STATUS)
MESSAGE(STATUS "  Not building Controllers: ${IRIS_NOINST_COMPS}")
MESSAGE(STATUS)
