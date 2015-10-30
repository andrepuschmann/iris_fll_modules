/**
 * \file controllers/MacPhy/MacPhyController.cpp
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
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
 * A controller that coordinates interaction among various
 * MAC and PHY layer components.
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "MacPhyController.h"

using namespace std;

namespace iris
{
    //Export library functions
    IRIS_CONTROLLER_EXPORTS(MacPhyController);

    MacPhyController::MacPhyController():
        Controller("macphycontroller", "A controller that coordinates interaction among MAC and PHY layer components", "Paolo Di Francesco, Andre Puschmann", "0.1")
    {
        registerParameter("engineSensingModule", "Name of the engine the sensing component lives in", "pnengine1",true, x_engineSensingModule);
        registerParameter("componentSensingModule", "Component name of the sensing module", "channelsense1",true, x_componentSensingModule);

        registerParameter("engineMacModule", "Name of the engine the Mac component lives in", "stackengine1",true, x_engineMacModule);
        registerParameter("componentMacModule", "Component name of the Mac module", "flexcsmamac1",true, x_componentMacModule);

        registerParameter("componentTxModule", "Component name of the module that issues burstAckEv event", "uhdtx1",true, x_componentTxModule);

        registerParameter("ccaThreshold", "Threshold for Clear Channel Assessment in dBm", "-70", true, ccaThreshold_x);
        
        registerParameter("debug", "Enable debug output", "false", false, x_debug);
    }


    void MacPhyController::subscribeToEvents()
    {
        // Format: subscribeToEvent(Event name, Component name);
        //subscribeToEvent("transmitPacketEv", x_componentMacModule);
        subscribeToEvent("burstack", x_componentTxModule);
        subscribeToEvent("ccarequest", x_componentMacModule);
        subscribeToEvent("rssiresult", x_componentSensingModule);
    }


    void MacPhyController::initialize()
    {
    }


    void MacPhyController::processEvent(Event &e)
    {
        if (x_debug) {
            LOG(LDEBUG) << "processEvent() called: " << e.eventName;
        }
        if (e.eventName == "burstackev") {
            Command packetInfo;
            packetInfo.typeId = e.typeId;
            packetInfo.trapWait = true;
            packetInfo.commandName = "burstack";
            packetInfo.componentName = x_componentMacModule;
            packetInfo.engineName = x_engineMacModule;
            postCommand(packetInfo);
        }
        if (e.eventName == "ccarequest") {
            processCcaRequestEvent(e);
        }
        if (e.eventName == "rssiresult") {
            processRssiResult(e);
        }
    }


    void MacPhyController::processCcaRequestEvent(Event &e)
    {
        assert(e.data.size() == 1);
        uint32_t ccaDuration = boost::any_cast<uint32_t>(e.data[0]);
        if (x_debug) LOG(LDEBUG) << "CCA duration: " << ccaDuration;
        
        // FIXME: simplify CCA functions
        triggerCca(x_componentSensingModule, x_engineSensingModule, ccaDuration);
    }


    void MacPhyController::processRssiResult(Event &e)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        
        // make sure we get at least a single measurements
        assert(e.data.size() != 0);
        size_t size = e.data.size();

        // all values must be below the CCA threshold
        bool channelFree = true;
        for (int i = 0; i < size; i++) {
            float rssi = boost::any_cast<float>(e.data[i]);
            if (rssi > ccaThreshold_x) {
                // FIXME: check if good
                channelFree = false;
                break;
            }
        }

        // inform MAC about CCA result
        Command reply;
        reply.trapWait = true;
        reply.engineName = x_engineMacModule;
        reply.componentName = x_componentMacModule;
        reply.commandName = "ccaresult";
        reply.data.push_back(boost::any(channelFree));
        postCommand(reply);
    }


    void MacPhyController::triggerCca(const std::string component, const std::string engine, const uint32_t duration)
    {
        Command command;
        command.trapWait = true;
        command.commandName = "rssirequest";
        command.componentName = x_componentSensingModule;
        command.engineName = x_engineSensingModule;
        command.data.push_back(boost::any(duration));
        postCommand(command);
    }


    void MacPhyController::destroy()
    {
    }

} /* namespace iris */
