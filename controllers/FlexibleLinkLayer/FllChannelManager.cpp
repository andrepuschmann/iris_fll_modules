/**
 * \file controllers/FlexibleLinkLayer/FllChannelManager.cpp
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
 * Implementation of the channel manager component of FLL.
 */

#include "FllChannelManager.h"

namespace iris
{

FllChannelManager::FllChannelManager(iris::FllController *handle = NULL) :
    controllerHandle_(handle),
    numTrx_(0)
{
    #if OSPECORR
    FllOspecorrHelper::initialize();
    #endif
}


void FllChannelManager::initialize(void)
{
    // initialize remaining variables
    #if OSPECORR
    for (FllChannel& e : channels_) {
        FllOspecorrHelper::setChannelState(e);
    }
    #endif

    if (not channels_.empty()) {
        for (int i = 0; i < numTrx_; i++) {
            setOperatingChannel(*channels_.begin(), i);
        }
        channelsIt_ = channels_.begin();
    }
}


void FllChannelManager::setMode(const std::string mode)
{
    boost::mutex::scoped_lock lock(mutex_);
    if (mode == "simple") {
        extractChannelsFromParameter();
        mode_ = ChannelMode::SIMPLE;
    } else
#if OSPECORR
    if (mode == "drm") {
        channels_ = FllDrmHelper::getChannelVector();
        mode_ = ChannelMode::DRM;
    } else
#endif
    if (mode == "random") {
        //FIXME: create random channels here
        mode_ = ChannelMode::RANDOM;
    } else {
        LOG(LERROR) << mode << " is not a proper mode, aborting!";
    }

    // get number of transceivers
    try {
        numTrx_ = boost::lexical_cast<uint32_t>(controllerHandle_->getDynamicParameterValue("numtrx"));
    } catch (FllException e) {
        LOG(LERROR) << "Number of transceivers not given.";
    }
}


// fill channel map with available channels
void FllChannelManager::extractChannelsFromParameter(void)
{
    // check for freq-start, stop and step first
    try {
        uint64_t freqstart = boost::lexical_cast<uint64_t>(controllerHandle_->getDynamicParameterValue("freqstart"));
        uint64_t freqstop = boost::lexical_cast<uint64_t>(controllerHandle_->getDynamicParameterValue("freqstop"));
        uint64_t freqstep = boost::lexical_cast<uint64_t>(controllerHandle_->getDynamicParameterValue("freqstep"));
        uint32_t rate = boost::lexical_cast<uint32_t>(controllerHandle_->getDynamicParameterValue("rate"));

        size_t nrOfChannels = (freqstop - freqstart) / freqstep;
        LOG(LINFO) << "Populate channel map with " << nrOfChannels << " channels.";
        for (size_t i = 0; i < nrOfChannels; i++) {
            uint64_t freq = freqstart + i * freqstep;
            FllChannel ch(freq, rate);
            channels_.push_back(ch);
        }

    } catch (FllException e) {
        LOG(LINFO) << "Extracting fixed channel configuration from dynamic parameters.";

        // try to extract a maximum of 10 channels from dynamic parameters
        for (size_t i = 0; i < 10; i++) {
            std::string name = boost::str(boost::format("freq%d") % i);
            //LOG(LDEBUG) << "name: " << name;
            //LOG(LDEBUG) << "rate: " << rate;
            try {
                uint64_t freq = boost::lexical_cast<uint64_t>(controllerHandle_->getDynamicParameterValue(name));
                uint32_t rate = boost::lexical_cast<uint32_t>(controllerHandle_->getDynamicParameterValue("rate"));
                //LOG(LDEBUG) << "freq: " << freq;
                FllChannel ch = { freq, rate };
                channels_.push_back(ch);
            } catch (FllException e) {
                break;
            }
        }
    }
}

void FllChannelManager::printChannelConfiguration(void)
{
    boost::mutex::scoped_lock lock(mutex_);
    LOG(LDEBUG) << "Current channel configuration:";
    LOG(LDEBUG) << "Available channels: " << channels_.size();
    for (FllChannelVectorIt it = channels_.begin(); it != channels_.end(); ++it) {
        size_t pos = it - channels_.begin() ;
        LOG(LDEBUG) << "  " << pos << ": " << it->pp_string();
    }

    for (int i = 0; i < numTrx_; i++) {
        LOG(LDEBUG) << "TRx: " << i << ": " << operatingChannelMap_[i].pp_string();
    }
}


const FllChannel FllChannelManager::getOperatingChannel(const int trx)
{
    boost::mutex::scoped_lock lock(mutex_);
    return operatingChannelMap_[trx];
}


const FllChannel FllChannelManager::getRandomChannel()
{
    boost::mutex::scoped_lock lock(mutex_);
    int index = rand() % channels_.size();
    return channels_[index];
}


const FllChannel FllChannelManager::getNextChannelInList()
{
    boost::mutex::scoped_lock lock(mutex_);
    std::advance(channelsIt_, 1);
    if (channelsIt_ == channels_.end()) {
        channelsIt_ = channels_.begin();
    }
    return *channelsIt_;
}


const FllChannelVector FllChannelManager::getAvailableChannels()
{
    boost::mutex::scoped_lock lock(mutex_);
    return channels_;
}


void FllChannelManager::setOperatingChannel(const FllChannel ch, const int trx)
{
    boost::mutex::scoped_lock lock(mutex_);
    operatingChannelMap_[trx] = ch;

    // only set operating channel for primary radio
    if (trx == 0) operatingChannelMap_[trx].setState(BUSY_SU);

    #if OSPECORR
    FllOspecorrHelper::setChannelState(operatingChannelMap_[trx], trx, true);
    #endif
}


void FllChannelManager::updateChannelState(FllChannel ch)
{
    boost::mutex::scoped_lock lock(mutex_);
    // find element in vector and update state
    for (int i = 0; i < channels_.size(); i++) {
        if (channels_[i] == ch) {
            LOG(LDEBUG) << "Set state: Freq: " << ch.getFreq() << " BW: " << ch.getBandwidth() << " " << ch.getStateString();
            channels_[i].setState(ch.getState());
            #if OSPECORR
            FllOspecorrHelper::setChannelState(ch);
            if (mode_ == DRM) FllDrmHelper::setChannelState(ch, i);
            #endif
            return;
        }
    }
    LOG(LERROR) << "Channel not found in FLL database.";
}



std::string FllChannelManager::getName() const
{
    return "FllChannelManager";
}


} // namespace iris
