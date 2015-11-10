/**
 * \file controllers/FlexibleLinkLayer/FllController.cpp
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
 * Implementation of the Flexibile Link Layer concept as an
 * Iris controller.
 */

#include <sstream>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "FllController.h"
#include "FllChannelManager.h"
#include "FllFsmBase.h"
#include "fsms/AgileReceiver/AgileReceiver.h"
#include "fsms/SpectrumMobility/SpectrumMobility.h"

using namespace std;
using namespace boost;

namespace iris
{
    //Export library functions
    IRIS_CONTROLLER_EXPORTS(FllController);

    FllController::FllController():
        Controller("fllcontroller", "Flexible Link Layer controller", "Andre Puschmann", "0.1"),
        ioService_(new boost::asio::io_service)
    {
        //Format: registerParameter(name, description, default, dynamic?, parameter, allowed values);
        //@todo: registerParameter("exampleparameter", "An example parameter", "0", true, x_example, Interval<uint32_t>(0,5));
        registerParameter("fsmName", "Name of the FSM to load on startup", "simplemobility", false, fsmName_x);

        registerParameter("dynamicParameter", "Comma separated list of key=value pairs", "", false, dynamicParameter_x);

        registerParameter("channelmode", "Channel configuration mode (simple,drm,random)", "simple", false, channelMode_x);

        registerParameter("dryrun", "Whether to actually perform reconfiguration or not", "false", false, dryRun_x);

        registerParameter("debug", "Enable debug output", "false", false, debug_x);

        registerEvent("EvNewOperatingChannel", "Notify that the current operating channel has changed", TypeInfo< uint32_t >::identifier);
    }


    /*! Initialize this controller
    *
    *   Do any initialization required by this controller.
    *   This function is guaranteed to be called by this controller's thread.
    */
    void FllController::initialize()
    {
        // parse dynamic parameters
        dynamicParameter_ = parseStringToParamterMap(dynamicParameter_x);

        // create and configure channel manager object
        LOG(LINFO) << "Initializing channel manager in " << channelMode_x << " mode.";
        channelManager_.reset(new FllChannelManager(this));
        channelManager_->setMode(channelMode_x);
        channelManager_->initialize();
        channelManager_->printChannelConfiguration();

        LOG(LINFO) << "Initializing Flexible Link Layer controller with FSM: " << fsmName_x;
        if (fsmName_x == "agilereceiver") {
            fsmBase_.reset(new AgileReceiver(*ioService_, this));
        } else
        if (fsmName_x == "spectrummobility") {
            fsmBase_.reset(new SpectrumMobility(*ioService_, this));
        } else {
            LOG(LERROR) << "FLL statemachine " << fsmName_x << " not found in repo. FLL FSM will not be started.";
        }

        if (fsmBase_) {
            fsmThread_.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, ioService_)));
        }
    }


    /*! Subscribe to events
    *
    *   In this function, the FLL registers for all possible event that could be
    *   handled by the FLL. Posting the events to the actual FSM will be done in
    *   processEvent()
    */
    void FllController::subscribeToEvents()
    {
        LOG(LINFO) << "Subscribing to events";
        subscribeToEvent("EvGetAvailableChannelsRequest", dynamicParameter_["channelclientmodule"]);
        subscribeToEvent("EvGetOperatingChannelRequest", dynamicParameter_["channelclientmodule"]);
        subscribeToEvent("EvUpdateBackupChannel", dynamicParameter_["channelclientmodule"]);
        subscribeToEvent("EvPrintChannelConfiguration", dynamicParameter_["channelclientmodule"]);
        subscribeToEvent("EvSetChannelState", dynamicParameter_["channelclientmodule"]);
        subscribeToEvent("EvLinkEstablished", dynamicParameter_["componentrendezvousmodule"]);
        subscribeToEvent("EvFrameDetected", dynamicParameter_["framedetectormodule"]);

        // register EvPuActive event for each transceiver
        int numTrx;
        try {
            numTrx = boost::lexical_cast<int>(getDynamicParameterValue("numtrx"));
        } catch (FllException &e) {
            // this shouldn't happen
            LOG(LERROR) << e.what();
        }
        for (int i = 0; i < numTrx; i++) {
            subscribeToEvent("EvPuActive" + std::to_string(i), dynamicParameter_["componentsensingmodule" + std::to_string(i)]);
        }
    }


    /*! Process an event
    *
    *   If an event which this controller has subscribed to is activated, this function is called by
    *   the controller thread.
    *
    *   \param  e   The event which was activated by the component
    */
    void FllController::processEvent(Event &e)
    {
        LOG(LDEBUG) << "processEvent: " << e.eventName;
        //!Tue event name string musst be small case!
        // try to handle generic FLL events first, otherwise pass them to the current FSM
        if (e.eventName == "evgetavailablechannelsrequest") {
            processEventGetAvailableChannels(e);
        } else if (e.eventName == "evgetoperatingchannelrequest") {
            processEventGetOperatingChannel(e);
        } else if (e.eventName == "evprintchannelconfiguration") {
            processEventPrintChannelConfiguration(e);
        } else {
            // FIXME: use EventManager and handle this inside FSM member
            if (fsmBase_)
                fsmBase_->processEvent(e);
        }
    }

    /**
     * Query channel manager and send list of availiable channels back
     */
    void FllController::processEventGetAvailableChannels(Event &e)
    {
        Command command;
        command.data.push_back(boost::any(channelManager_->getAvailableChannels()));
        command.engineName = dynamicParameter_["channelclientengine"];
        command.componentName = e.componentName; // or d_dynamicParameter["channelclientmodule"]
        command.commandName = "EvGetAvailableChannelsResult";
        command.trapWait = true;
        postCommand(command);
    }


    /**
     * Query channel manager and return current operating channel
     */
    void FllController::processEventGetOperatingChannel(Event &e)
    {
        Command command;
        int trx = 0;
        try
        {
            trx = boost::any_cast<int>(e.data[0]);
        }
        catch(const boost::bad_any_cast &)
        {
            throw FllException("Couldn't read transceiver id.");
        }
        command.data.push_back(boost::any(channelManager_->getOperatingChannel(trx)));
        command.engineName = dynamicParameter_["channelclientengine"];
        command.componentName = e.componentName; // or d_dynamicParameter["channelclientmodule"]
        command.commandName = "EvGetOperatingChannelResult";
        command.trapWait = true;
        postCommand(command);
    }


    /**
     * Print current channel configuration
     */
    void FllController::processEventPrintChannelConfiguration(Event &e)
    {
        channelManager_->printChannelConfiguration();
    }


    void FllController::destroy()
    {
        if (fsmBase_) {
            // Create deinit event and push it to FSM queue to transit into save state
            Event e;
            e.eventName = "EvStop";
            fsmBase_->processEvent(e);

            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
            ioService_->stop();
            fsmThread_->interrupt();
            fsmThread_->join();
        }
    }


    void FllController::reconfigureChannel(const FllChannel channel, const int trx)
    {
        LOG(LDEBUG) << "Reconfigure radio " << trx << " to " << channel.getFreq() << "Hz.";

        std::string engineRxModule;
        std::string componentRxModule;
        std::string engineTxModule;
        std::string componentTxModule;
        bool reconfigureRx = true;
        bool reconfigureTx = true;
        int delay = 0;

        // get relevent engine and component names
        try {
            engineRxModule = getDynamicParameterValue("enginerxmodule" + std::to_string(trx));
            componentRxModule = getDynamicParameterValue("componentrxmodule" + std::to_string(trx));
        } catch (FllException &) {
            // Rx configuration not given, ignore
            reconfigureRx = false;
        }
        try {
            engineTxModule = getDynamicParameterValue("enginetxmodule" + std::to_string(trx));
            componentTxModule = getDynamicParameterValue("componenttxmodule" + std::to_string(trx));
        } catch (FllException &) {
            // Tx configuration not given, ignore
            reconfigureTx = false;
        }
        try {
            delay = boost::lexical_cast<int>(getDynamicParameterValue("reconfdelay" + std::to_string(trx)));
        } catch (FllException &) {
            throw FllException("Reconfiguration delay for transceiver " + std::to_string(trx) + " not specifiied.");
        }

        // sanity check
        if (not (reconfigureTx || reconfigureRx)) {
            throw FllException("Neither Tx nor Rx parameters given.");
        }

        iris::ReconfigSet paramSet;
        if (reconfigureRx) {
            // freq
            iris::ParametricReconfig configRxFreq;
            configRxFreq.engineName = engineRxModule;
            configRxFreq.componentName = componentRxModule;
            configRxFreq.parameterName = "frequency";
            configRxFreq.parameterValue = boost::lexical_cast<std::string>(channel.getFreq());
            paramSet.paramReconfigs.push_back(configRxFreq);

            // rate
            iris::ParametricReconfig configRxRate;
            configRxRate.engineName = engineRxModule;
            configRxRate.componentName = componentRxModule;
            configRxRate.parameterName = "rate";
            configRxRate.parameterValue = boost::lexical_cast<std::string>(channel.getBandwidth());
            paramSet.paramReconfigs.push_back(configRxRate);
        }

        if (reconfigureTx) {
            // freq
            iris::ParametricReconfig configTxFreq;
            configTxFreq.engineName = engineTxModule;
            configTxFreq.componentName = componentTxModule;
            configTxFreq.parameterName = "frequency";
            configTxFreq.parameterValue = boost::lexical_cast<std::string>(channel.getFreq());
            paramSet.paramReconfigs.push_back(configTxFreq);

            // rate
            iris::ParametricReconfig configTxRate;
            configTxRate.engineName = engineTxModule;
            configTxRate.componentName = componentTxModule;
            configTxRate.parameterName = "rate";
            configTxRate.parameterValue = boost::lexical_cast<std::string>(channel.getBandwidth());
            paramSet.paramReconfigs.push_back(configTxRate);
        }

        // reconfigure and sleep for a moment
        if (!inDryRunMode()) {
            reconfigureRadio(paramSet);
            boost::this_thread::sleep(boost::posix_time::milliseconds(delay));
        } else {
            LOG(LINFO) << "Reconfiguration not taking place.";
        }

        channelManager_->setOperatingChannel(channel, trx);
        //activateEvent("EvNewOperatingChannel", channel);

        Event e;
        e.eventName = std::string("EvReconfDone" + std::to_string(trx));
        e.data.push_back(boost::any(channel));
        fsmBase_->processEvent(e);
    }


    void FllController::setGdtpOperationMode(bool value)
    {
        // turn on/off GDTP operation by setting passthrough parameter
        LOG(LDEBUG) << "Setting GDTP operation mode to " << value;

        std::string engineGdtpModule;
        std::string componentGdtpModule;

        // get relevent engine and component names
        try {
            engineGdtpModule = getDynamicParameterValue("enginegdtpmodule");
            componentGdtpModule = getDynamicParameterValue("componentgdtpmodule");
        } catch (FllException &e) {
            // this shouldn't happen
            LOG(LERROR) << e.what();
        }

        iris::ReconfigSet paramSet;
        iris::ParametricReconfig gdtpConfig;
        gdtpConfig.engineName = engineGdtpModule;
        gdtpConfig.componentName = componentGdtpModule;
        gdtpConfig.parameterName = "passthrough";
        gdtpConfig.parameterValue = boost::lexical_cast<std::string>(value);
        paramSet.paramReconfigs.push_back(gdtpConfig);

        // reconfigure and sleep for a moment
        if (!inDryRunMode()) {
            reconfigureRadio(paramSet);
        } else {
            LOG(LINFO) << "Reconfiguration not taking place.";
        }
    }


    /**
     * @brief Enable or disable sensing operation of a particular transceiver chain.
     * @param trx   The transceiver chain that includes the sensing module.
     * @param mode  Whether to enable or disable the module.
     */
    void FllController::setSensingMode(const int trx, const bool mode)
    {
        // trigger sensing operation
        std::string engineSensingModule;
        std::string componentSensingModule;

        // get relevent engine and component names
        try {
            engineSensingModule = getDynamicParameterValue("enginesensingmodule" + std::to_string(trx));
            componentSensingModule = getDynamicParameterValue("componentsensingmodule" + std::to_string(trx));
        } catch (FllException &e) {
            // this shouldn't happen
            LOG(LERROR) << e.what();
        }

        Command command;
        command.data.push_back(boost::any(mode));
        command.engineName = engineSensingModule;
        command.componentName = componentSensingModule;
        command.commandName = "EvSetSensingMode";
        command.trapWait = false;
        postCommand(command);
    }


    void FllController::updateChannelState(const FllChannel channel)
    {
        channelManager_->updateChannelState(channel);
    }


    /*! Parse std::string and extract key=value pairs
    *
    */
    DynamicParameter_t FllController::parseStringToParamterMap(const std::string str)
    {
        using namespace boost::spirit;
        DynamicParameter_t contents;
        std::string::const_iterator first = str.begin();
        std::string::const_iterator last  = str.end();

        const bool result = qi::phrase_parse(first,last,
                            *( *(qi::char_-"=")  >> qi::lit("=") >> *(qi::char_-",") >> -qi::lit(",") ),
                            ascii::space, contents);

        assert(result && first==last);
        return contents;
    }


    /*! return value to given parameter key
    *
    */
    std::string FllController::getDynamicParameterValue(const string name)
    {
        std::map<std::string, std::string>::iterator mapIt = dynamicParameter_.find(name);
        if (mapIt == dynamicParameter_.end() || (*mapIt).second.empty()) {
            std::stringstream errString;
            errString << "Parameter " << name << " not registered at FLL controller - exiting";
            throw FllException(errString.str());
        } else {
            return dynamicParameter_[name];
        }
    }

} /* namespace iris */
