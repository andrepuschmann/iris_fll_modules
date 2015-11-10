/**
 * \file controllers/FlexibleLinkLayer/FllException.h
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
 * Flexible Link Layer specific exception.
 */

#ifndef FLLEXCEPTION_H
#define FLLEXCEPTION_H

//! Exception which can be thrown by FLL state machines
class FllException : public std::exception
{
private:
    std::string d_message;
public:
    FllException(const std::string &message) throw()
        :exception(), d_message(message)
    {};
    virtual const char* what() const throw()
    {
        return d_message.c_str();
    };
    virtual ~FllException() throw()
    {};
};

#endif // FLLEXCEPTION_H
