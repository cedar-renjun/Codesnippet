//******************************************************************************************
//!
//! \file   SoftTimer.c
//! \brief  SoftTimer Implement File
//!         This module is used to replace 'delay' function in state machine
//! \author cedar
//! \date   2014-9-17
//! \email  xuesong5825718@gmail.com
//! \qq     819280802
//!
//! \license
//!
//! Copyright (c) 2014 Cedar MIT License
//!
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights to
//! use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//! the Software, and to permit persons to whom the Software is furnished to do so,
//! subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in all
//! copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//! IN THE SOFTWARE.
///
//******************************************************************************************

#include "SoftTimer.h"


//******************************************************************************************
//!                     Typedef
//******************************************************************************************
typedef struct
{
    uint16_t handle;                                       // Timer Resource Handle
    uint32_t delay;                                        // Delay Value
    uint32_t timestamp;                                    // Timer Stamp
}TimerElem_t;


//******************************************************************************************
//!                     Private Variable
//******************************************************************************************

static volatile uint32_t TimerCnt    = 0;
static volatile uint16_t HandleIndex = 0;
static TimerElem_t       SoftTimer[TIMER_ELEMENT_NUM_MAX];


//******************************************************************************************
//!                     Function Implement
//******************************************************************************************


//******************************************************************************************
//
//! \brief  Initialize Timer resource
//!
//! \param  None.
//! \retval 
//!         - SOFT_TIMER_SUCCESS
//!
//! \note
//!         - This function must be called first !.
//!         
//
//******************************************************************************************
uint16_t SoftTimer_Init(void)
{
    uint8_t i = 0;

    // Clear All Elements
    for(i = 0; i < TIMER_ELEMENT_NUM_MAX; i++)
    {
        SoftTimer[i].handle    = 0;
        SoftTimer[i].delay     = 0;
        SoftTimer[i].timestamp = 0;
    }

    // Initialize Global Status Variable
    TimerCnt    = 0;
    HandleIndex = 1;

    return (SOFT_TIMER_SUCCESS);
}

//******************************************************************************************
//
//! \brief  Request Timer resource
//!
//! \param  [in] Tick is time eclipse value.
//! \retval 
//!         - Zero                     Operate Failure, No Timer Available
//!         - Non-Zero                 Valid Timer Handle
//!
//! \note
//!         - Timer handle only can be used once.
//!         
//
//******************************************************************************************
uint16_t SoftTimer_Req(uint32_t Tick)
{
    uint8_t i = 0;

    for (i = 0; i < TIMER_ELEMENT_NUM_MAX; i++)
    {
        if (SoftTimer[i].handle == 0)
        {

            ENTER_SECURE_SECTION();

            // Timer Handle != 0
            if (++HandleIndex == 0)
            {
                ++HandleIndex;
            }

            SoftTimer[i].handle    = HandleIndex;
            SoftTimer[i].delay     = Tick;
            SoftTimer[i].timestamp = Tick + TimerCnt;

            EXIT_SECURE_SECTION();

            return (HandleIndex);
        }
    }

    return (0);
}

//******************************************************************************************
//
//! \brief  Check Timer status
//!         You can check register timer status at any time.
//!
//! \param  [in] Handle is Timer Handle, which you can get it from \ref SoftTimer_Req.
//! \retval 
//!         - \ref SOFT_TIMER_ING      Timer Counting
//!         - \ref SOFT_TIMER_TIMEOUT  Timer TimeOut
//!         - \ref SOFT_TIMER_ERR      Invalid Timer Handle
//!
//! \note
//!         - You must call \ref SoftTimer_Req to request an valid timer handle.
//!         - Timer handle only can be used once.
//!         
//
//******************************************************************************************
uint16_t SoftTimer_Check(uint16_t Handle)
{
    uint8_t i = 0;

    for(i = 0; i < TIMER_ELEMENT_NUM_MAX; i++)
    {
        if(SoftTimer[i].handle ==  Handle)
        {
            ENTER_SECURE_SECTION();
            if(SoftTimer[i].delay + TimerCnt < SoftTimer[i].timestamp)
            {
                return (SOFT_TIMER_ING);
            }
            else
            {
                return (SOFT_TIMER_TIMEOUT);
            }
            EXIT_SECURE_SECTION();
        }
    }

    return (SOFT_TIMER_ERR);
}


//******************************************************************************************
//
//! \brief  SoftTimer Hook Function
//!         This callback function must be called interval
//!
//! \note   Typical 1ms interval
//
//******************************************************************************************
void TimerISR_Hook(void)
{
    TimerCnt++;
}
