/**
 * \file components/gpp/phy/SoftCsma/SoftCsmaComponent.h
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

#ifndef SOFTCSMACOMPONENT_H_
#define SOFTCSMACOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include "utility/RandomGenerator.h"

namespace iris
{
class SoftCsmaComponent: public PhyComponent
{
public:
    SoftCsmaComponent(std::string name);
    virtual ~SoftCsmaComponent();
    virtual void calculateOutputTypes(
        std::map<std::string, int>& inputTypes,
        std::map<std::string, int>& outputTypes);
    virtual void registerPorts();
    virtual void initialize();
    virtual void parameterHasChanged(std::string name);
    virtual void process();

private:
    //Exposed parameters
    bool debug_x;               ///< Whether to output debug information
    bool hasCca_x;              ///< Whether to perform clear channel assessment before transmitting
    bool hasDcfBehavior_x;      ///< Whether to behave like 802.11 DCF
    bool hasPostTxBackoff_x;    ///< Whether to execute posttx-backoff after every transmission
    bool hasBurstAck_x;         ///< Whether to expect an burst ACK after every transmission
    uint32_t x_minCw;           ///< Minimal length of contention window
    uint32_t x_maxCw;           ///< Maximal length of contention window
    uint32_t x_slotTime;        ///< Slottime in microseconds
    uint32_t x_difs;            ///< DIFS in microseconds

    //Local variables
    bool waitForBurstAck_;        ///< True if previous frame was a data frame
    uint32_t currentCw_;        ///< Current backoff window in slottime
    boost::scoped_ptr<boost::thread> backoffThread_;
    boost::condition_variable backoffOverCond_;
    bool backoffOver_;
    boost::mutex mutex_;

    // member functions
    void backoffThreadFunction(void);
    bool getChannelState(uint32_t duration);
    void doubleCw(void);
    void resetCw(void);
};

} /* namespace iris */

#endif /* SOFTCSMACOMPONENT_H_ */
