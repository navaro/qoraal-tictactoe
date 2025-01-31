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

static void 
html_emit_cb (void* ctx, const char* html, uint32_t len)
{
    HTTP_USER_T *user = (HTTP_USER_T *) ctx ;
    if (user && html && len) {
        httpserver_chunked_append_str (user, html, len) ;

    }
    
}

/**
 * @name    HTML text
 * @{
 */
#define WSERVER_ABOUT_TEXT      "Qoraal About"

const char*
wengine_metadata (HTTP_USER_T *user, uint32_t method, char* endpoint, uint32_t type)
{
    if (type == WSERVER_METADATA_TYPE_HEADING) {
        return "Engine" ;
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

    const char * filename = cmd[0] ;
    if (!filename)  filename = "welcome.e" ;

    if (method == HTTP_HEADER_METHOD_GET) {

        if (engine_machine_start (filename,0, 0, true, false) || !html_emit_ready()) {
            httpserver_write_response (user, WSERVER_RESP_CODE_400, HTTP_SERVER_CONTENT_TYPE_HTML,
                0, 0, WSERVER_RESP_CONTENT_400, strlen(WSERVER_RESP_CONTENT_400)) ;

        }

        const   HTTP_HEADER_T headers[]   = { {"Cache-Control", "no-cache"} };
        if ((res = httpserver_chunked_response (user, 200, 
                        HTTP_SERVER_CONTENT_TYPE_HTML, headers, 
                        sizeof(headers)/sizeof(headers[0])) < HTTP_SERVER_E_OK)) {
            return res ;

        }

        HTML_EMIT_T emit ;
        html_emit_wait (&emit, html_emit_cb, user, 4000) ;
        engine_machine_stop () ;
        return httpserver_chunked_complete (user) ;

    } else {
        return HTTP_SERVER_WSERVER_E_METHOD ;
    }

    return HTTP_SERVER_WSERVER_E_OK ;
 
}


