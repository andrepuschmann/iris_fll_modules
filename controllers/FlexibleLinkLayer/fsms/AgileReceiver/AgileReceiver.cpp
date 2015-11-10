/**
 * \file controllers/FlexibleLinkLayer/fsms/AgileReceiver/AgileReceiver.cpp
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
 * Source of the AgileReceiver.
 *
 */

#include <iostream>
#include <FllFsmObserver.h>
#include "AgileReceiver.h"
#include "AgileReceiverFsm.h"

using namespace std;

AgileReceiver::AgileReceiver(boost::asio::io_service &io_service, iris::FllController *handle) : FllFsmBase(handle),
  finishedCounter_(0),
  timeoutCounter_(0),
  frameTimeout_(1000),
  hasReceivedFrame_(false)
{
    context_ = new AgileReceiverContext(*this, io_service);
    context_->SetObserver(FllFsmObserver::GetInstance());

    // try to get user-defined timeout paramter
    try {
        int temp = boost::lexical_cast<int>(getParameterValue("frametimeout"));
        if (temp > 0)
            frameTimeout_ = temp;
    } catch (boost::bad_lexical_cast &) {
        // no valid timeout parameter given
    }
    context_->EnterInitialState();
    //context_->EvStart();
}


AgileReceiver::~AgileReceiver()
{
    delete context_;
}


AgileReceiverContext& AgileReceiver::GetContext()
{
    if (context_ != NULL) {
        return *context_;
    } else {
        throw std::runtime_error("context is not set");
    }
}


void AgileReceiver::processEvent(iris::Event &e)
{
    if (e.eventName == "EvStop") {
        this->EvStop();
    } else
    if (e.eventName == "EvReconfDone") {
        this->EvReconfDone();
    } else
    if (e.eventName == "evframedetected") {
        setHasReceivedFrame(true);
        this->EvFrameReceived();
    } else
    {
        LOG(LERROR) << "No action registered for event " << e.eventName;
    }
}


void AgileReceiver::Print(const std::string& message)
{
    LOG(LINFO) << message;
}


void AgileReceiver::setFrameTimeout(const int timeout)
{
    frameTimeout_ = timeout;
}


int AgileReceiver::getFrameTimeout()
{
    return frameTimeout_;
}


bool AgileReceiver::getHasReceivedFrame()
{
    return hasReceivedFrame_;
}

void AgileReceiver::setHasReceivedFrame(const bool value)
{
    //LOG(LDEBUG) << "Set hasReceivedFrame_ to " << value;
    hasReceivedFrame_ = value;
}

void AgileReceiver::EvStart()
{
    GetContext().EvStart();
}

void AgileReceiver::EvStop()
{
    GetContext().EvStop();
}


void AgileReceiver::EvReconfDone()
{
    GetContext().EvReconfDone();
}


void AgileReceiver::EvFrameReceived()
{
    GetContext().EvFrameReceived();
}


void AgileReceiver::FindNextChannel()
{
    LOG(LDEBUG) << "Pick next channel in available channels.";
    FllChannel operatingChannel = this->getOperatingChannel(0);
    FllChannelVector channels = this->getAvailableChannels();
    FllChannelVector::iterator channelIterator;

    //LOG(LINFO) << "Current freq: " << operatingChannel.getFreq();
    // find current channel and let iterator point to it
    for (channelIterator = channels.begin(); channelIterator != channels.end(); channelIterator++) {
        if (*channelIterator == operatingChannel) break;
    }

    // wrap around if needed
    if (boost::next(channelIterator) != channels.end()) {
        channelIterator++;
    } else {
        channelIterator = channels.begin();
    }

    //LOG(LINFO) << "New freq: " << (*channelIterator).getFreq();
    this->printOperatingChannel();
    GetContext().EvChannelFound(*channelIterator);
}
