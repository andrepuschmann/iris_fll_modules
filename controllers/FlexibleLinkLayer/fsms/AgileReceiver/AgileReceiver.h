/**
 * \file controllers/FlexibleLinkLayer/fsms/AgileReceiver/AgileReceiver.h
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

#ifndef AGILERECEIVER_H
#define AGILERECEIVER_H

#include <cstdlib>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "FllController.h"
#include "FllFsmBase.h"
#include "FllChannel.h"

class AgileReceiverContext;

class AgileReceiver : public iris::FllFsmBase
{
public:
    AgileReceiver(boost::asio::io_service &io_service, iris::FllController *handle);
    virtual ~AgileReceiver();

    void init() {}
    void deinit() {}
    void processEvent(iris::Event &e);

    void Print(const std::string& message);
    int getFrameTimeout();
    void setFrameTimeout(const int timeout);
    bool getHasReceivedFrame();
    void setHasReceivedFrame(const bool value);

    // Events
    void EvStart();
    void EvStop();
    void EvActionFinished();
    void EvReconfDone();
    void EvFrameReceived();

    // State methods
    void FindNextChannel();

  private:
    virtual std::string getName() const { return "AgileReceiver"; }

    AgileReceiverContext *context_; //Generated class
    AgileReceiverContext& GetContext();
    int finishedCounter_;
    int timeoutCounter_;
    int frameTimeout_;
    bool hasReceivedFrame_;
};

#endif // AGILERECEIVER_H
