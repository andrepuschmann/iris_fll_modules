/**
 * \file lib/generic/utility/FllStackComponent.h
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
 * This Class provides common methods to interact with the FLL controller.
 *
 */

#ifndef FLLSTACKCOMPONENT_H
#define FLLSTACKCOMPONENT_H

#include "irisapi/StackComponent.h"
#include "FllChannelManager.h"


namespace iris
{

class FllStackComponent : public StackComponent
{
public:

    FllStackComponent(std::string name, std::string type, std::string description, std::string author, std::string version )
        :StackComponent(name, type, description, author, version)
    {}

    /// Destructor
    virtual ~FllStackComponent() {}

    /**
     * \brief Get set of available channels.
     *
     * \param none.
     * \return FllChannelVector containing channels.
     */
    FllChannelVector getAvailableChannels(void)
    {
        int32_t dummy = 1;
        activateEvent("EvGetAvailableChannelsRequest", dummy);
        Command command = waitForCommand("EvGetAvailableChannelsResult");
        assert(command.data.size() == 1);
        return boost::any_cast<FllChannelVector>(command.data[0]);
    }


    FllChannel getOperatingChannel(const int32_t trx)
    {
        activateEvent("EvGetOperatingChannelRequest", trx);
        Command command = waitForCommand("EvGetOperatingChannelResult");
        assert(command.data.size() == 1);
        return boost::any_cast<FllChannel>(command.data[0]);
    }

    void setBackupChannel(FllChannel channel)
    {
        activateEvent("EvUpdateBackupChannel", channel);
    }
};

} // end of iris namespace

#endif // FLLSTACKCOMPONENT_H
