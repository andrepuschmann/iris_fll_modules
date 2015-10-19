/**
 * \file components/gpp/stack/Gdtp/GdtpComponent.h
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
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Wrapper for libgdtp.
 *
 */

#ifndef STACK_GDTPCOMPONENT_H_
#define STACK_GDTPCOMPONENT_H_

#include "irisapi/StackComponent.h"
#include "libgdtp.h"
#include <stdio.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>

using namespace libgdtp;

namespace iris
{
namespace stack
{

typedef std::map<std::string, FlowId> PortMap;

class GdtpComponent
        : public StackComponent
{
public:
    GdtpComponent(std::string name);
    virtual ~GdtpComponent();
    virtual void registerPorts();
    virtual void initialize();

    virtual void processMessageFromAbove(boost::shared_ptr<StackDataSet> set);
    virtual void processMessageFromBelow(boost::shared_ptr<StackDataSet> set);

    virtual void start();
    virtual void stop();

private:
    //Exposed parameters
    int numTopports_x;                  ///< Number of supported ports of this component.
    std::string scheduler_x;            ///< Default frame scheduler.
    uint64_t localAddress_x;            ///< Source address of this client (may get overwritten)
    uint64_t destinationAddress_x;      ///< Address of destination client
    bool isEthernetDevice_x;            ///< Whether to interpret incoming frames as Ethernet frames
    std::string ethernetDeviceName_x;   ///< Name of the Ethernet device to use (e.g. tap0)
    int ackTimeout_x;                   ///< Time to wait for ACK packets (ms)
    int maxRetry_x;                     ///< Number of retransmissions
    int maxSeqNo_x;                     ///< Maximum sequence number
    bool isReliable_x;                  ///< Whether error control is enabled or not
    int statusInterval_x;               ///< Interval in ms between status updates

    // local variables
    std::unique_ptr<libgdtp::Gdtp> gdtp_;
    PortMap gdtpPorts_;

    StackDataBuffer rxBuffer_;
    boost::ptr_vector<boost::thread> threads_;              ///< Component threads.

    // private functions
    void topPortHandler(std::string portName, FlowId id);
    void rxThreadFunction();
    void txThreadFunction();
    void statusThreadFunction();
};

} // namespace stack
} // namespace iris

#endif // STACK_GDTPCOMPONENT_H_
