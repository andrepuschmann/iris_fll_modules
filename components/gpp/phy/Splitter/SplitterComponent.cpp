/**
 * \file components/gpp/phy/Splitter/SplitterComponent.cpp
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

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"
#include "irisapi/TypeVectors.h"
#include "SplitterComponent.h"

using namespace std;

namespace iris
{

namespace phy
{
// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, SplitterComponent);

template <class T>
inline std::string toString(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}


SplitterComponent::SplitterComponent(string name):
    PhyComponent(name, "splitterphycomponent", "Splits data into two or more outputs", "Paul Sutton", "0.1")
{
    //Register all parameters
    registerParameter("numoutputs", "Number of outputs to split across",
            "2", false, numOutputs_x, Interval<uint32_t>(1, 10));

    registerParameter("activeports", "Comma separated list of active ports",
            "all", false, activePorts_x);
}


void SplitterComponent::registerPorts()
{
    //This component supports all types
    vector<int> validTypes = convertToTypeIdVector<IrisDataTypes>();

    registerInputPort("input1", validTypes);
    for (uint32_t i = 1; i <= numOutputs_x; i++) {
        string portName = "output" + toString(i);
        registerOutputPort(portName, validTypes);
    }
}


void SplitterComponent::calculateOutputTypes(
        std::map<std::string,int>& inputTypes,
        std::map<std::string,int>& outputTypes)
{
    for (uint32_t i = 1; i <= numOutputs_x; i++) {
        string portName = "output" + toString(i);
        outputTypes[portName] = inputTypes["input1"];
    }
}


void SplitterComponent::initialize()
{
    updateActivePorts();
}


void SplitterComponent::parameterHasChanged(std::string name)
{
    updateActivePorts();
}


void SplitterComponent::process()
{
    if (outputBuffers.size() != numOutputs_x || inputBuffers.size() != 1) {
        //Need to throw an exception here
    }

    switch (inputBuffers[0]->getTypeIdentifier()) {
    case 0:
        writeOutput<uint8_t>();
        break;
    case 1:
        writeOutput<uint16_t>();
        break;
    case 2:
        writeOutput<uint32_t>();
        break;
    case 3:
        writeOutput<uint64_t>();
        break;
    case 4:
        writeOutput<int8_t>();
        break;
    case 5:
        writeOutput<int16_t>();
        break;
    case 6:
        writeOutput<int32_t>();
        break;
    case 7:
        writeOutput<int64_t>();
        break;
    case 8:
        writeOutput<float>();
        break;
    case 9:
        writeOutput<double>();
        break;
    case 10:
        writeOutput<long double>();
        break;
    case 11:
        writeOutput< complex<float> >();
        break;
    case 12:
        writeOutput< complex<double> >();
        break;
    case 13:
        writeOutput<complex<long double> >();
        break;
    default:
        break;
    }
}


template<typename T>
void SplitterComponent::writeOutput()
{
    ReadBuffer<T>* inBuf = castToType<T>(inputBuffers[0]);

    DataSet<T>* readDataSet = NULL;
    DataSet<T>* writeDataSet = NULL;

    inBuf->getReadData(readDataSet);
    size_t size = readDataSet->data.size();

    // only send data on active ports
    std::vector<int>::iterator it;
    for (it = activePortsVector_.begin(); it != activePortsVector_.end(); ++it) {
        WriteBuffer< T >* outBuf = castToType<T>(outputBuffers[*it]);
        outBuf->getWriteData(writeDataSet, size);
        for (uint32_t i=0; i < size; i++) {
            writeDataSet->data[i] = readDataSet->data[i];
        }
        outBuf->releaseWriteData(writeDataSet);
    }

    inBuf->releaseReadData(readDataSet);
}


void SplitterComponent::updateActivePorts()
{
    activePortsVector_.clear();
    if (activePorts_x == "all") {
        // enable all available ports
        for (uint32_t i = 0; i < numOutputs_x; i++) {
            activePortsVector_.push_back(i);
        }
    } else {
        // enable ports given in variable, tokenize first
        istringstream ss(activePorts_x);
        while (ss) {
            string s;
            if (!getline(ss, s, ','))
                break;
            // try to extract port number from string
            std::size_t pos = s.find_first_of("0123456789");
            if (pos != std::string::npos) {
                int id = boost::lexical_cast<int>(s[pos]);
                id--; // internally, zero-based counting
                if (id < numOutputs_x) {
                    activePortsVector_.push_back(id);
                }
            }
        }
    }
}

} /* namespace phy */

} /* namespace iris */
