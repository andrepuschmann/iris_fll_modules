/**
 * \file components/gpp/phy/SimpleTdmaTagger/SimpleTdmaTaggerComponent.h
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

#ifndef PHY_SIMPLETDMATAGGER_H_
#define PHY_SIMPLETDMATAGGER_H_

#include <uhd/usrp/multi_usrp.hpp>
#include <math.h>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

class SimpleTdmaTagger
  : public PhyComponent
{
 public:
  SimpleTdmaTagger(std::string name);
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:
   // exposed parameter
   uint32_t slotLength_x;
   uint32_t numSlots_x;
   uint32_t mySlotNumber_x;
   uint32_t guardLength_x;
   int32_t  dspTimeBuffer_x;
   uint32_t sampleRate_x;
   int32_t worldClockOffset_x;
   std::string args_x;
   bool debug_x;

   // local parameter
   uhd::usrp::multi_usrp::sptr usrp_;
   uint32_t maxSamplesPerSlot_;
   double slotLength_;
   double guardLength_;
   double frameLength_;

   // local member functions
   void verifyParameter(void);
   boost::posix_time::ptime getUsrpTime(void);
   uhd::time_spec_t getNextTransmitTime(void);
};

} // namespace phy
} // namespace iris

#endif // PHY_SIMPLETDMATAGGER_H_
