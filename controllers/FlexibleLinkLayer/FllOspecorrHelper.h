/**
 * \file controllers/FlexibleLinkLayer/FllOspecorrHelper.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2014 The Iris Project Developers. See the
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
 * GNU General Public License for more details.
 *
 * A copy of the GNU General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Helper class to communicate between OSPECORR applications
 * and the FLL using the OSPECORR message format.
 */

#ifndef FLLOSPECORRHELPER_H
#define FLLOSPECORRHELPER_H

#include "sclhelper.hpp"
#include "phy.pb.h"

namespace iris
{

class FllOspecorrHelper
{
    public:
        static void initialize(void)
        {
            GateFactory &myFactory = GateFactory::getInstance();
            sclGate* gate = myFactory.createGate("PhyComponent", "event");
            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
        }

        static void setChannelState(FllChannel channel, const int trx = 0, bool isActive = false)
        {
            GateFactory &myFactory = GateFactory::getInstance();
            sclGate *gate = myFactory.createGate("PhyComponent", "event");
            scl_phy::PhyMessage stats;
            stats.set_state(fromFllChannelState(channel.getState()));
            stats.set_is_active(isActive);
            stats.set_trx(trx);
            scl_phy::BasicChannel* ch = stats.mutable_channel();
            ch->set_f_center(channel.getFreq());
            ch->set_bandwidth(channel.getBandwidth());
            gate->sendProto(stats);
        }

        static scl_phy::ChannelState fromFllChannelState(FllChannelState state)
        {
            switch (state) {
            case FREE: return scl_phy::FREE; break;
            case BUSY_SU: return scl_phy::BUSY_SU; break;
            case BUSY_PU: return scl_phy::BUSY_PU; break;
            default: return scl_phy::UNKNOWN;
            }
        }
};

} // namespace iris

#endif // FLLOSPECORRHELPER_H
