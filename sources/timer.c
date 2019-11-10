/*
 * Copyright (c) 2019, University of Kaiserslautern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Andre Lucas Chinazzo
 */


#include "../headers/timer.h"

bool isValidTimer(TIMER* thisTimer)
{
  return (thisTimer->initializedFlag == 'y');
}

void startTimer(TIMER* thisTimer)
{
  clock_gettime(CLOCK_MONOTONIC, &thisTimer->begin);
  if ( !isValidTimer(thisTimer) ) {
    thisTimer->elapsedTime = 0;
    thisTimer->totalElapsedTime = 0;
    thisTimer->initializedFlag = 'y'; // Must agree with isValidTimer()
  }
}

void clearTimer(TIMER* thisTimer)
{
  thisTimer->elapsedTime = 0;
  thisTimer->totalElapsedTime = 0;
  thisTimer->initializedFlag = 'n'; // Must (dis)agree with isValidTimer()
}

bool stopTimer(TIMER* thisTimer)
{
  if ( !isValidTimer(thisTimer) ) {
    STDERR_INFO("Unexpected behavior - tried to use an uninitialized timer!");
    RETURN_ERROR;
  }
  clock_gettime(CLOCK_MONOTONIC, &thisTimer->end);
  return SUCCESS;
}

double getElapsedTime(TIMER* thisTimer)
{
  if ( !isValidTimer(thisTimer) ) {
    STDERR_INFO("Unexpected behavior - tried to use an uninitialized timer!");
    RETURN_ERROR_V(-1);
  }
  stopTimer(thisTimer);
  thisTimer->elapsedTime = (double)(thisTimer->end.tv_sec
                                      - thisTimer->begin.tv_sec);
  thisTimer->elapsedTime += (double)(thisTimer->end.tv_nsec
                                      - thisTimer->begin.tv_nsec) / 1e9;
  return thisTimer->elapsedTime;
}

bool accElapsedTime(TIMER* thisTimer)
{
  if ( !isValidTimer(thisTimer) ) {
    STDERR_INFO("Unexpected behavior - tried to use an uninitialized timer!");
    RETURN_ERROR;
  }
  thisTimer->totalElapsedTime += getElapsedTime(thisTimer);
  return SUCCESS;
}

double getTotalElapsedTime(TIMER* thisTimer)
{
  if ( !isValidTimer(thisTimer) ) {
    STDERR_INFO("Unexpected behavior - tried to use an uninitialized timer!");
    RETURN_ERROR_V(-1);
  }
  return thisTimer->totalElapsedTime;
}
