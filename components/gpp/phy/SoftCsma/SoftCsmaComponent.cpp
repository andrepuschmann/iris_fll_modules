/**
 * \file components/gpp/phy/SoftCsma/SoftCsmaComponent.cpp
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
 * A component implementing the carrier sense multiple access (CSMA) strategy.
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "SoftCsmaComponent.h"
#include <algorithm>

using namespace std;

namespace iris
{
// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, SoftCsmaComponent);


SoftCsmaComponent::SoftCsmaComponent(string name):
    PhyComponent(name, "softcsmaphycomponent", "A component implementing the carrier sense multiple access strategy.", "Andre Puschmann", "0.1")
    ,currentCw_(0)
    ,dataFrameSent_(false)
    ,backoffOver_(true)
{
    //Format: registerParameter(name, description, default, dynamic?, parameter)
    registerParameter("debug", "Whether to output debug information",
        "false", true, debug_x);

    registerParameter("hascca", "Whether to perform clear channel assessment before transmitting",
        "true", false, hasCca_x);

    registerParameter("hasdcfbehavior", "Whether to behave like 802.11 DCF algorithm",
        "true", false, hasDcfBehavior_x);

    registerParameter("hasposttxbackoff", "Whether to execute backoff algorithm after every transmission",
        "true", false, hasPostTxBackoff_x);

    registerParameter("hasburstack", "Whether to expect an burst ACK after every transmission",
        "true", false, hasBurstAck_x);

    registerParameter("mincw", "Minimal length of contention window",
        "31", false, x_minCw);

    registerParameter("maxcw", "Maximal length of contention window",
        "1023", false, x_maxCw);

    registerParameter("slottime", "Slottime in microseconds",
        "1000", false, x_slotTime);

    registerParameter("difs", "DIFS in microseconds",
        "2000", false, x_difs);

    registerEvent("ccarequest", "A event for triggering CCA request",
        TypeInfo< uint32_t >::identifier);
}


SoftCsmaComponent::~SoftCsmaComponent()
{
    if (backoffThread_) {
        backoffThread_->interrupt();
        backoffThread_->join();
    }
}


void SoftCsmaComponent::registerPorts()
{
    registerInputPort("input1", TypeInfo< complex<float> >::identifier);
    registerOutputPort("output1", TypeInfo< complex<float> >::identifier);

}

void SoftCsmaComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}


void SoftCsmaComponent::initialize()
{
    resetCw();
    backoffThread_.reset(new boost::thread(boost::bind(&SoftCsmaComponent::backoffThreadFunction, this)));
}

void SoftCsmaComponent::parameterHasChanged(std::string name)
{
}

void SoftCsmaComponent::process()
{
    // Get a DataSet from the input DataBuffer
    DataSet< complex<float> >* readDataSet = NULL;
    getInputDataSet("input1", readDataSet);

    // Copy input dataset to output dataset
    DataSet< complex<float> >* writeDataSet = NULL;
    getOutputDataSet("output1", writeDataSet, readDataSet->data.size());
    writeDataSet->data = readDataSet->data;
    writeDataSet->metadata = readDataSet->metadata;

    LOG(LDEBUG) << "Processing " << readDataSet->data.size() << " Bytes of data.";

    // wait for burst ACK event
    if (hasBurstAck_x && dataFrameSent_) {
        Command command = waitForCommand("burstack");
    }

    // wait until posttx backoff is finished
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (not backoffOver_) {
            backoffOverCond_.wait(lock);
        }
    }

    // assume frame is data frame
    bool isData = true;
    if (readDataSet->metadata.hasMetadata("isdata"))
        readDataSet->metadata.getMetadata("isdata", isData); // isData is not touched if metadata is not found
    //LOG(LDEBUG) << "Frame len: " << readDataSet->data.size() << " isData: " << isData;

    // FIXME: read metadata an query transmission attempt to adjust CW accordingly

    if (hasCca_x and isData) {
        if (hasDcfBehavior_x) {
            // perform normal DCF operation for data frames
            LOG(LDEBUG) << "Perform CCA for DIFS (" << x_difs << "us)";
            if (getChannelState(x_difs) == false) {
                // this is the backoff procedure (http://www.sss-mag.com/pdf/802_11tut.pdf, page 9)
                // wait until channel gets free, draw a new random backoff and start
                // decrementing it until zero if channel is free
                // FIXME: retrieve retransmission counter from metadata
                // FIXME: wait for entire DIFS before backoff
                uint32_t backoff = RandomGenerator::getInstance().Uniform0toN(currentCw_);
                LOG(LDEBUG) << "Not free for DIFS: backoff: " << backoff << " cw: " << currentCw_;
                while (backoff) {
                    if (getChannelState(x_slotTime) == true) {
                        backoff--;
                    }
                }
            } else {
                LOG(LDEBUG) << "Free for DIFS!";
            }
        } else {
            // keep sensing until channel is free
            // this is useful for determining the minimum CCA duration
            while (getChannelState(0) == false);
        }
    }

    dataFrameSent_ = isData; // save frame type
    if (hasPostTxBackoff_x and dataFrameSent_) {
        boost::unique_lock<boost::mutex> lock(mutex_);
        backoffOver_ = false;
        backoffOverCond_.notify_one();
    }

    releaseOutputDataSet("output1", writeDataSet);
    releaseInputDataSet("input1", readDataSet);
}

// This thread performs the posttx backoff waiting asynchronously
void SoftCsmaComponent::backoffThreadFunction()
{
    try {
        while (true) {
            boost::this_thread::interruption_point();

            boost::unique_lock<boost::mutex> lock(mutex_);
            while (backoffOver_) {
                backoffOverCond_.wait(lock);
            }

            // perform post tx backoff
            uint32_t backoff = RandomGenerator::getInstance().Uniform0toN(x_minCw);
            boost::this_thread::sleep(boost::posix_time::microseconds(backoff * x_slotTime));
            backoffOver_ = true;
            backoffOverCond_.notify_one();
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in SoftCsmaComponent: " << ex.what() << " - Backoff thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Backoff thread in SoftCsmaComponent interrupted";
    }
}

/*!
 * This function issues a CCA request event and queries the
 * sensing controller (configured in the XML)
 * whether the channel is occupied or not.
 *
 * @param duration CCA duration in us
 * @return true if the channel is free, false otherwise
 *
 */
bool SoftCsmaComponent::getChannelState(const uint32_t duration)
{
    // activate event and pass CCA duration (in us) as parameter
    activateEvent("ccarequest", duration);
    //LOG(LDEBUG) << "CCA request sent, wait for cmd";
    Command reply = waitForCommand("ccaresult");
    //LOG(LDEBUG) << "CCA result received";
    assert(reply.data.size() == 1);
    return boost::any_cast<bool>(reply.data[0]);
}


void SoftCsmaComponent::doubleCw(void)
{
    currentCw_ *= 2;
    currentCw_ = std::min(currentCw_, x_maxCw);
}


void SoftCsmaComponent::resetCw(void)
{
    currentCw_ = x_minCw;
}


} /* namespace iris */
