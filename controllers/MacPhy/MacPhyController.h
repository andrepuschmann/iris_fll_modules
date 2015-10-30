/**
 * \file controllers/MacPhy/MacPhyController.h
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
 * A controller that coordinates interaction among various
 * MAC and PHY layer components.
 */

#ifndef MACPHYCONTROLLER_H_
#define MACPHYCONTROLLER_H_

#include "irisapi/Controller.h"

namespace iris
{

class MacPhyController: public Controller
{
private:
    // exposed parameters
    std::string x_engineSensingModule;
    std::string x_componentSensingModule;
    std::string x_engineMacModule;
    std::string x_componentMacModule;
    std::string x_componentTxModule;
    float ccaThreshold_x;
    bool x_debug;
    
    // local variables
    boost::mutex mutex_;
    
    // private member functions
    void processCcaRequestEvent(Event &e);
    void processRssiResult(Event &e);
    void triggerCca(const std::string component, const std::string engine, const uint32_t duration);

public:
    MacPhyController();
    virtual void subscribeToEvents();
    virtual void initialize();
    virtual void processEvent(Event &e);
    virtual void destroy();
};

} /* namespace iris */

#endif /* MACPHYCONTROLLER_H_ */
