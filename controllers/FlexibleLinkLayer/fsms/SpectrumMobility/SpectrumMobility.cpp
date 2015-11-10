/**
 * \file controllers/FlexibleLinkLayer/fsms/SpectrumMobility/SpectrumMobility.cpp
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
 * Source of the SpectrumMobility FSM.
 *
 */

#include <iostream>
#include <FllFsmObserver.h>
#include "SpectrumMobility.h"
#include "SpectrumMobilityFsm.h"

using namespace std;

SpectrumMobility::SpectrumMobility(boost::asio::io_service &io_service, iris::FllController *handle) : FllFsmBase(handle),
  puIactivityTimeout_(1000),
  puSensingTimeout_(500),
  bcInvalidationTimeout_(500),
  sensingTrx_(0),
  numHandovers_(0)
{
    context_.reset(new SpectrumMobilityContext(*this, io_service));
    context_->SetObserver(FllFsmObserver::GetInstance());

    // try to get user-defined timeout paramters
    try {
        int temp = boost::lexical_cast<int>(getParameterValue("puinactivitytimeout"));
        if (temp > 0)
            puIactivityTimeout_ = temp;

        temp = boost::lexical_cast<int>(getParameterValue("pusensingtimeout"));
        if (temp > 0)
            puSensingTimeout_ = temp;

        temp = boost::lexical_cast<int>(getParameterValue("bcinvalidationtimeout"));
        if (temp > 0)
            bcInvalidationTimeout_ = temp;
    } catch (boost::bad_lexical_cast &) {
        // no valid timeout parameter given
    }

    // initialize BC
    FllChannel channel;
    try {
        int temp = boost::lexical_cast<uint64_t>(getParameterValue("freq1"));
        if (temp > 0)
            channel.setFreq(temp);

        temp = boost::lexical_cast<uint64_t>(getParameterValue("rate"));
        if (temp > 0)
            channel.setBandwidth(temp);
    } catch (boost::bad_lexical_cast &) {
        // no valid timeout parameter given
    }

    // get nodes' role
    try {
        isFollower_ = (getParameterValue("role") == "follower");
    } catch (boost::bad_lexical_cast &) {
        throw FllException("Nodes' role not specified.");
    }

    if (not isFollower()) {
        // initialize sensing transceiver
        try {
            sensingTrx_ = boost::lexical_cast<int>(getParameterValue("sensingtrx"));
            sensingModule_ = getParameterValue("componentsensingmodule" + std::to_string(sensingTrx_));
        } catch (boost::bad_lexical_cast &) {
            throw FllException("Sensing transceiver not specified.");
        }
    }

    setSensingChannel(channel);
    context_->EnterInitialState();
}


SpectrumMobility::~SpectrumMobility()
{
    LOG(LINFO) << "Total number of handovers: " << numHandovers_;
}


SpectrumMobilityContext& SpectrumMobility::GetContext()
{
    if (context_ != NULL) {
        return *context_.get();
    } else {
        throw std::runtime_error("context is not set");
    }
}


// Event handlers for this FSM
void SpectrumMobility::processEvent(iris::Event &e)
{
    if (e.eventName == "EvStop") {
        GetContext().EvStop();
    } else
    if (e.eventName.find("EvReconfDone") != string::npos) {
        try {
            FllChannel channel = boost::any_cast<FllChannel>(e.data[0]);
            switch(e.eventName.back()) {
            case '0':
                GetContext().EvReconfDone0(channel);
                break;
            case '1':
                GetContext().EvReconfDone1(channel);
                break;
            default:
                throw FllException("Invalid EvReconfDone received.");
            }
        } catch ( const std::exception& e ) {
            LOG(LERROR) << e.what();
        }
    } else
    if (e.eventName.find("evpuactive") != string::npos) {
        try {
            switch(e.eventName.back()) {
            case '0':
                GetContext().EvPuActiveOperatingChannel();
                break;
            case '1':
                GetContext().EvPuActiveSensingChannel();
                break;
            default:
                throw FllException("Invalid EvPuActive event received.");
            }
        } catch ( const std::exception& e ) {
            LOG(LERROR) << e.what();
        }
    } else
    if (e.eventName == "evupdatebackupchannel") {
        try {
            FllChannel channel = boost::any_cast<FllChannel>(e.data[0]);
            setBackupChannel(channel);
            GetContext().EvUpdateBackupChannel(channel);
        } catch ( const std::exception& e ) {
            LOG(LERROR) << e.what();
        }
    } else
    {
        LOG(LERROR) << "No action registered for event " << e.eventName;
    }
}


void SpectrumMobility::print(const std::string& message)
{
    LOG(LINFO) << message;
}


int SpectrumMobility::getInvalidationTimeout(void)
{
    return bcInvalidationTimeout_;
}


int SpectrumMobility::getInactivityTimeout(void)
{
    return puIactivityTimeout_;
}


int SpectrumMobility::getSensingTimeout(void)
{
    return puSensingTimeout_;
}


bool SpectrumMobility::isFollower(void)
{
    return isFollower_;
}


void SpectrumMobility::getNextChannel(void)
{
    // skip operating channel for sensing
    FllChannel trx0Channel = this->getOperatingChannel(0);
    FllChannel trx1channel = this->getNextChannelInList();
    while (trx0Channel == trx1channel) {
        trx1channel = this->getNextChannelInList();
    }
    GetContext().EvChannelFound(trx1channel);
}


void SpectrumMobility::setSensingChannel(FllChannel channel)
{
    sensingChannel_ = channel;
}


void SpectrumMobility::setSensingChannelState(FllChannelState state)
{
    sensingChannel_.setState(state);
    this->setChannelState(sensingChannel_);
}
