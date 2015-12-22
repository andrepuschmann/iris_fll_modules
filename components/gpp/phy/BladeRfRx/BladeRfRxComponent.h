/**
 * \file components/gpp/phy/BladeRfRx/BladeRfRxComponent.h
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
 * A source component to use the BladeRf within Iris.
 */

#ifndef PHY_BLADERFRXCOMPONENT_H_
#define PHY_BLADERFRXCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <libbladeRF.h>

#define BLADERF_SAMPLE_BLOCK_SIZE (1024)
#define BLADERF_SYNC_TIMEOUT_MS (500)
#define BLADERF_DEFAULT_STREAM_XFERS (16)
#define BLADERF_DEFAULT_STREAM_BUFFERS (32)
#define BLADERF_DEFAULT_STREAM_SAMPLES (8192)

namespace iris
{
namespace phy
{

class BladeRfRxComponent
  : public PhyComponent
{
public:
  BladeRfRxComponent(std::string name);
  ~BladeRfRxComponent();
  virtual void calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

private:
  std::string deviceName_x;     //!< The device string to identify the bladeRF
  std::string fpgaImage_x;      //!< FPGA Image filename
  int outputBlockSize_x;        //!< Output block size
  uint32_t frequency_x;         //!< Receive frequency
  double rate_x;                //!< Receive rate
  uint32_t bw_x;                //!< Daughterboard IF filter bandwidth in Hz
  uint32_t lnaGain_x;           //!< LNA gain
  uint32_t vga1Gain_x;          //!< Receiver variable gain 1
  uint32_t vga2Gain_x;          //!< Receiver variable gain 2

  struct bladerf *device_;
  DataSet< std::complex<short> > rawSampleBuffer_;
  WriteBuffer< std::complex<float> >* outBuf_;  //!< Output DataBuffer
};

} // namespace phy
} // namespace iris

#endif // PHY_BLADERFRXCOMPONENT_H_
