/**
 * \file controllers/FlexibleLinkLayer/FllFsmBase.cpp
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

#include "FllFsmBase.h"

namespace iris
{

// Wrapper functions to access the FllController
std::string FllFsmBase::getParameterValue(const std::string name)
{
    if (d_controllerHandle) {
        return d_controllerHandle->getDynamicParameterValue(name);
    }
}


void FllFsmBase::reconfigureChannel(const FllChannel channel, const int trx)
{
    if (d_controllerHandle) {
        d_controllerHandle->reconfigureChannel(channel, trx);
    }
}


void FllFsmBase::setGdtpOperationMode(const bool value)
{
    if (d_controllerHandle) {
        d_controllerHandle->setGdtpOperationMode(value);
    }
}


void FllFsmBase::setSensingMode(const int trx, const bool mode)
{
    if (d_controllerHandle) {
        d_controllerHandle->setSensingMode(trx, mode);
    }
}


void FllFsmBase::setChannelState(FllChannel channel)
{
    if (d_controllerHandle) {
        d_controllerHandle->updateChannelState(channel);
    }
}


template<typename T>
void FllFsmBase::activateEvent(std::string name, T &data)
{
    if (d_controllerHandle) {
        d_controllerHandle->activateEvent(name, data);
    }
}


template<typename T>
void FllFsmBase::activateEvent(std::string name, std::vector<T> &data)
{
    if (d_controllerHandle) {
        d_controllerHandle->activateEvent(name, data);
    }
}


void FllFsmBase::postCommand(Command command)
{
    if (d_controllerHandle) {
        d_controllerHandle->postCommand(command);
    }
}

void FllFsmBase::postEvent(Event &e)
{
    if (d_controllerHandle) {
        d_controllerHandle->postEvent(e);
    }
}

bool FllFsmBase::inDryRunMode()
{
    if (d_controllerHandle) {
        return d_controllerHandle->inDryRunMode();
    }
}


// Helper functions to access the ChannelManager
FllChannelVector FllFsmBase::getAvailableChannels(void) {
    if (d_controllerHandle) {
        return d_controllerHandle->getChannelManagerHandle()->getAvailableChannels();
    }
}


FllChannel FllFsmBase::getOperatingChannel(const int trx)
{
    if (d_controllerHandle) {
        return d_controllerHandle->getChannelManagerHandle()->getOperatingChannel(trx);
    }
}


FllChannel FllFsmBase::getRandomChannel(void)
{
    if (d_controllerHandle) {
        return d_controllerHandle->getChannelManagerHandle()->getRandomChannel();
    }
}


FllChannel FllFsmBase::getNextChannelInList(void)
{
    if (d_controllerHandle) {
        return d_controllerHandle->getChannelManagerHandle()->getNextChannelInList();
    }
}


void FllFsmBase::printOperatingChannel(void)
{
    if (d_controllerHandle) {
        d_controllerHandle->getChannelManagerHandle()->printChannelConfiguration();
    }
}


} // namespace iris
