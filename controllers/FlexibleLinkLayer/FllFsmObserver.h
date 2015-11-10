/**
 * \file controllers/FlexibleLinkLayer/FllFsmObserver.h
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
 * Helper class for FLL state machine debugging.
 */

#ifndef FLLFSMOBSERVER_H
#define FLLFSMOBSERVER_H

#include <irisapi/Logging.h>
#include <fsm/IObserver.h>

class FllFsmObserver : public fsm::IObserver
{
  public:
    static FllFsmObserver& GetInstance();
    ~FllFsmObserver();

    void OnTransitionBegin(const char *pContextName,
                          const char *pStatePrevious,
                          const char *pStateNext,
                          const char *pTransitionName);
    void OnTransitionEnd(const char *pContextName,
                         const char *pStatePrevious,
                         const char *pStateNext,
                         const char *pTransitionName);
    void OnTimerStart(const char *pContextName, const char *pTimerName, long duration);
    void OnTimerStop(const char *pContextName, const char *pTimerName);
    void OnEntry(const char *pContextName, const char *pStateName);
    void OnExit(const char *pContextName, const char *pStateName);
    void OnProcessEventStart(const char *pContextName, size_t remainingEvent);
    void OnProcessEventStop(const char *pContextName, size_t remainingEvent);

    std::string getName() const;

  protected:
    FllFsmObserver();
    // static class
    static FllFsmObserver *observer_;
};

#endif // FLLFSMOBSERVER_H
