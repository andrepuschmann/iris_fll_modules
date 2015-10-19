/**
 * \file components/gpp/stack/GdtpComponent/test/GdtpComponent_test.cpp
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Main test file for the Gdtp component.
 */

#define BOOST_TEST_MODULE GdtpComponent_test

#include <boost/test/unit_test.hpp>
#include "../GdtpComponent.h"

using namespace std;
using namespace iris;
using namespace iris::stack;

BOOST_AUTO_TEST_SUITE (GdtpComponent_test)

BOOST_AUTO_TEST_CASE(GdtpComponent_Basic_Test)
{
    BOOST_REQUIRE_NO_THROW(GdtpComponent("test"));
}

BOOST_AUTO_TEST_CASE(GdtpComponent_Parm_Test)
{
    GdtpComponent gdtp("test");
    BOOST_CHECK(gdtp.getParameterDefaultValue("scheduler") == "fifo");
    BOOST_CHECK(gdtp.getParameterDefaultValue("localaddress") == "f009e090e90e");
    BOOST_CHECK(gdtp.getParameterDefaultValue("destinationaddress") == "00f0f0f0f0f0");
#ifdef __unix__
    BOOST_CHECK(gdtp.getParameterDefaultValue("isethdevice") == "false");
    BOOST_CHECK(gdtp.getParameterDefaultValue("ethdevicename") == "tap0");
#endif
    BOOST_CHECK(gdtp.getParameterDefaultValue("acktimeout") == "100");
    BOOST_CHECK(gdtp.getParameterDefaultValue("maxretry") == "100");
}

BOOST_AUTO_TEST_SUITE_END()
