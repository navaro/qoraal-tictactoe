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
#include <string.h>
#include "qoraal/qoraal.h"
#include "qoraal-http/wserver.h"
#include "../../services.h"
#include "../../parts/html.h"

static HTML_EMIT_T  _wengine_emit ;


static void 
html_emit_cb (void* ctx, const char* html, uint32_t len)
{
    HTTP_USER_T *user = (HTTP_USER_T *) ctx ;
    if (user && html && len) {
        httpserver_chunked_append_str (user, html, len) ;

    }
    
}

const char*
wengine_ctrl (HTTP_USER_T *user, uint32_t method, char* endpoint, uint32_t type)
{
    if (type == WSERVER_CTRL_METADATA_HEADING) {
        return "Engine" ;
    }
    else if (type == WSERVER_CTRL_START){
        html_emit_create (&_wengine_emit) ;
    }
    else if (type == WSERVER_CTRL_STOP){
        html_emit_delete (&_wengine_emit) ;
    }
    
    return 0 ;
}


/**
 * @brief   Handler 
 *
 * @app
 */
int32_t
wengine_handler (HTTP_USER_T *user, uint32_t method, char* endpoint)
{
    int32_t res = HTTP_SERVER_WSERVER_E_OK ;

    char* cmd[5]  = {0} ;
    int i ;

    cmd[0] = strchr (endpoint, '/') ;
    for (i=0; i<5; i++) {
        if (cmd[i]) *(cmd[i])++ = 0 ;
        if (cmd[i]) cmd[i+1] = strchr (cmd[i], '/') ;
        if (cmd[i+1] == 0) break ;

    }

    if (method == HTTP_HEADER_METHOD_GET) {

        if (html_emit_lock (&_wengine_emit, 4000) != EOK) {
            return httpserver_write_response (user, WSERVER_RESP_CODE_500, HTTP_SERVER_CONTENT_TYPE_HTML,
                0, 0, WSERVER_RESP_CONTENT_500, strlen(WSERVER_RESP_CONTENT_500)) ;

        }


        const   HTTP_HEADER_T headers[]   = { {"Cache-Control", "no-cache"} };
        if ((res = httpserver_chunked_response (user, 200, 
                        HTTP_SERVER_CONTENT_TYPE_HTML, headers, 
                        sizeof(headers)/sizeof(headers[0])) < HTTP_SERVER_E_OK)) {
            html_emit_unlock (&_wengine_emit) ;
            return res ;

        }

        html_emit_wait (&_wengine_emit, html_emit_cb, user, 4000) ;
        res =  httpserver_chunked_complete (user) ;
        html_emit_unlock (&_wengine_emit) ;
        return res ;

    } else {
        return HTTP_SERVER_WSERVER_E_METHOD ;
    }

    return HTTP_SERVER_WSERVER_E_OK ;
 
}


