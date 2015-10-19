/**
 * \file controllers/GenericDisplay/GenericDisplayController.h
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
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * A display controller using the qt scatter- and realplot widget.
 */

#ifndef CONTROLLERS_GENERICDISPLAYCONTROLLER_H_
#define CONTROLLERS_GENERICDISPLAYCONTROLLER_H_

#include "irisapi/Controller.h"
#include <boost/scoped_ptr.hpp>
#include "graphics/qt/realplot/Realplot.h"
#include "graphics/qt/scatterplot/Scatterplot.h"

namespace iris
{

class GenericDisplayController
        : public Controller
{
public:
    GenericDisplayController();
    virtual void subscribeToEvents();
    virtual void initialize();
    virtual void processEvent(Event &e);
    virtual void destroy();

private:
    std::string realEventCompName_x;
    std::string scatterEventCompName_x;
    std::string title_x;
    bool xAxisAutoScale_x;
    bool yAxisAutoScale_x;
    double xAxisMin_x;
    double xAxisMax_x;
    double yAxisMin_x;
    double yAxisMax_x;

    boost::scoped_ptr<Realplot> realPlot_;
    boost::scoped_ptr<Scatterplot> scatterPlot_;
};

} // namespace iris

#endif // CONTROLLERS_GENERICDISPLAYCONTROLLER_H_
