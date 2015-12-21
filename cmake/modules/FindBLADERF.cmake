#
# Copyright 2012-2013 The Iris Project Developers. See the
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

# - Try to find libbladerf - https://github.com/Nuand/bladeRF
# Once done this will define
#  BLADERF_FOUND - System has libbladerf
#  BLADERF_INCLUDE_DIRS - The libbladerf include directories
#  BLADERF_LIBRARIES - The libraries needed to use libbladerf

find_path(BLADERF_INCLUDE_DIR
            NAMES libbladeRF.h
            HINTS $ENV{BLADERF_DIR}/include
            PATHS /usr/local/include
                  /usr/include )

find_library(BLADERF_LIBRARY
            NAMES bladeRF
            HINTS $ENV{BLADERF_DIR}/lib
            PATHS /usr/local/lib
                  /usr/lib)

set(BLADERF_LIBRARIES ${BLADERF_LIBRARY} )
set(BLADERF_INCLUDE_DIRS ${BLADERF_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set BLADERF_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(bladerf DEFAULT_MSG
                                  BLADERF_LIBRARY BLADERF_INCLUDE_DIR)

mark_as_advanced(BLADERF_INCLUDE_DIR BLADERF_LIBRARY)
