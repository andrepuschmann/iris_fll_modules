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
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "GdtpComponent.h"

using namespace std;
using namespace libgdtp;

namespace iris
{
namespace stack
{

// export library symbols
IRIS_COMPONENT_EXPORTS(StackComponent, GdtpComponent);

GdtpComponent::GdtpComponent(std::string name)
    : StackComponent(name,
                     "gdtpstackcomponent",
                     "Wrapper for libgdtp.",
                     "Andre Puschmann",
                     "0.1")
    ,rxBuffer_(20)
{
    //Format: registerParameter(name, description, default, dynamic?, parameter, allowed values);
    registerParameter("scheduler", "Default frame scheduler", "fifo", false, scheduler_x);
    registerParameter("numtopports", "Number of supported ports", "1", false, numTopports_x);
    registerParameter("localaddress", "Address of this client", "f009e090e90e", false, localAddress_x);
    registerParameter("destinationaddress", "Address of the destination client", "00f0f0f0f0f0", false, destinationAddress_x);
    registerParameter("isethdevice", "Whether to act as an Ethernet device", "false", false, isEthernetDevice_x);
    registerParameter("ethdevicename", "Name of the Ethernet device", "tap0", false, ethernetDeviceName_x);
    registerParameter("acktimeout", "Time to wait for ACK packets in ms", "100", false, ackTimeout_x);
    registerParameter("maxretry", "Number of retransmissions", "100", false, maxRetry_x);
    registerParameter("maxseqno", "Maximum sequence number", "127", false, maxSeqNo_x);
    registerParameter("isreliable", "Whether error control is enabled or not", "true", false, isReliable_x);
    registerParameter("passthrough", "Whether to pass data through component", "true", false, passthrough_x);
    registerParameter("statusinterval", "Interval in ms between status updates (0 for no updates)", "0", false, statusInterval_x);

    registerEvent("ferevent", "An event providing the current frame error rate [0:1]", TypeInfo< float >::identifier);
}


GdtpComponent::~GdtpComponent()
{
}


void GdtpComponent::registerPorts()
{
    std::vector<int> types;
    types.push_back( int(TypeInfo< uint8_t >::identifier) );

    for (uint32_t i = 1; i <= numTopports_x; i++) {
        string portName = "topport" + boost::lexical_cast<std::string>(i);
        registerInputPort(portName, types);
    }
    registerInputPort("bottomport1", types);
}


void GdtpComponent::initialize()
{
    FlowProperties props((isReliable_x == true ? RELIABLE : UNRELIABLE),
                       99,
                       (isEthernetDevice_x == true ? IMPLICIT : EXPLICIT),
                        maxSeqNo_x,
                        ackTimeout_x,
                        maxRetry_x,
                        ethernetDeviceName_x);

    // configure and init gdtp instance
    gdtp_ = std::unique_ptr<libgdtp::Gdtp>(new libgdtp::Gdtp());
    gdtp_->set_default_source_address(localAddress_x);
    gdtp_->set_default_destination_address(destinationAddress_x);
    gdtp_->initialize();

    // register libgdtp flow objects with default values
    for (uint32_t i = 1; i <= numTopports_x; i++) {
        string portName = "topport" + boost::lexical_cast<std::string>(i);
        props.set_priority(99 - i); // decrease priority of each newly allocated flow
        FlowId id = gdtp_->allocate_flow(i, props);
        gdtpPorts_[portName] = id;
        // start handler for incoming traffic on this port
        threads_.push_back(new boost::thread(boost::bind(&GdtpComponent::topPortHandler, this, portName, i)));
    }
}


void GdtpComponent::processMessageFromAbove(boost::shared_ptr<StackDataSet> incomingFrame)
{
    //StackHelper::printDataset(incomingFrame, "vanilla from above");

    // lookup libgdtp flow ID
    FlowId id;
    try {
        id = gdtpPorts_.at(incomingFrame->destPortName);
    }
    catch (std::out_of_range& e) {
        LOG(LERROR) << e.what();
    }

    if (passthrough_x) {
        // convert incoming frame and pass it to libgdtp
        std::shared_ptr<Data> sdu = std::make_shared<Data>(incomingFrame->data.begin(), incomingFrame->data.end());
        gdtp_->handle_data_from_above(sdu, id);
    }
}


void GdtpComponent::processMessageFromBelow(boost::shared_ptr<StackDataSet> incomingFrame)
{
    //StackHelper::printDataset(incomingFrame, "vanilla from below");
    rxBuffer_.pushDataSet(incomingFrame);
}


void GdtpComponent::start()
{
    threads_.push_back(new boost::thread(boost::bind(&GdtpComponent::rxThreadFunction, this)));
    threads_.push_back(new boost::thread(boost::bind(&GdtpComponent::txThreadFunction, this)));
    if (statusInterval_x) {
        threads_.push_back(new boost::thread(boost::bind(&GdtpComponent::statusThreadFunction, this)));
    }
}

void GdtpComponent::stop()
{
    // stop threads
    boost::ptr_vector<boost::thread>::iterator it;
    for (it = threads_.begin(); it != threads_.end(); ++it) {
        it->interrupt();
        it->join();
    }
}


void GdtpComponent::topPortHandler(std::string portName, FlowId id)
{
    LOG(LINFO) << "Handler for " << portName << " started.";

    try {
        while (true) {
            std::shared_ptr<Data> data = gdtp_->get_data_for_above(id);
            boost::shared_ptr<StackDataSet> pdu(boost::shared_ptr<StackDataSet>(new StackDataSet));
            pdu->data.insert(pdu->data.begin(), data->begin(), data->end());
            sendUpwards(portName, pdu);
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in Gdtp component: " << ex.what() << " - topPortHandler thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "TopPortHandler for " << portName << " interrupted (id: " << boost::this_thread::get_id() << ").";
    }
}


void GdtpComponent::rxThreadFunction()
{
    LOG(LINFO) << "Rx thread started.";
    try {
        while (true) {
            boost::this_thread::interruption_point();

            // retrieve next PDU from buffer
            boost::shared_ptr<StackDataSet> pdu = rxBuffer_.popDataSet();

            // initialize libgdtp type with data stored inside the stackdataset
            Data frame(pdu->data.begin(), pdu->data.end());
            gdtp_->handle_data_from_below(DEFAULT_BELOW_PORT_ID, frame);
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in Gdtp component: " << ex.what() << " - Rx thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Rx thread interrupted (id: " << boost::this_thread::get_id() << ").";
    }
}


void GdtpComponent::txThreadFunction()
{
    LOG(LINFO) << "Tx thread started.";

    try {
        Data frame;

        while (true) {
            boost::this_thread::interruption_point();

            // this call may block if no frames are present
            gdtp_->get_data_for_below(DEFAULT_BELOW_PORT_ID, frame);

            // FIXME: too much copying involved
            boost::shared_ptr<StackDataSet> irisFrame(boost::shared_ptr<StackDataSet>(new StackDataSet));
            irisFrame->data.insert(irisFrame->data.begin(), frame.begin(), frame.end());
            irisFrame->metadata.setMetadata("isdata", true);

            // send downwards
            //sendDownwards("belowport1", pdu);
            sendDownwards(irisFrame);

            // FIXME: wait for Txover event
            gdtp_->set_data_transmitted(DEFAULT_BELOW_PORT_ID);
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in Gdtp component: " << ex.what() << " - Tx thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Tx thread interrupted (id: " << boost::this_thread::get_id() << ").";
    }
}


void GdtpComponent::statusThreadFunction()
{
    LOG(LINFO) << "Status thread started.";

    try {
        while(true) {
            boost::this_thread::interruption_point();
            // FIXME: remove dummy value
            float fer = 0.1;
            activateEvent("ferevent", fer);
            boost::this_thread::sleep(boost::posix_time::milliseconds(statusInterval_x));
        }
    }
    catch(IrisException& ex)
    {
        LOG(LFATAL) << "Error in Gdtp component: " << ex.what() << " - Status thread exiting.";
    }
    catch(boost::thread_interrupted)
    {
        LOG(LINFO) << "Status thread interrupted (id: " << boost::this_thread::get_id() << ").";
    }
}


} // namespace stack
} // namespace iris
