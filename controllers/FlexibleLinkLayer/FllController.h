/**
 * \file controllers/FlexibleLinkLayer/FllController.h
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

#ifndef FLLCONTROLLER_H_
#define FLLCONTROLLER_H_

#include "irisapi/Controller.h"
#include <memory>
#include <boost/spirit/include/qi.hpp>         // Parsing
#include <boost/fusion/adapted/std_pair.hpp>   // Make std::pair a fusion vector
#include <boost/asio.hpp>
#include "FllException.h"
#include "FllChannel.h"


namespace iris
{
// forward declaration of FllFsmBase
class FllFsmBase;
class FllChannelManager;

typedef std::map<std::string, std::string> DynamicParameter_t;

class FllController: public Controller
{
private:
    std::string fsmName_x; // the name of the FLL state machine to load on startup
    std::string dynamicParameter_x; // a comma separted list of key=value pairs
    std::string channelMode_x;
    bool dryRun_x;
    bool debug_x;

    // local variables
    DynamicParameter_t dynamicParameter_;
    std::shared_ptr<FllChannelManager> channelManager_;
    std::shared_ptr< boost::asio::io_service > ioService_;
    std::unique_ptr<FllFsmBase> fsmBase_;
    std::unique_ptr< boost::thread > fsmThread_;

    // private methods
    DynamicParameter_t parseStringToParamterMap(const std::string str);

    // event handler
    void processEventGetAvailableChannels(Event &e);
    void processEventGetOperatingChannel(Event &e);
    void processEventPrintChannelConfiguration(Event &e);

public:
    FllController();
    virtual void subscribeToEvents();
    virtual void initialize();
    virtual void processEvent(Event &e);
    virtual void destroy();

    //void setFsmHandle(FllFsmBase *handle) { fsmBase_ = handle; }
    std::shared_ptr<FllChannelManager> getChannelManagerHandle() {
        if (channelManager_)
            return channelManager_;
    }
    bool inDryRunMode(void) { return dryRun_x; }
    std::string getDynamicParameterValue(const std::string name);
    void reconfigureChannel(const FllChannel channel, const int trx);
    void setGdtpOperationMode(bool value);
    void setSensingMode(const int trx, const bool mode);
    void updateChannelState(const FllChannel channel);
};

} /* namespace iris */

#endif /* FLLCONTROLLER_H_ */
