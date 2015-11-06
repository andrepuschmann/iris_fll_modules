/**
 * \file components/gpp/phy/MetadataDetecto/MetadataDetectorComponent.h
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
 * A simple component that tries to find a certain pattern inside
 * the metadata passed through it. If the pattern has been found,
 * a PU detection event is triggerd.
 *
 */

#ifndef METADATADETECTORCOMPONENT_H_
#define METADATADETECTORCOMPONENT_H_

#include <irisapi/PhyComponent.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>

namespace iris
{
class MetadataDetectorComponent: public PhyComponent
{
private:
    // exposed parameters
    std::string key_x;
    std::string value_x;
    int32_t interval_x;
    std::string eventName_;
    bool hasEvent_x;
    int32_t manualDetection_;
    bool debug_x;

    // local variables
    boost::scoped_ptr<boost::thread> commandThread_;   ///< Thread handle
    boost::system_time nextRefresh_;
    boost::posix_time::time_duration intervalIncrement_;
    boost::mutex mutex_;
    bool enableSensing_;

    // local members
    void commandHandler(void);

public:
    MetadataDetectorComponent(std::string name);
    virtual ~MetadataDetectorComponent();
    virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
    virtual void registerPorts();
    virtual void initialize();
    virtual void process();
    virtual void parameterHasChanged(std::string name);
};

} /* namespace iris */

#endif /* METADATADETECTORCOMPONENT_H_*/
