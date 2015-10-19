/**
 * \file controllers/GenericDisplay/GenericDisplayController.cpp
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

#include <sstream>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "GenericDisplayController.h"

using namespace std;

namespace iris
{

//! Export library functions
IRIS_CONTROLLER_EXPORTS(GenericDisplayController);

GenericDisplayController::GenericDisplayController()
    : Controller("GenericDisplay", "A generic plotter", "Andre Puschmann", "0.1")
{
    registerParameter("spectrogramcomponent", "Name of realevent component",
                      "", false, realEventCompName_x);
    registerParameter("scattercomponent", "Name of scatter component",
                      "", false, scatterEventCompName_x);
    registerParameter("title", "Title of the plot",
                      "Plot", false, title_x);
    registerParameter("xaxisautoscale", "Whether X axis scales automatically",
                      "true", false, xAxisAutoScale_x);
    registerParameter("yaxisautoscale", "Whether Y axis scales automatically",
                      "true", false, yAxisAutoScale_x);
    registerParameter("xaxismin", "Minimum value of the X axis",
                      "", false, xAxisMin_x);
    registerParameter("xaxismax", "Maximum value of the X axis",
                      "", false, xAxisMax_x);
    registerParameter("yaxismin", "Minimum value of the Y axis",
                      "", false, yAxisMin_x);
    registerParameter("yaxismax", "Maximum value of the Y axis",
                      "", false, yAxisMax_x);
}

void GenericDisplayController::subscribeToEvents()
{
    subscribeToEvent("realevent", realEventCompName_x);
    subscribeToEvent("scatterevent", scatterEventCompName_x);
}

void GenericDisplayController::initialize()
{
    if (!realEventCompName_x.empty() && !scatterEventCompName_x.empty())
        throw IrisException("Each controller can only handle one event.");

    if (!realEventCompName_x.empty()) {
        realPlot_.reset(new Realplot());
        realPlot_->setTitle(title_x);
        realPlot_->setXAxisAutoScale(xAxisAutoScale_x);
        realPlot_->setYAxisAutoScale(yAxisAutoScale_x);
        realPlot_->setXAxisScale(xAxisMin_x, xAxisMax_x);
        realPlot_->setYAxisScale(-100, 0);
    }

    if (!scatterEventCompName_x.empty()) {
        scatterPlot_.reset(new Scatterplot());
        scatterPlot_->setTitle(title_x);
        scatterPlot_->setXAxisAutoScale(xAxisAutoScale_x);
        scatterPlot_->setYAxisAutoScale(yAxisAutoScale_x);
        scatterPlot_->setXAxisScale(xAxisMin_x, xAxisMax_x);
        scatterPlot_->setYAxisScale(yAxisMin_x, yAxisMax_x);
    }
}

void GenericDisplayController::processEvent(Event &e)
{
    if (e.eventName == "realevent" && realPlot_) {
        vector<float> data;
        for(int i=0;i<e.data.size();i++)
            data.push_back(boost::any_cast<float>(e.data[i]));
        realPlot_->setNewData(data.begin(), data.end());
    }

    if (e.eventName == "scatterevent" && scatterPlot_) {
        std::vector<std::complex<float> > data;
        for(int i = 0; i < e.data.size(); i++)
            data.push_back(boost::any_cast< std::complex<float> >(e.data[i]));
        scatterPlot_->setNewData(&data[0], data.size());
    }
}

void GenericDisplayController::destroy()
{}

} // namespace iris
