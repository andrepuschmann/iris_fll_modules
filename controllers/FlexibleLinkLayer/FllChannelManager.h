/**
 * \file controllers/FlexibleLinkLayer/FllChannelManager.h
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
 * Header file of the channel manager component of FLL.
 */

#ifndef FLLCHANNELMANAGER_H
#define FLLCHANNELMANAGER_H

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/format.hpp>
#include <vector>
#include <string>
#include <irisapi/Logging.h>
#include "FllChannel.h"
#include "FllController.h"
#include "FllException.h"
#if OSPECORR
#include "FllOspecorrHelper.h"
#include "FllDrmHelper.h"
#endif

namespace iris
{

class FllChannelManager
{
public:
    FllChannelManager(iris::FllController *handle);
    void initialize(void);

    // getter
    const FllChannelVector getAvailableChannels();
    const FllChannel getOperatingChannel(const int trx);
    const FllChannel getRandomChannel();
    const FllChannel getNextChannelInList();

    // setter
    void setMode(const std::string mode);
    void setOperatingChannel(const FllChannel ch, const int trx);
    void updateChannelState(const FllChannel ch);

    // misc
    void printChannelConfiguration(void);

private:
    enum ChannelMode { SIMPLE, DRM, RANDOM, UNKNOWN };
    void extractChannelsFromParameter();

    std::string getName() const;

    // members
    FllController *controllerHandle_;
    ChannelMode mode_;
    FllChannelVector channels_;
    FllChannelVectorIt channelsIt_;
    uint32_t numTrx_;
    std::map<int, FllChannel> operatingChannelMap_; /** could be a vector for multiple transceiver setups */
    boost::mutex mutex_;
};

} // namespace iris

#endif // FLLCHANNELMANAGER_H
