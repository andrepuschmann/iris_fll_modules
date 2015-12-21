/**
 * \file components/gpp/phy/BladeRfRx/BladeRfRxComponent.cpp
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
 * A source component to use the BladeRf within Iris. This component
 * is based on the bladerf-cli command line tool provided by Nuand as
 * well as on the gr-osmosdr GNU Radio component.
 *
 * See the below sources for more information:
 * https://github.com/Nuand/bladeRF
 * http://git.osmocom.org/gr-osmosdr
 */

#include "BladeRfRxComponent.h"
#include <boost/system/system_error.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/thread.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, BladeRfRxComponent);

BladeRfRxComponent::BladeRfRxComponent(std::string name)
  : PhyComponent(name,
                "bladerfrx",
                "A BladeRf receiver component",
                "Andre Puschmann",
                "0.1")
  ,device_(NULL)
{
  /*
   * format:
   * registerParameter(name,
   *                   description,
   *                   default value,
   *                   dynamic?,
   *                   parameter,
   *                   allowed values);
   */
  registerParameter("device",
                    "The bladeRF device to use",
                    "libusb: instance=0",
                    true,
                    deviceName_x);
  registerParameter("fpga",
                    "If this is non-empty,try to load the FPGA image",
                    "",
                    true,
                    fpgaImage_x);
  registerParameter("outputblocksize",
                    "How many samples to output in each block",
                    "1024",
                    true,
                    outputBlockSize_x);
  registerParameter("rate",
                    "The receive rate",
                    "1000000",
                    true,
                    rate_x);
  registerParameter("frequency",
                    "The receive frequency",
                    "2400000000",
                    true,
                    frequency_x);
  registerParameter("lnagain",
                    "The LNA gain",
                    "0",
                    true,
                    lnaGain_x);
  registerParameter("vga1gain",
                    "VGA1 gain",
                    "1",
                    true,
                    vga1Gain_x);
  registerParameter("vga2gain",
                    "VGA2 gain",
                    "1",
                    true,
                    vga2Gain_x);
  registerParameter("bw",
                    "IF filter bandwidth in Hz",
                    "0",
                    false,
                    bw_x);
}


BladeRfRxComponent::~BladeRfRxComponent()
{
    if (device_) {
        if (bladerf_enable_module(device_, BLADERF_MODULE_RX, false) != 0)
            LOG(LERROR) << "Couldn't shutdown BladeRF Rx module";
        bladerf_close(device_);
    }
}


void BladeRfRxComponent::registerPorts()
{
    //Register all ports
    vector<int> validTypes;
    validTypes.push_back(TypeInfo< complex<float> >::identifier);

    //format:        (name, vector of valid types)
    registerOutputPort("output1", validTypes);
}


void BladeRfRxComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    //One output type - complex<float>
    outputTypes["output1"] = TypeInfo< complex<float> >::identifier;
}

//! Do any initialization required
void BladeRfRxComponent::initialize()
{
    // Set up the output DataBuffers
    outBuf_ = castToType< complex<float> >(outputBuffers.at(0));
    rawSampleBuffer_.data.resize(outputBlockSize_x);

    // Set up the BladeRF
    try
    {
        // Create the device
        LOG(LINFO) << "Trying to open device " << deviceName_x;
        int ret = bladerf_open(&device_, deviceName_x.c_str());
        if (ret != 0) {
            throw IrisException("Failed to open bladeRF device!");
        }

        // Check whether FPGA is configured yet
        if (bladerf_is_fpga_configured(device_) == 1 ) {
            LOG(LINFO) << "FPGA is already configured.";
        } else {
            // try to load FPGA image
            if (not fpgaImage_x.empty()) {
                ret = bladerf_load_fpga(device_, fpgaImage_x.c_str());
                if (ret != 0) {
                    throw IrisException("Failed to load FPGA to bladeRF!");
                } else {
                    LOG(LINFO) << "FPGA image successfully loaded.";
                }
            } else {
                throw IrisException("BladeRF FPGA is not configured and no FPGA image given!");
            }
        }

        // Print some information about device
        struct bladerf_version version;
        if (bladerf_fw_version(device_, &version) == 0) {
            LOG(LINFO) << "Using FW " << version.describe;
        }
        if (bladerf_fpga_version(device_, &version) == 0) {
            LOG(LINFO) << "Using FPGA " << version.describe;
        }
        if (bladerf_is_fpga_configured(device_) != 1 ) {
            throw IrisException("BladeRF FPGA is not configured!");
        }

        // setting up sync config
        ret = bladerf_sync_config(device_,
                                  BLADERF_MODULE_RX,
                                  BLADERF_FORMAT_SC16_Q11,
                                  BLADERF_DEFAULT_STREAM_BUFFERS,
                                  BLADERF_DEFAULT_STREAM_SAMPLES,
                                  BLADERF_DEFAULT_STREAM_XFERS,
                                  BLADERF_SYNC_TIMEOUT_MS);
        if (ret != 0) {
            throw IrisException("Couldn't enable BladeRF Tx sync handle!");
            LOG(LERROR) << bladerf_strerror(ret);
        }

        // Turn on receiver
        ret = bladerf_enable_module(device_, BLADERF_MODULE_RX, true);
        if ( ret != 0 ) {
            throw IrisException("Couldn't enable BladeRF Rx module!");
        }

        // Set sample rate
        uint32_t actualValue;
        ret = bladerf_set_sample_rate(device_, BLADERF_MODULE_RX, (uint32_t)rate_x, &actualValue);
        if (ret != 0) {
            throw IrisException("Failed to set sample rate!");
        }
        LOG(LINFO) << "Actual Rx sample rate is: " << actualValue << " Hz";

        // Set center frequency
        ret = bladerf_set_frequency(device_, BLADERF_MODULE_RX, frequency_x);
        if (ret != 0) {
            throw IrisException("Failed to set center frequency!");
        }
        bladerf_get_frequency(device_, BLADERF_MODULE_RX, &actualValue);
        LOG(LINFO) << "Actual Rx center frequency is: " << actualValue << " Hz";

        // Set bandwidth
        ret = bladerf_set_bandwidth(device_, BLADERF_MODULE_RX, bw_x, &actualValue);
        if (ret != 0) {
            throw IrisException("Failed to set receive bandwidth!");
        }
        LOG(LINFO) << "Actual Rx bandwidth is " << actualValue << " Hz";

        // Set LNA gain
        bladerf_lna_gain gain;
        if (lnaGain_x == 0.0) {
            gain = BLADERF_LNA_GAIN_BYPASS;
        } else if(lnaGain_x == 3.0 ) {
            gain = BLADERF_LNA_GAIN_MID;
        } else if(lnaGain_x == 6.0 ) {
            gain = BLADERF_LNA_GAIN_MAX;
        } else {
            LOG(LERROR) << "Invalid gain setting, turning off LNA gain!";
            gain = BLADERF_LNA_GAIN_BYPASS;
        }
        ret = bladerf_set_lna_gain(device_, gain);
        if (ret != 0) {
            throw IrisException("Failed to set LNA gain!");
        }
        ret = bladerf_get_lna_gain(device_, &gain);
        LOG(LINFO) << "Actual LNA gain is " << (gain == BLADERF_LNA_GAIN_BYPASS ? 0 : gain == BLADERF_LNA_GAIN_MID ? 3 : 6) << " dB";

        // Set VGA1 gain
        int actualGain;
        ret = bladerf_set_rxvga1(device_, vga1Gain_x);
        if (ret != 0) {
            throw IrisException("Failed to set VGA1 gain!");
        }
        bladerf_get_rxvga1(device_, &actualGain);
        LOG(LINFO) << "Actual VGA1 gain is " << actualGain << " dB";

        // Set VGA2 gain
        ret = bladerf_set_rxvga2(device_, vga2Gain_x);
        if (ret != 0) {
            throw IrisException("Failed to set VGA2 gain!");
        }
        bladerf_get_rxvga2(device_, &actualGain);
        LOG(LINFO) << "Actual VGA2 gain is " << actualGain << " dB";
    }
    catch(const boost::exception &e)
    {
        throw IrisException(boost::diagnostic_information(e));
    }
    catch(std::exception& e)
    {
        throw IrisException(e.what());
    }
}


void BladeRfRxComponent::process()
{
    //Get a DataSet from the output DataBuffer
    DataSet< complex<float> >* writeDataSet = NULL;
    outBuf_->getWriteData(writeDataSet, outputBlockSize_x);

    int16_t si, sq;
    struct bladerf_metadata metadata;

    // Read samples from device
    int num_rx_samps = rawSampleBuffer_.data.size();
    int ret = bladerf_sync_rx(device_, &(rawSampleBuffer_.data.front()), num_rx_samps, &metadata, BLADERF_SYNC_TIMEOUT_MS);
    if (ret != 0) {
        throw IrisException("Failed to read samples from device!");
        LOG(LERROR) << bladerf_strerror(ret);
    }

    // Convert wireformat to complex<float> (FIXME: convert to std::transform)
    for (size_t i = 0; i < num_rx_samps; ++i) {
        si = rawSampleBuffer_.data.at(i).real() & 0xfff;
        sq = rawSampleBuffer_.data.at(i).imag() & 0xfff;

        // Sign extend the 12-bit IQ values, if needed
        if( si & 0x800 ) si |= 0xf000;
        if( sq & 0x800 ) sq |= 0xf000;

        std::complex<float> sample((float)si * (1.0f/2048.0f),
                                   (float)sq * (1.0f/2048.0f));

        writeDataSet->data[i] = sample;
    }

    writeDataSet->metadata.setMetadata("sampleRate", rate_x);
    writeDataSet->metadata.setMetadata("timeStamp", 0.0 );

    //Release the DataSet
    outBuf_->releaseWriteData(writeDataSet);
}

//! This gets called whenever a parameter is reconfigured
void BladeRfRxComponent::parameterHasChanged(std::string name)
{
  try
  {
    if(name == "frequency")
    {
        // change freq here
    }
    else if(name == "rate")
    {
        // change rate here
    }
    else if(name == "gain")
    {
        // change gain here
    }
  }
  catch(std::exception &e)
  {
    throw IrisException(e.what());
  }
}


} // namespace phy
} // namespace iris
