/**
 * \file components/gpp/phy/SimpleTdmaTagger/SimpleTdmaTaggerComponent.cpp
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
 * A component that uses metadata to attach timestampts to datasets
 * passing through. It implements a simple TDMA based channel
 * scheduling algorithm operating on statically allocated slots with
 * a fixed length. It also assumes time synchronisation among nodes,
 * e.g. GPS or a common reference clock.
 *
 * Some ideas have been taken from Sam Leffler [1].
 * [1] http://people.freebsd.org/~sam/TDMAPresentation-20090921.pdf
 */

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "SimpleTdmaTagger.h"

using namespace std;
using namespace uhd;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, SimpleTdmaTagger);

SimpleTdmaTagger::SimpleTdmaTagger(std::string name)
    : PhyComponent(name,                      // component name
                   "simpletdmatagger",         // component type
                   "A simple TDMA tagger",     // description
                   "Andre Puschmann",          // author
                   "0.1"),                     // version
      maxSamplesPerSlot_(0)
{
    registerParameter(
                "slotlength",
                "Length of a slot within a SF in ms",
                "20",
                true,
                slotLength_x,
                Interval<uint32_t>(0,1000));

    registerParameter(
                "numslots",
                "Actual number of slots within SF",
                "10",
                true,
                numSlots_x,
                Interval<uint32_t>(0,1000));

    registerParameter(
                "myslotnumber",
                "Actual slot position within in SF",
                "0",
                true,
                mySlotNumber_x,
                Interval<uint32_t>(0,999));

    registerParameter(
                "guardlength",
                "Length of the guard interval",
                "0",
                true,
                guardLength_x,
                Interval<uint32_t>(0,1000));

    registerParameter(
                "dsptimebuffer",
                "DSP time buffer for next scheduled Tx",
                "0",
                true,
                dspTimeBuffer_x,
                Interval<int32_t>(-1000,1000));

    registerParameter(
                "worldclockoffset",
                "Local offset to world clock",
                "0",
                true,
                worldClockOffset_x,
                Interval<int32_t>(-1000,1000));

    registerParameter(
                "samplerate",
                "Sample rate of the transmitter",
                "2500000",
                true,
                sampleRate_x,
                Interval<uint32_t>(0,25000000));

    registerParameter(
                "args",
                "A delimited string which may be used to specify a particular usrp",
                "",
                false,
                args_x);

    registerParameter(
                "debug",
                "Whether to print debug information",
                "false",
                true,
                debug_x);
}

void SimpleTdmaTagger::registerPorts()
{
    registerInputPort("input1", TypeInfo< complex<float> >::identifier);
    registerOutputPort("output1", TypeInfo< complex<float> >::identifier);
}

void SimpleTdmaTagger::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    //One output type - always uint32_t
    outputTypes["output1"] = TypeInfo<complex<float> >::identifier;
}

void SimpleTdmaTagger::initialize()
{
    verifyParameter();

    // Set up the usrp
    try
    {
      // Create the device
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      LOG(LINFO) << "Creating the usrp device with args: " << args_x;
      usrp_ = uhd::usrp::multi_usrp::make(args_x);
    }
    catch(std::exception& e)
    {
      throw IrisException(e.what());
    }
}

void SimpleTdmaTagger::process()
{
    // Get a DataSet from the input DataBuffer
    DataSet<complex<float> >* readDataSet = NULL;
    getInputDataSet("input1", readDataSet);
    std::size_t size = readDataSet->data.size();

    if (size <= maxSamplesPerSlot_) {
        // Get a DataSet from the output DataBuffer
        DataSet<complex<float> >* writeDataSet = NULL;
        getOutputDataSet("output1", writeDataSet, size);

        // Copy the input DataSet to the output DataSet
        std::copy(readDataSet->data.begin(), readDataSet->data.end(), writeDataSet->data.begin());

        // Attach transmit time tag to dataset
        uhd::time_spec_t nextTx = getNextTransmitTime();
        writeDataSet->metadata.setMetadata("txtime", nextTx);

        releaseOutputDataSet("output1", writeDataSet);
    } else {
        LOG(LERROR) << "Too many samples to fit in time slot, abort.";
    }

    releaseInputDataSet("input1", readDataSet);
}


void SimpleTdmaTagger::parameterHasChanged(std::string name)
{
    verifyParameter();
}


void SimpleTdmaTagger::verifyParameter(void)
{
    // perfrom a few sanity checks on the given parameter
    if (mySlotNumber_x > (numSlots_x - 1)) {
        LOG(LERROR) << "Actual slot number can't be bigger than total number of slots.";
    }

    if (guardLength_x >= slotLength_x) {
        LOG(LERROR) << "Guard length can't be larger than slot length.";
    }
    // TODO: has minTimeOffset_x any contraint?

    // based on given parameter, calculate timespecs  as fractional seconds
    frameLength_ = (slotLength_x * numSlots_x) / double(1000);
    slotLength_ = slotLength_x / double(1000);
    guardLength_ = guardLength_x / double(1000);
    maxSamplesPerSlot_ = sampleRate_x / 1000 * (slotLength_x - guardLength_x); // maximum frame length allowed per slot
}


uhd::time_spec_t SimpleTdmaTagger::getNextTransmitTime(void)
{
    uhd::time_spec_t now = usrp_->get_time_now();

    // calculate start of next frame and my position therein
    double nextFrameStart = ceil(now.get_frac_secs() / frameLength_) * frameLength_;
    double mySlotStart = nextFrameStart + (mySlotNumber_x * slotLength_) + guardLength_;

    // take full seconds from current time and add
    uhd::time_spec_t txTime(now.get_full_secs());
    txTime += mySlotStart;

    if (debug_x) {
        std::cout << boost::format("USRP time %u full secs, %f frac secs")
                                    % now.get_full_secs() /* time_t */
                                    % now.get_frac_secs() << std::endl; /* in double */

        LOG(LDEBUG) << "frameLength_:  " << frameLength_;
        LOG(LDEBUG) << "nextFrameStart:  " << nextFrameStart;
        LOG(LDEBUG) << "mySlotStart:  " << mySlotStart;

        std::cout << boost::format("Tx time %u full secs, %f frac secs")
                                    % txTime.get_full_secs() /* time_t */
                                    % txTime.get_frac_secs() << std::endl; /* in double */
    }

    return txTime;
}

} // namesapce phy
} // namespace iris
