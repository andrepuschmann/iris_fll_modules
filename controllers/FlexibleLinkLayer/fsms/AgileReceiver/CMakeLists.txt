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

# Automatically and includes, lib, lib dirs and definition
include(${STATEBUILDERCPP_USE_FILE})

# Set the state machine(s)
set(FSM_FILES AgileReceiver.fsmcpp)

statebuildercpp_compile(FSM_CPP ${FSM_FILES})

INCLUDE_DIRECTORIES(${LOGLITE_INCLUDE_DIR})

ADD_LIBRARY(fll_fsm_agilereceiver STATIC AgileReceiver.cpp AgileReceiverFsm.h ${FSM_CPP} ${FSM_FILES})
IRIS_SET_PIC(fll_fsm_agilereceiver)
