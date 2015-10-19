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

# - Try to find libgdtp - https://github.com/andrepuschmann/libgdtp
# Once done this will define
#  GDTP_FOUND - System has gdtp
#  GDTP_INCLUDE_DIRS - The libgdtp include directories
#  GDTP_LIBRARIES - The libraries needed to use libgdtp

find_path(GDTP_INCLUDE_DIR 
            NAMES libgdtp.h
            HINTS $ENV{GDTP_DIR}/include/libgdtp ${LIBGDTP_SOURCE_DIR}/include
            PATHS /usr/local/include/libgdtp
                  /usr/include/libgdtp )

find_library(GDTP_LIBRARY 
            NAMES gdtp
            HINTS $ENV{GDTP_DIR}/lib ${LIBGDTP_BINARY_DIR}/src
            PATHS /usr/local/lib
                  /usr/lib)

set(GDTP_LIBRARIES ${GDTP_LIBRARY} )
set(GDTP_INCLUDE_DIRS ${GDTP_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GDTP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(gdtp  DEFAULT_MSG
                                  GDTP_LIBRARY GDTP_INCLUDE_DIR)

mark_as_advanced(GDTP_INCLUDE_DIR GDTP_LIBRARY )
