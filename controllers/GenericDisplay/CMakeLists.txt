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

MESSAGE(STATUS "  Processing genericdisplay.")

########################################################################
# Add includes and dependencies
########################################################################

########################################################################
# Build the library from source files
########################################################################
SET(sources
    GenericDisplayController.cpp
)

# Targets must be globally unique for cmake
IF(IRIS_HAVE_REALPLOT AND IRIS_HAVE_SCATTERPLOT)
    ADD_LIBRARY(controller_genericdisplay SHARED ${sources})
    SET_TARGET_PROPERTIES(controller_genericdisplay PROPERTIES OUTPUT_NAME "genericdisplay")
    TARGET_LINK_LIBRARIES(controller_genericdisplay realplot scatterplot)
    IRIS_INSTALL(controller_genericdisplay)
    IRIS_APPEND_INSTALL_LIST("genericdisplay")
ELSE(IRIS_HAVE_REALPLOT AND IRIS_HAVE_SCATTERPLOT)
    IRIS_APPEND_NOINSTALL_LIST("genericdisplay")
ENDIF(IRIS_HAVE_REALPLOT AND IRIS_HAVE_SCATTERPLOT)
