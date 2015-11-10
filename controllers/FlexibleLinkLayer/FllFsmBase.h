/**
 * \file controllers/FlexibleLinkLayer/FllFsmBase.h
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
 * Helper/base class for the FLL state machines.
 */

#ifndef FLLFSMBASE_H
#define FLLFSMBASE_H

#include <iostream>
#include <string>
#include "FllController.h"
#include "irisapi/Event.h"
#include "FllException.h"
#include "FllChannelManager.h"
#include "irisapi/Controller.h"

namespace iris
{

class FllFsmBase
{
public:
    FllFsmBase(FllController *handle) :
        d_controllerHandle(handle)
    {}
    virtual ~FllFsmBase() {};

    virtual void init(void) = 0;
    virtual void deinit(void) = 0;
    virtual void processEvent(iris::Event &e) = 0;

    // Helper functions to access the FllController
    std::string getParameterValue(const std::string name);
    template<typename T>
    void activateEvent(std::string name, T &data);
    template<typename T>
    void activateEvent(std::string name, std::vector<T> &data);
    void postCommand(Command command);
    void postEvent(Event &e);
    bool inDryRunMode();

    // Helper functions to access the ChannelManager
    FllChannelVector getAvailableChannels(void);
    FllChannel getOperatingChannel(const int trx);
    FllChannel getRandomChannel(void);
    FllChannel getNextChannelInList(void);
    //void setOperatingChannel(FllChannel channel, const int trx);
    void printOperatingChannel(void);
    void reconfigureChannel(const FllChannel channel, const int trx = 0);
    void setGdtpOperationMode(const bool value);
    void setSensingMode(const int trx, const bool mode);
    void setChannelState(FllChannel channel);

private:
    virtual std::string getName() const { return "FllFsmBase"; }
    FllController *d_controllerHandle; ///< for accessing Controller functions
};

} /* namespace iris */

#endif // FLLFSMBASE_H
