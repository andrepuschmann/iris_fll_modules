/**
 * \file components/gpp/phy/MetadataDetecto/MetadataDetectorComponent.cpp
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
 * A simple component that tries to find a certain pattern inside
 * the metadata passed through it. If the pattern has been found,
 * a PU detection event is triggerd.
 *
 */

#include "MetadataDetectorComponent.h"

using namespace std;
using namespace boost::posix_time;

namespace iris
{
// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, MetadataDetectorComponent);

MetadataDetectorComponent::MetadataDetectorComponent(string name) :
    PhyComponent(name, "metadatadetectorcomponent", "Metadata detector", "Andre Puschmann", "0.1")
{
    registerParameter("key", "Metadata key to check for", "none", false, key_x);
    registerParameter("value", "Value of the key", "none", false, value_x);
    registerParameter("interval", "Interval for periodic detection in ms (-1 means no peridic detection)", "1000", false, interval_x);

    registerParameter("hasevent", "Whether to activate event after match", "true", false, hasEvent_x);
    registerParameter("eventname", "The name of the event to trigger", "true", false, eventName_);

    registerParameter("manualsensing", "If changed, the component will trigger one sensing cycle", "1", false, manualDetection_);
    registerParameter("debug", "Enable debug output", "false", false, debug_x);
}

MetadataDetectorComponent::~MetadataDetectorComponent()
{
    if (commandThread_) {
        commandThread_->interrupt();
        commandThread_->join();
    }
}

void MetadataDetectorComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

void MetadataDetectorComponent::registerPorts()
{
    registerInputPort("input1", TypeInfo< uint8_t >::identifier);
    registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

void MetadataDetectorComponent::initialize()
{
    registerEvent(eventName_, "An event to trigger only if PU has been detected", TypeInfo< uint8_t >::identifier);
    nextRefresh_ = boost::get_system_time();
    if (interval_x == -1) {
        // periodic sensing disabled, set increment to infinity
        LOG(LINFO) << "Periodic detection disabled.";
        intervalIncrement_ = pos_infin;
        nextRefresh_ += intervalIncrement_; // set to infinity
    } else {
        // set increment to given parameter (in milliseconds)
        LOG(LINFO) << "Periodic detection interval set to " << interval_x << "ms!";
        intervalIncrement_ = milliseconds(interval_x);
    }
    commandThread_.reset(new boost::thread(boost::bind(&MetadataDetectorComponent::commandHandler, this)));
}


void MetadataDetectorComponent::commandHandler()
{
    try {
        while (true) {
            boost::this_thread::interruption_point();
            //LOG(LDEBUG) << "Waiting for command ..";
            Command command = waitForCommand("EvSetSensingMode");
            {
                boost::unique_lock<boost::mutex> lock(mutex_);
                assert(command.data.size() == 1);
                enableSensing_ = boost::any_cast<bool>(command.data[0]);
            }
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in MetadataDetectorComponent: " << ex.what() << " - Command handler thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Command handler thread in MetadataDetectorComponent interrupted";
    }
}


void MetadataDetectorComponent::process()
{
    DataSet< uint8_t >* in = NULL;
    DataSet< uint8_t >* out = NULL;

    getInputDataSet("input1", in);
    getOutputDataSet("output1", out, in->data.size());

    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        // check for next refresh
        if ((boost::get_system_time() >= nextRefresh_) || enableSensing_) {
            bool patternFound = false;
            // extract string from metadata, if any
            std::string tmp;
            in->metadata.getMetadata(key_x, tmp);
            // match value
            if (not tmp.empty()) {
                if (tmp.find(value_x) != std::string::npos) {
                    patternFound = true;
                }
            }

            if (hasEvent_x and patternFound) {
                uint8_t eventData = patternFound;
                activateEvent(eventName_, eventData);
                if (debug_x) LOG(LDEBUG) << "Activate " << eventName_;
                enableSensing_ = false;
            }

            // calculate next refresh
            nextRefresh_ = boost::get_system_time() + intervalIncrement_;
        }

    }

    // just copy entire content of input port to output port
    std::copy(in->data.begin(), in->data.end(), out->data.begin());

    releaseOutputDataSet("output1", out);
    releaseInputDataSet("input1", in);
}

void MetadataDetectorComponent::parameterHasChanged(std::string name)
{
    if (name == "manualsensing") {
        // just set nextfresh to now to detection sensing in next process() cycle
        nextRefresh_ = boost::get_system_time();
    }
}

} /* namespace iris */
