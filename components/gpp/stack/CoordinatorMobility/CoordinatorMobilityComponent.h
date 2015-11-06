/**
 * \file components/gpp/stack/CoordinatorMobility/CoordinatorMobilityComponent.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Implementation of a mobility protocol that has a coordinator that
 * determines a backup channel for a set of follower nodes.
 *
 */

#ifndef STACK_COORDINATORMOBILITYCOMPONENT_H_
#define STACK_COORDINATORMOBILITYCOMPONENT_H_

#include "irisapi/StackComponent.h"
#include <stdio.h>
#include "coordinatormobility.pb.h"
#include "utility/FllStackComponent.h"
#include "utility/StackHelper.h"
#if OSPECORR
#include "FllDrmHelper.h"
#endif

namespace iris
{
    using boost::mutex;
    using boost::condition_variable;
    using boost::shared_ptr;
    using boost::lock_guard;

class CoordinatorMobilityComponent: public FllStackComponent
{
private:
    //Exposed parameters
    std::string role_x; // coordinator or follower?
    uint32_t updateInterval_x;
    bool queryDrm_x;

    // local variables
    boost::scoped_ptr< boost::thread > beaconingThread_;

    // private functions
    void beaconingThreadFunction();
    void sendUpdateBackupChannelPacket(FllChannel backupChannel);
    bool findBackupChannel(FllChannel &bc);
    void FllChannelToMobilityChannel(const FllChannel fllchannel, MobilityChannel *mobilityChannel);
    FllChannel MobilityChannelToFllChannel(MobilityChannel *mobilityChannel);

public:
    CoordinatorMobilityComponent(std::string name);
    virtual ~CoordinatorMobilityComponent();

    virtual void initialize();
    virtual void processMessageFromAbove(shared_ptr<StackDataSet> set);
    virtual void processMessageFromBelow(shared_ptr<StackDataSet> set);

    virtual void start();
    virtual void stop();
};

} /* namespace iris */

#endif /* STACK_COORDINATORMOBILITYCOMPONENT_H_ */
