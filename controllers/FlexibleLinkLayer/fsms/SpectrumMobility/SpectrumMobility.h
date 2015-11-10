/**
 * \file controllers/FlexibleLinkLayer/fsms/SpectrumMobility/SpectrumMobility.h
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

#ifndef SPECTRUMMOBILITY_H
#define SPECTRUMMOBILITY_H

#include <cstdlib>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/any.hpp>
#include "FllController.h"
#include "FllFsmBase.h"
#include "FllChannel.h"

class SpectrumMobilityContext;

class SpectrumMobility : public iris::FllFsmBase
{
public:
    SpectrumMobility(boost::asio::io_service &io_service, iris::FllController *handle);
    virtual ~SpectrumMobility();

    void init() {}
    void deinit() {}
    void processEvent(iris::Event &e);

    // State methods
    void print(const std::string& message);
    bool hasBackupChannel(void);
    int getInvalidationTimeout(void); // getter
    int getInactivityTimeout(void);
    int getSensingTimeout(void);
    bool isFollower(void);
    void getNextChannel(void);
    void setSensingChannel(FllChannel channel);
    void setSensingChannelState(FllChannelState state);

    void setBackupChannel(const FllChannel channel)
    {
        backupChannel_ = channel;
    }
    FllChannel getBackupChannel(void)
    {
        return backupChannel_;
    }

    void incrementCounter(void)
    {
        numHandovers_++;
    }

    int getSensingTrx(void)
    {
        return sensingTrx_;
    }

  private:
    virtual std::string getName() const { return "SpectrumMobility"; }

    std::unique_ptr<SpectrumMobilityContext> context_; //Generated class
    SpectrumMobilityContext& GetContext();
    FllChannel sensingChannel_;
    FllChannel backupChannel_;
    int sensingTrx_;
    int numHandovers_;
    int puIactivityTimeout_;
    int puSensingTimeout_;
    int bcInvalidationTimeout_;
    bool isFollower_;
    std::string sensingModule_;
};

#endif // SPECTRUMMOBILITY_H
