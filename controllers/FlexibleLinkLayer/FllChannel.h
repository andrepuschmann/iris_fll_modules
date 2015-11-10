/**
 * \file controllers/FlexibleLinkLayer/FllChannel.h
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
 * Header file defining "the channel" as seen by the FLL.
 */

#ifndef FLLCHANNEL_H
#define FLLCHANNEL_H

#include <string>
#include <vector>

class FllChannel;

typedef std::vector<FllChannel> FllChannelVector;
typedef std::vector<FllChannel>::iterator FllChannelVectorIt;

enum FllChannelState
{
    FREE = 0,
    BUSY_SU = 1,
    BUSY_PU = 2,
    UNKNOWN = 3
};

// FLL internal representation of a _channel_
class FllChannel
{
public:
    FllChannel(const uint64_t freq = 0, const uint32_t bandwidth = 0, FllChannelState state = UNKNOWN):
        freq_(freq),
        bandwidth_(bandwidth),
        state_(state)
    {}

    bool operator== (FllChannel &c1)
    {
        return (this->getFreq() == c1.getFreq() &&
                this->getBandwidth() == c1.getBandwidth());
    }

    bool operator!= (FllChannel c1)
    {
        return !(*this == c1);
    }

    void setFreq(const uint64_t freq) { freq_ = freq; }
    void setBandwidth(const uint32_t bandwidth) { bandwidth_ = bandwidth; }
    void setState(const FllChannelState state) { state_ = state; }
    uint64_t getFreq() const { return freq_; }
    uint32_t getBandwidth() const { return bandwidth_; }
    FllChannelState getState() const { return state_; }
    std::string getStateString() const
    {
        switch (state_) {
        case FREE: return "FREE"; break;
        case BUSY_SU: return "BUSY_SU"; break;
        case BUSY_PU: return "BUSY_PU"; break;
        default: return "UNKNOWN";
        }
    }
    bool isValid() const
    {
        if (freq_ > 0 && bandwidth_ > 0)
            return true;
        return false;
    }
    bool isFree() const
    {
        return (state_ == FREE);
    }

    std::string pp_string(void)
    {
        return std::string("f_center: " + std::to_string(getFreq()) +
                           " bw: " + std::to_string(getBandwidth()) +
                           " state: " + getStateString());
    }

private:
    uint64_t freq_;
    uint32_t bandwidth_;
    FllChannelState state_;
};

#endif // FLLCHANNEL_H
