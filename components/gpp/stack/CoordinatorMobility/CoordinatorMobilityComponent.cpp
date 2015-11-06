/**
 * \file components/gpp/stack/CoordinatorMobility/CoordinatorMobilityComponent.cpp
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
 * Implementation of a mobility protocol that has a coordinator that
 * determines a backup channel for a set of follower nodes.
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "CoordinatorMobilityComponent.h"

using namespace std;
using namespace boost;

namespace iris
{
// export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, CoordinatorMobilityComponent);

CoordinatorMobilityComponent::CoordinatorMobilityComponent(std::string name)
    : FllStackComponent(name,
                     "coordinatorcstackcomponent",
                     "A very simple mobility protocol with coordinator/follower roles",
                     "Andre Puschmann",
                     "0.1")
{
    //Format: registerParameter(name, description, default, dynamic?, parameter, allowed values);
    registerParameter("role", "Role of node", "coordinator", false, role_x);
    registerParameter("updateinterval", "Interval between BC updates", "1000", true, updateInterval_x);
    registerParameter("querydrm", "Whether to use DRM to determine backupchannel", "false", true, queryDrm_x);

    //Format: registerEvent(name, description, data type);
    registerEvent("EvGetAvailableChannelsRequest", "Retrieve list of available channels from FllController", TypeInfo< int32_t >::identifier);
    registerEvent("EvGetOperatingChannelRequest", "Retrieve current operating channel from FllController", TypeInfo< int32_t >::identifier);
    registerEvent("EvUpdateBackupChannel", "Set new backup channel", TypeInfo< int32_t >::identifier);
}


CoordinatorMobilityComponent::~CoordinatorMobilityComponent()
{
}


void CoordinatorMobilityComponent::initialize()
{
    // turn off logging to file, system will take care of closing file handle
    //LoggingPolicy::getPolicyInstance()->setFileStream(NULL);
}


void CoordinatorMobilityComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> incomingFrame)
{
    //StackHelper::printDataset(incomingFrame, "vanilla from above");
}


void CoordinatorMobilityComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> incomingFrame)
{
    //StackHelper::printDataset(incomingFrame, "vanilla from below");
    MobilityPacket updatePacket;
    StackHelper::deserializeAndStripDataset(incomingFrame, updatePacket);

    switch(updatePacket.type()) {
        case MobilityPacket::UPDATE_BC:
        {
            if (updatePacket.source() == MobilityPacket::COORDINATOR) {
                // take first channel in update packet as backup channel
                if (updatePacket.channelmap_size() > 0) {
                    MobilityChannel *mobilityChannel = updatePacket.mutable_channelmap(0);
                    FllChannel fllChannel = MobilityChannelToFllChannel(mobilityChannel);
                    LOG(LINFO) << "Received new backup channel from coordinator: " << fllChannel.getFreq() << "Hz.";
                    setBackupChannel(fllChannel);
                }
            }
            break;
        }
        default:
            LOG(LERROR) << "Undefined packet type.";
            break;
    } // switch
}


void CoordinatorMobilityComponent::start()
{
    // start beaconing thread in coordinator mode
    LOG(LINFO) << "I am a " << role_x << " node.";
    if (role_x == "coordinator") {
        LOG(LINFO) << "Start beaconing thread  ..";
        beaconingThread_.reset(new boost::thread(boost::bind( &CoordinatorMobilityComponent::beaconingThreadFunction, this)));
    } else
    if (role_x == "follower") {
        LOG(LINFO) << "I am waiting for beacons ..";
    } else {
        LOG(LERROR) << "Mode is not defined: " << role_x;
    }

    FllChannelVector channels = getAvailableChannels();
    LOG(LINFO) << "No of channels: " << channels.size();
}


void CoordinatorMobilityComponent::stop()
{
    // stop threads
    if (beaconingThread_) {
        beaconingThread_->interrupt();
        beaconingThread_->join();
    }
}


void CoordinatorMobilityComponent::beaconingThreadFunction()
{
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    LOG(LINFO) << "Beaconing thread started.";

    try
    {
        while(true)
        {
            boost::this_thread::interruption_point();

            // determine backup channel and transmit beacon
            FllChannel bc;
            if (findBackupChannel(bc) == true) {
                setBackupChannel(bc);
                sendUpdateBackupChannelPacket(bc);
            }

            // sleep until next beacon
            boost::this_thread::sleep(boost::posix_time::milliseconds(updateInterval_x));
        } // while
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in CoordinatorMobility component: " << ex.what() << " - Beaconing thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Thread " << boost::this_thread::get_id() << " in stack component interrupted.";
    }
}


void CoordinatorMobilityComponent::sendUpdateBackupChannelPacket(FllChannel backupChannel)
{
    MobilityPacket updatePacket;
    updatePacket.set_source(MobilityPacket::COORDINATOR);
    updatePacket.set_destination(MobilityPacket::FOLLOWER);
    updatePacket.set_type(MobilityPacket::UPDATE_BC);

    MobilityChannel *channel = updatePacket.add_channelmap();
    FllChannelToMobilityChannel(backupChannel, channel);

    shared_ptr<StackDataSet> buffer(new StackDataSet);
    StackHelper::mergeAndSerializeDataset(buffer, updatePacket);
    //StackHelper::printDataset(buffer, "UpdateBC packet Tx");

    sendDownwards(buffer);
    LOG(LINFO) << "Tx UpdateBC packet ";
}

void CoordinatorMobilityComponent::FllChannelToMobilityChannel(const FllChannel fllchannel, MobilityChannel *mobilityChannel)
{
    mobilityChannel->set_f_center(fllchannel.getFreq());
    mobilityChannel->set_bandwidth(fllchannel.getBandwidth());
    switch (fllchannel.getState()) {
        case FREE: mobilityChannel->set_status(MobilityChannel_Status_FREE); break;
        case BUSY_SU: mobilityChannel->set_status(MobilityChannel_Status_BUSY_SU); break;
        case BUSY_PU: mobilityChannel->set_status(MobilityChannel_Status_BUSY_PU); break;
        default: LOG(LERROR) << "Unknown channel state.";
    }
}

FllChannel CoordinatorMobilityComponent::MobilityChannelToFllChannel(MobilityChannel *mobilityChannel)
{
    FllChannel channel;
    channel.setFreq(mobilityChannel->f_center());
    channel.setBandwidth(mobilityChannel->bandwidth());

    switch (mobilityChannel->status()) {
        case MobilityChannel_Status_FREE: channel.setState(FREE); break;
        case MobilityChannel_Status_BUSY_SU: channel.setState(BUSY_SU); break;
        case MobilityChannel_Status_BUSY_PU: channel.setState(BUSY_PU); break;
        default: LOG(LERROR) << "Unknown channel state.";
    }
    return channel;
}


bool CoordinatorMobilityComponent::findBackupChannel(FllChannel& bc)
{
    FllChannelVector channels = getAvailableChannels();
    FllChannel operatingChannel = getOperatingChannel(0); // get operating channel of primary receiver, i.e. trx 0

    #if OSPECORR
    if (queryDrm_x) {
        bool ret = FllDrmHelper::getChannel(bc);
        if (ret == true && bc != operatingChannel) {
            // success, got valid backup channel from DRM
            LOG(LINFO) << "Got backup channel at " << bc.getFreq() << " from DRM.";
            return true;
        } else {
            return false;
        }
    }
    #endif

    // find free channel other than operating channel in set of available channels
    FllChannelVector::iterator it;
    bool channelFound = false;
    for (it = channels.begin(); it != channels.end(); ++it) {
        if (it->getState() == FREE && *it != operatingChannel) {
            LOG(LINFO) << "Free channel found: " << it->getFreq() << "Hz.";
            bc = *it;
            channelFound = true;
            break;
        }
    }

    if (not channelFound) {
        LOG(LERROR) << "No free backup channel found. Skip update.";
    }
    return channelFound;
}


} /* namespace iris */
