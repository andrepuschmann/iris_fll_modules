/**
 * \file components/gpp/phy/BladeRfTx/BladeRfTxComponent.cpp
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
 * A sink component to use the BladeRf within Iris. This component
 * is based on the bladerf-cli command line tool provided by Nuand as
 * well as on the gr-osmosdr GNU Radio component.
 *
 * See the below sources for more information:
 * https://github.com/Nuand/bladeRF
 * http://git.osmocom.org/gr-osmosdr
 */

#include "BladeRfTxComponent.h"
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
IRIS_COMPONENT_EXPORTS(PhyComponent, BladeRfTxComponent);

BladeRfTxComponent::BladeRfTxComponent(std::string name)
  : PhyComponent(name,
                "bladerftx",
                "A BladeRf transmitter component",
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


BladeRfTxComponent::~BladeRfTxComponent()
{
    if (device_) {
        if (bladerf_enable_module(device_, BLADERF_MODULE_TX, false) != 0)
            LOG(LERROR) << "Couldn't shutdown BladeRF Tx module!";
        bladerf_close(device_);
    }
}


void BladeRfTxComponent::registerPorts()
{
    //Register all ports
    vector<int> validTypes;
    validTypes.push_back(TypeInfo< complex<float> >::identifier);

    //format:        (name, vector of valid types)
    registerInputPort("input1", validTypes);
}


void BladeRfTxComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    // No output types
}

//! Do any initialization required
void BladeRfTxComponent::initialize()
{
    // Set up the input DataBuffer
    inBuf_ = castToType< complex<float> >(inputBuffers.at(0));

    // Initialize raw sample vector to some multiple of block size
    rawSampleBuffer_.data.resize(128 * BLADERF_SAMPLE_BLOCK_SIZE);

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
        if (bladerf_is_fpga_configured(device_) != 1 ) {
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
                                  BLADERF_MODULE_TX,
                                  BLADERF_FORMAT_SC16_Q11,
                                  BLADERF_DEFAULT_STREAM_BUFFERS,
                                  BLADERF_DEFAULT_STREAM_SAMPLES,
                                  BLADERF_DEFAULT_STREAM_XFERS,
                                  BLADERF_SYNC_TIMEOUT_MS);
        if (ret != 0) {
            throw IrisException("Couldn't enable BladeRF Tx sync handle!");
            LOG(LERROR) << bladerf_strerror(ret);
        }

        // Turn on transmitter
        ret = bladerf_enable_module(device_, BLADERF_MODULE_TX, true);
        if ( ret != 0 ) {
            throw IrisException("Couldn't enable BladeRF Tx module!");
        }

        // Set sample rate
        uint32_t actualValue;
        ret = bladerf_set_sample_rate(device_, BLADERF_MODULE_TX, (uint32_t)rate_x, &actualValue);
        if (ret != 0) {
            throw IrisException("Failed to set sample rate!");
        }
        LOG(LINFO) << "Actual Tx sample rate is: " << actualValue << " Hz";

        // Set center frequency
        ret = bladerf_set_frequency(device_, BLADERF_MODULE_TX, frequency_x);
        if (ret != 0) {
            throw IrisException("Failed to set center frequency!");
        }
        bladerf_get_frequency(device_, BLADERF_MODULE_TX, &actualValue);
        LOG(LINFO) << "Actual Tx center frequency is: " << actualValue << " Hz";

        // Set bandwidth
        ret = bladerf_set_bandwidth(device_, BLADERF_MODULE_TX, bw_x, &actualValue);
        if (ret != 0) {
            throw IrisException("Failed to set receive bandwidth!");
        }
        LOG(LINFO) << "Actual Tx bandwidth is " << actualValue << " Hz";

        // Set VGA1 gain
        int actualGain;
        ret = bladerf_set_txvga1(device_, vga1Gain_x);
        if (ret != 0) {
            throw IrisException("Failed to set VGA1 gain!");
        }
        bladerf_get_txvga1(device_, &actualGain);
        LOG(LINFO) << "Actual VGA1 gain is " << actualGain << " dB";

        // Set VGA2 gain
        ret = bladerf_set_txvga2(device_, vga2Gain_x);
        if (ret != 0) {
            throw IrisException("Failed to set VGA2 gain!");
        }
        bladerf_get_txvga2(device_, &actualGain);
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


void BladeRfTxComponent::process()
{
    //Get a DataSet from the input DataBuffer
    DataSet< complex<float> >* readDataSet = NULL;
    inBuf_->getReadData(readDataSet);

    // Check if we have to append dummy frames, samps must be multiple of 1024
    size_t size = readDataSet->data.size();
    if (size % BLADERF_SAMPLE_BLOCK_SIZE != 0) {
        int num_samps_to_append = BLADERF_SAMPLE_BLOCK_SIZE - (size % BLADERF_SAMPLE_BLOCK_SIZE);
        std::vector<std::complex<float> > samples(num_samps_to_append, std::complex<float>(0.0, 0.0));
        readDataSet->data.insert(readDataSet->data.end(), samples.begin(), samples.end());
        size += num_samps_to_append;
    }

    // Adjust raw buffer if needed
    if (rawSampleBuffer_.data.capacity() < size) rawSampleBuffer_.data.resize(size);

    // convert samples to bladeRF format
    for (int i = 0; i < size; i++) {
        rawSampleBuffer_.data[i].real(0xa000 | (int16_t)((readDataSet->data[i].real()) * 2000));
        rawSampleBuffer_.data[i].imag(0x5000 | (int16_t)((readDataSet->data[i].imag()) * 2000));
    }

    // Write samples to device
    int ret = bladerf_sync_tx(device_, &(rawSampleBuffer_.data.front()), size, NULL, BLADERF_SYNC_TIMEOUT_MS);
    if (ret != 0) {
        throw IrisException("Failed to send samples to device!");
        LOG(LERROR) << bladerf_strerror(ret);
    }

    //Release the DataSet
    inBuf_->releaseReadData(readDataSet);
}

//! This gets called whenever a parameter is reconfigured
void BladeRfTxComponent::parameterHasChanged(std::string name)
{
  try
  {
    if(name == "frequency") {
        // Set center frequency
        uint32_t actualValue;
        int ret = bladerf_set_frequency(device_, BLADERF_MODULE_TX, frequency_x);
        if (ret != 0) {
            throw IrisException("Failed to set center frequency!");
        }
        bladerf_get_frequency(device_, BLADERF_MODULE_TX, &actualValue);
        LOG(LINFO) << "Actual Tx center frequency is: " << actualValue << " Hz";
    } else if(name == "rate") {
        uint32_t actualValue;
        int ret = bladerf_set_sample_rate(device_, BLADERF_MODULE_TX, (uint32_t)rate_x, &actualValue);
        if (ret != 0) {
            throw IrisException("Failed to set sample rate!");
        }
        LOG(LINFO) << "Actual Tx sample rate is: " << actualValue << " Hz";
    }
  }
  catch(std::exception &e)
  {
    throw IrisException(e.what());
  }
}


} // namespace phy
} // namespace iris
