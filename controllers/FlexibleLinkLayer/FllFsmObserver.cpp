/**
 * \file controllers/FlexibleLinkLayer/FllFsmObserver.cpp
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

#include "FllFsmObserver.h"

using namespace std;

FllFsmObserver *FllFsmObserver::observer_ = NULL;

FllFsmObserver::FllFsmObserver()
: IObserver()
{
}

FllFsmObserver::~FllFsmObserver()
{
}

FllFsmObserver& FllFsmObserver::GetInstance()
{
    if(observer_ == NULL){
        observer_ = new FllFsmObserver();
    }
    return *observer_;
}

void FllFsmObserver::OnTransitionBegin(const char *pContextName,
                                        const char *pStatePrevious,
                                        const char *pStateNext,
                                        const char *pTransitionName)
{
    LOG(LDEBUG) << pContextName << ", from " << pStatePrevious << " to " << pStateNext << ", event " << pTransitionName;
}

void FllFsmObserver::OnTransitionEnd(const char *pContextName,
                                      const char *pStatePrevious,
                                      const char *pStateNext,
                                      const char *pTransitionName)
{
    LOG(LDEBUG) << pContextName << ", from " << pStatePrevious << " to " << pStateNext << ", event " << pTransitionName;
}

void FllFsmObserver::OnTimerStart(const char *pContextName, const char *pTimerName, long duration)
{
    LOG(LDEBUG) << pContextName << ", timer " << pTimerName << ", duration " << duration << " msec";
}

void FllFsmObserver::OnTimerStop(const char *pContextName, const char *pTimerName)
{
    LOG(LDEBUG) << pContextName << ", timer " << pTimerName;
}

void FllFsmObserver::OnEntry(const char *pContextName, const char *pStateName)
{
    LOG(LDEBUG) << pContextName << ", state " << pStateName;
}

void FllFsmObserver::OnExit(const char *pContextName, const char *pStateName)
{
    LOG(LDEBUG) << pContextName << ", state " << pStateName;
}

void  FllFsmObserver::OnProcessEventStart(const char *pContextName, size_t remainingEvent)
{
    LOG(LDEBUG) << pContextName << ", remaining event(s) " << remainingEvent;
}

void FllFsmObserver::OnProcessEventStop(const char *pContextName, size_t remainingEvent)
{
    LOG(LDEBUG) << pContextName << ", remaining event(s) " << remainingEvent;
}

std::string FllFsmObserver::getName() const
{
    return "FllFsmObserver";
}
