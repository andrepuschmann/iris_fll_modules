/**
 * \file components/gpp/phy/Splitter/SplitterComponent.h
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
 * Implementation of an a splitter which maps one input to one
 * or more outputs.
 */

#ifndef SPLITTERCOMPONENT_H_
#define SPLITTERCOMPONENT_H_

#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

class SplitterComponent: public PhyComponent
{
private:
    //! The number of outputs required
    uint32_t numOutputs_x;

    //! A template function used to write input data to the output
    template<typename T> void writeOutput();
public:
    SplitterComponent(std::string name);
    virtual void calculateOutputTypes(
            std::map<std::string, int>& inputTypes,
            std::map<std::string, int>& outputTypes);

    virtual void registerPorts();
    virtual void initialize();
    virtual void process();
};

} // namespace phy
} // namespace iris

#endif /* SPLITTERCOMPONENT_H_ */
