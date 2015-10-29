/**
 * \file components/gpp/phy/EnergyDetector/EnergyDetectorComponent.cpp
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
 * A component for calculating the energy level of a given set
 * of I/Q samples using a simple IIR filter.
 *
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "EnergyDetectorComponent.h"
#include <algorithm>

using namespace std;

namespace iris
{
// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, EnergyDetectorComponent);


EnergyDetectorComponent::EnergyDetectorComponent(string name):
    PhyComponent(name, "energydetectorphycomponent", "A component for calculating the received energy level.", "Andre Puschmann", "0.1")
    ,ccaOver_(true)
    ,numSamples_(0)
    ,minNumSamples_(0)
    ,samplesPerProcess_(0)
    ,sampleRate_(0)
    ,rateSet_(false)
{
    //Format: registerParameter(name, description, default, dynamic?, parameter)
    registerParameter("debug", "Whether to output debug information",
        "false", true, debug_x);

    registerParameter("alpha", "Alpha value of the IIR filter",
        "0.2", true, alpha_x);

    registerParameter("isprobe", "Act as a probe (provide received energy via events)",
        "false", false, isProbe_x);

    registerParameter("ispassthrough", "Pass incoming samples through to output",
        "false", false, isPassthrough_x);

    registerParameter("issink", "Act as a sink (do not provide output)",
        "false", false, isSink_x);

    registerEvent("rssievent", "An event providing the current received energy level in dBm",
        TypeInfo< float >::identifier);
}


EnergyDetectorComponent::~EnergyDetectorComponent()
{
    if (commandThread_) {
        commandThread_->interrupt();
        commandThread_->join();
    }
}


void EnergyDetectorComponent::registerPorts()
{
    registerInputPort("input1", TypeInfo< complex<float> >::identifier);
    if (!isSink_x)
    {
        std::vector<int> types;
        if (isPassthrough_x) {
            types.push_back(TypeInfo< complex<float> >::identifier);
        } else {
            types.push_back(TypeInfo< float >::identifier);
        }
        types.push_back(TypeInfo< float >::identifier);
        registerOutputPort("output1", types);
    }
}

void EnergyDetectorComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    if (!isSink_x) {
        if (isPassthrough_x) {
            outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
        } else {
            outputTypes["output1"] = TypeInfo< float >::identifier;
        }
    }
}


void EnergyDetectorComponent::initialize()
{
    commandThread_.reset(new boost::thread(boost::bind(&EnergyDetectorComponent::commandHandler, this)));
}


void EnergyDetectorComponent::parameterHasChanged(std::string name)
{
}


void EnergyDetectorComponent::commandHandler()
{
    try {
        while (true) {
            boost::this_thread::interruption_point();
            //LOG(LDEBUG) << "Waiting for command ..";
            Command command = waitForCommand("rssirequest");
            {
                boost::unique_lock<boost::mutex> lock(mutex_);

                // read CCA duration
                assert(command.data.size() == 1);
                uint32_t ccaTime = boost::any_cast<uint32_t>(command.data[0]);

                // calculate number of measurements to be performed
                minNumSamples_ = ((double)ccaTime / (double)US_PER_S) * sampleRate_;
                minNumSamples_ = std::max(samplesPerProcess_, minNumSamples_);
                assert(minNumSamples_ > 0);

                // reset
                rssiVector_.clear();
                ccaOver_ = false;

                // wait until CCA is done ..
                while (not ccaOver_) {
                    ccaOverCond_.wait(lock);
                }

                // finally, trigger CCA result event
                activateEvent("rssiresult", rssiVector_);
            }
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in EnergyDetectorComponent: " << ex.what() << " - Command handler thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Command handler thread in EnergyDetectorComponent interrupted";
    }
}


float EnergyDetectorComponent::calculateEnergyLevel(std::vector<std::complex<float> >& data, const int len)
{
    float rssiHat = 0.0f;
    // this should help producing vectorized code if flags are enabled in compiler settings
    for (int i = 0; i < len; i++) {
        // compute magnitude squared
        float magnitude = data[i].real() * data[i].real() + data[i].imag() * data[i].imag();

        // update rssi estimate
        rssiHat = (1.0f - alpha_x) * rssiHat + alpha_x * magnitude;
    }
    return (10 * log10f(rssiHat));
}


void EnergyDetectorComponent::process()
{
    //Get a DataSet from the input DataBuffer
    DataSet< complex<float> >* readDataSet = NULL;
    getInputDataSet("input1", readDataSet);
    const std::size_t size = readDataSet->data.size();

    // calculate window width only once
    if (not rateSet_) {
        readDataSet->metadata.getMetadata("sampleRate", sampleRate_);
        samplesPerProcess_ = size;
        rateSet_ = true;
    }

    float tmp = 0.0;
    if (isProbe_x || not ccaOver_) {
        tmp = calculateEnergyLevel(readDataSet->data, size);
        if (not ccaOver_) {
            boost::unique_lock<boost::mutex> lock(mutex_);
            // append rssi to result vector until necessary samples are present
            if (numSamples_ < minNumSamples_) {
                numSamples_ += size;
                rssiVector_.push_back(tmp);
            } else {
                // signal end of CCA
                ccaOver_ = true;
                numSamples_ = 0; // reset counter
                ccaOverCond_.notify_one();
            }
        }
    }

    if (debug_x) {
        LOG(LDEBUG) << "RSSI: " << tmp;
    }
    if (isProbe_x) {
        activateEvent("rssievent", tmp);
    }

    if (not isSink_x) {
        if (isPassthrough_x) {
            // Pass data through
            DataSet< complex<float> >* writeDataSet = NULL;
            getOutputDataSet("output1", writeDataSet, size);
            writeDataSet->data = readDataSet->data;
            writeDataSet->metadata = readDataSet->metadata;
            releaseOutputDataSet("output1", writeDataSet);
        } else {
            // Output energy level
            DataSet< float >* writeDataSet = NULL;
            getOutputDataSet("output1", writeDataSet, 1);
            writeDataSet->data[0] = tmp;
            releaseOutputDataSet("output1", writeDataSet);
        }
    }

    releaseInputDataSet("input1", readDataSet);
}

} /* namespace iris */
