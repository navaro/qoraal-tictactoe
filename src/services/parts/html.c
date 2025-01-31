/*
    Copyright (C) 2015-2025, Navaro, All Rights Reserved
    SPDX-License-Identifier: MIT

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */


#include <stdio.h>
#include "qoraal-engine/parts/parts.h"
#include "qoraal-engine/engine.h"
#include "html.h"

static HTML_EMIT_T *        _html_emit  = 0 ;
static p_mutex_t            _html_mutex = 0 ;
uint32_t                    _html_event_mask = 0 ;

/*===========================================================================*/
/* part local functions.                                                */
/*===========================================================================*/
static int32_t      part_html_cmd (PENGINE_T instance, uint32_t start) ;
static int32_t      action_html_emit (PENGINE_T instance, uint32_t parm, uint32_t flags) ;
static int32_t      action_html_ready (PENGINE_T instance, uint32_t parm, uint32_t flags) ;



/**
 * @brief   Initializes actions for part
 *
 */
ENGINE_ACTION_IMPL  (html_emit,        "Emits html text.") ;
ENGINE_ACTION_IMPL  (html_ready,        "Completes html text / ready to render.") ;

/**
 * @brief   Initialises events for part
 *
 */
ENGINE_EVENT_IMPL   ( _html_render,     "Render content with html_emit and finally html_done.") ;

/**
 * @brief   Initialises constants for part
 *
 */
ENGINE_CONST_IMPL(1, START,                  "Start");
ENGINE_CONST_IMPL(0, STOP,                   "Stop");


/**
 * @brief   part declaration.
 *
 */
ENGINE_CMD_FP_IMPL (part_html_cmd) ;

bool        
html_emit_ready (void)
{
    return _html_event_mask != 0 ;
}

int32_t 
html_emit_wait (HTML_EMIT_T* emit, HTML_EMIT_CB cb, void * ctx, uint32_t timeout)
{
    if (!_html_event_mask) return E_UNEXP ;

    if (os_sem_create (&emit->complete, 0) != EOK) {
        return EFAIL ;
    }
    emit->cb = cb ;
    emit->ctx = ctx ;
    _html_emit = emit ;

   os_mutex_lock (&_html_mutex) ;
    if (engine_queue_masked_event (_html_event_mask, ENGINE_EVENT_ID_GET(_html_render), 0) != EOK) {
        timeout = 0 ;

    } else {
         _html_event_mask = 0 ;

    }
    os_mutex_unlock (&_html_mutex) ;

    int32_t res = os_sem_wait_timeout (&emit->complete, OS_MS2TICKS(timeout)) ;

    os_mutex_lock (&_html_mutex) ;
    if (_html_emit) {
        os_sem_delete (&emit->complete) ;
        _html_emit = 0 ;
    }
    os_mutex_unlock (&_html_mutex) ;
    return res ;
}



/**
 * @brief   part_toaster_cmd
 * @param[in] instance      Statemachine instance.
 * @param[in] start         Start/stop.
 */
int32_t
part_html_cmd (PENGINE_T instance, uint32_t start)
{
    if (!instance) {
        if (start == PART_CMD_PARM_START) {
            return os_mutex_create (&_html_mutex) ;
        } else {        
            os_mutex_delete (&_html_mutex) ;
        }
    }   
    return ENGINE_OK ;
}

/**
 * @brief   emmits html text
 * @param[in] instance      engine instance.
 * @param[in] parm          parameter.
 * @param[in] flags         validate and parameter type flag.
 */
int32_t
action_html_emit (PENGINE_T instance, uint32_t parm, uint32_t flags)
{
    if (flags & (PART_ACTION_FLAG_VALIDATE)) {
        return parts_valadate_string (instance, parm, flags) ;

    }
    uint16_t len ;
    const char* str = 0 ;
    if (flags & PART_ACTION_FLAG_STRING) {
        str = engine_get_string (instance, parm, &len) ;
    } 

    os_mutex_lock (&_html_mutex) ;
    if (str && _html_emit && _html_emit->cb) {
        _html_emit->cb (_html_emit->ctx, str, len) ;
    }
    os_mutex_unlock (&_html_mutex) ;

    return ENGINE_OK ;
}

/**
 * @brief   emmits html text
 * @param[in] instance      engine instance.
 * @param[in] parm          parameter.
 * @param[in] flags         validate and parameter type flag.
 */
int32_t
action_html_ready (PENGINE_T instance, uint32_t parm, uint32_t flags)
{
    if (flags & (PART_ACTION_FLAG_VALIDATE)) {
        return EOK ;

    }

    os_mutex_lock (&_html_mutex) ;
    _html_event_mask |= engine_get_mask (instance) ;
    if (_html_emit && _html_emit->complete) {
       os_sem_signal (&_html_emit->complete) ;
    }
    os_mutex_unlock (&_html_mutex) ;

    return ENGINE_OK ;
}
