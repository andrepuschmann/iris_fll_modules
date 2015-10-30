/**
 * \file lib/generic/utility/RandomGenerator.h
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
 * This Class provides helper methods for generating random numbers.
 *
 */

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <stdio.h>
#include <time.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

namespace iris
{

class RandomGenerator
{
private:
    // private constructor
    RandomGenerator() {
        generator.seed(static_cast<unsigned int>(std::time(0)));
    }
    RandomGenerator(const RandomGenerator&) {}  // private copy constructor
    RandomGenerator& operator=(RandomGenerator const&) {} // private assignment operator
    ~RandomGenerator() {}
    static RandomGenerator* instance;
    boost::mt19937 generator;

public:
    static RandomGenerator& getInstance()
    {
        static RandomGenerator _instance;
        return _instance;
    }

    static void destroy() {
        if (instance)
            delete instance;
        instance = 0;
    }

    uint32_t Uniform0toN(uint32_t n) {
        boost::uniform_int<> window(0, n);
        boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(generator, window);
        return die();
    }
};

} // end of iris namespace
#endif // RANDOMGENERATOR_H
