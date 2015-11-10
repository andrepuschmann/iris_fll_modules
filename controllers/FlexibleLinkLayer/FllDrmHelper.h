/**
 * \file controllers/FlexibleLinkLayer/FllDrmHelper.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2014 The Iris Project Developers. See the
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
 * Small helper class to convert DRM channel specifications
 * into the FLL format and back.
 */


#ifndef FLLDRMHELPER_H
#define FLLDRMHELPER_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include "sclhelper.hpp"
#include "drm.pb.h"
#include "FllChannel.h"

namespace iris
{

//! Exception which can be thrown by DRM helper class
class FllDrmHelperException : public std::exception
{
private:
    std::string d_message;
public:
    FllDrmHelperException(const std::string &message) throw()
        :exception(), d_message(message)
    {};
    virtual const char* what() const throw()
    {
        return d_message.c_str();
    };
    virtual ~FllDrmHelperException() throw()
    {};
};

class FllDrmHelper
{
    public:
        static FllChannelVector getChannelVector(void)
        {
            GateFactory &myFactory = GateFactory::getInstance();
            sclGate *controlGate = myFactory.createGate("drmclient", "control");

            scl_drm::control request, reply;

            // create control packet to request channel specification
            //cout << "Send GET_CHANNEL_LIST request" << endl;
            request.set_type(scl_drm::REQUEST);
            request.set_command(scl_drm::GET_CHANNEL_LIST);
            controlGate->sendProto(request);

            // wait for answer from DRM
            //cout << "Wait for reply .." << endl;
            controlGate->recvProto(reply);
            assert(reply.type() == scl_drm::REPLY);
            //cout << "Got reply:" << endl;
            //cout << "Number of channels is: " << reply.channelmap_size() << endl;

            // convert DRM reply into FLL type
            FllChannelVector vector;
            for (int i = 0; i < reply.channelmap_size(); i++) {
                vector.push_back(drmToFllSpec(reply.channelmap(i)));
            }
            return vector;
        }

        static bool getChannel(FllChannel &channel, const scl_drm::channelProp property = scl_drm::LEAST_USED)
        {
            GateFactory &myFactory = GateFactory::getInstance();
            sclGate *controlGate = myFactory.createGate("drmclient", "control");

            scl_drm::control request, reply;

            // create control packet to request channel specification
            //cout << "Send GET_SINGLE_CHANNEL request" << endl;
            request.set_type(scl_drm::REQUEST);
            request.set_command(scl_drm::GET_SINGLE_CHANNEL);
            request.set_prop(property);
            controlGate->sendProto(request);

            // wait for answer from DRM
            //cout << "Wait for reply .." << endl;
            controlGate->recvProto(reply);
            assert(reply.type() == scl_drm::REPLY);
            //cout << "Got reply: len is " << reply.channelmap_size() << endl;
            if (reply.channelmap_size() != 1)
                return false;

            // set channel accordingly
            channel = drmToFllSpec(reply.channelmap(0));
            return true;
        }

        static void setChannelState(const FllChannel channel, const uint32_t id)
        {
            GateFactory &myFactory = GateFactory::getInstance();
            sclGate *dataGate = myFactory.createGate("drmclient", "data");
            scl_drm::statusUpdate packet;
            packet.set_channel_id(id + 1); // within the DRM, the id starts with 1
            packet.set_timestamp(getTimeSinceEpoch());
            packet.set_status(toDrmState(channel.getState()));
            dataGate->sendProto(packet);
            //cout << "Update channel state packet sent to DRM." << endl;
        }

        static scl_drm::statusUpdate_statusType toDrmState(const FllChannelState state)
        {
            switch (state) {
            case FREE: return scl_drm::statusUpdate_statusType_FREE; break;
            case BUSY_SU: return scl_drm::statusUpdate_statusType_BUSY_SU; break;
            case BUSY_PU: return scl_drm::statusUpdate_statusType_BUSY_PU; break;
            default: return scl_drm::statusUpdate_statusType_BUSY_PU;
            }
        }


        static FllChannel drmToFllSpec(const scl_drm::channelSpec from)
        {
            FllChannel to(from.f_center(), from.bandwidth());
            return to;
        }

        static uint64_t getTimeSinceEpoch()
        {
            boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
            boost::posix_time::ptime other = boost::posix_time::microsec_clock::local_time();
            return (other-epoch).total_milliseconds();
        }
};

} // namespace iris

#endif // FLLDRMHELPER_H
