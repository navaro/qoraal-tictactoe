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



// #include "system_config.h"
#if 1

#include <stdio.h>
#include <string.h>
#include "qoraal/config.h"
#include "qoraal/qoraal.h"
#include "qoraal-http/qoraal.h"
#include "qoraal/svc/svc_services.h"
#include "qoraal/svc/svc_shell.h"
#include "qoraal/svc/svc_logger.h"
#include "services.h"

#define SHELL_PROMPT        "[Qoraal] #> "

/*===========================================================================*/
/* Client pre-compile time settings.                                         */
/*===========================================================================*/
#define DBG_MESSAGE_TELNET(severity, fmt_str, ...)          DBG_MESSAGE_T(SVC_LOGGER_TYPE(severity,0), QORAAL_SERVICE_TELNET, fmt_str, ##__VA_ARGS__)

/*===========================================================================*/
/* Constants.                                                                */
/*===========================================================================*/

#define TELENTSERVER_SESSION_TIMEOUT        (60*10)



static int32_t  telenetserver_init (uintptr_t arg) ;
static int32_t  telenetserver_start (uintptr_t arg) ;
static int32_t  telenetserver_stop (uintptr_t arg) ;


static int32_t _telenetserver_quit = 0 ;

static int                  _telnet_server_sock = -1;
static int                  _telnet_client_sock = -1 ;
// static LOGGER_CHANNEL_T     _telnet_log_channel = {0} ;


static void
_qshell_prompt(int sock, const char * prompt)
{
    int len = strlen(prompt)  ;

    send( sock, prompt, len, 0);


    return ;
}


static int32_t
_qshell_out(void* ctx, uint32_t out, const char* str)
{
    int sock = (int) ctx ;
    //uint32_t prio = os_thread_get_prio () ;


    if (out == SVC_SHELL_IN_STD) {
        uint32_t timeout = 0 ;
        if (str) {
             svc_shell_scan_int(str, &timeout) ;
        }

        fd_set   fdread;
        fd_set   fdex;
        struct timeval tv;
        uint8_t buffer[1] ;


        FD_ZERO(&fdread) ;
        FD_ZERO(&fdex) ;
        FD_SET(_telnet_client_sock, &fdread);
        FD_SET(_telnet_client_sock, &fdex);
        tv.tv_sec = timeout/1000 ;
        tv.tv_usec = (timeout%1000)*1000 ; ;


        int32_t status = select (_telnet_client_sock+1, &fdread, 0, &fdex, &tv);

        if (_telenetserver_quit || (status < 0) || (FD_ISSET(_telnet_client_sock, &fdex))) {
            return SVC_SHELL_CMD_E_BREAK ;
        }

        if (status > 0) {
            status = recv (_telnet_client_sock, buffer, sizeof(buffer), 0);
            return buffer[0] ;
            
        }

    }


    else if (sock && str) {
        int len = strlen(str)  ;

        send(sock, (void*)str, len, 0);

    }

    return EOK ;
}

#define SHELLCMD_MAX            192
static char     _shellcmd_cmd[SHELLCMD_MAX]   ;

void     telenetserver_reset (void)
{
    _shellcmd_cmd[0] = '\0' ;
}

int32_t     telenetserver_shellcmd (int sock, const char* shellcmd, int len)
{
    //const char* str = 0 ;
    int32_t res = EOK ;
    char *argv[SVC_SHELL_ARGC_MAX];
    int argc ;


    if (shellcmd && len) {

        unsigned int i ;
        SVC_SHELL_IF_T shellif ;

        if (len + strlen(_shellcmd_cmd) > SHELLCMD_MAX-2) {
            _qshell_prompt (sock, SHELL_PROMPT) ;
            _shellcmd_cmd[0] = '\0' ;
            return res ;
        }
        strncat (_shellcmd_cmd, shellcmd, len) ;

        for (i=0; _shellcmd_cmd[i] != '\0'; i++) {
            if ((_shellcmd_cmd[i] == '\r') || (_shellcmd_cmd[i] == '\n')) {
                _shellcmd_cmd[i] = '\0' ;
                argc = svc_shell_cmd_split(_shellcmd_cmd, i, argv, SVC_SHELL_ARGC_MAX-1);
                if (argc > 0) {
                    if (strncmp(argv[0], "exit", 4) == 0) {
                        res = E_EOF ;
                    } else {
                        svc_shell_if_init (&shellif, (void*)sock, _qshell_out, 0) ;
                        /*res =*/ svc_shell_cmd_run (&shellif, &argv[0],  argc-0) ;
                        _qshell_prompt (sock, SHELL_PROMPT) ;
                    }
                } else {
                    _qshell_prompt (sock, SHELL_PROMPT) ;
                }
                _shellcmd_cmd[0] = '\0' ;
                break ;
            }
            if (_shellcmd_cmd[i] == 0x8) { // backspace
                if (i > 0) {
                    unsigned int j   ;
                    for (j = i - 1; j<SHELLCMD_MAX-2; j++) {
                        _shellcmd_cmd[j] = _shellcmd_cmd[j+2] ;
                        if (_shellcmd_cmd[j] == '\0') break ;
                    }
                } else {
                    _shellcmd_cmd[0] = '\0' ;
                    break ;

                }

            }
            if (_shellcmd_cmd[i] == 0x3) { // ctrl-c
                _qshell_prompt (sock, SHELL_PROMPT) ;
                _shellcmd_cmd[0] = '\0' ;
                break ;

            }

        }


    }


    return res ;
}

void
_telnetserver_hello(int client)
{
    int len = 128  ;
    char* buffer = qoraal_malloc(QORAAL_HeapAuxiliary, len+1) ;

    if (!buffer) {
        DBG_MESSAGE_TELNET(DBG_MESSAGE_SEVERITY_WARNING,
                "TELNE :W: malloc failed for %d bytes!",
                len);
        return  ;
    }
    len = snprintf (buffer,128,  "%s\r\n" SHELL_PROMPT " ", "Telnet Server") ;

    send(client, (char*)buffer, len, 0);
    qoraal_free (QORAAL_HeapAuxiliary, buffer)  ;

}
        
int32_t telnetserver_service_run (uintptr_t arg)
{
    int32_t status = EOK ;
    uint32_t len ;
    //void* recv ;
    struct sockaddr_in   recv_from = { 0 };
    //IP4_SOCKADDR_DECL(addr, 0, 0, 0, 0, 0) ;
    struct sockaddr_in address = {0};
    int32_t timeout  ;
    char    buffer[32] ;

    fd_set   fdread;
    fd_set   fdex;
    struct timeval tv;

    if (arg == 0) arg = 23 ;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t)arg);
#if QORAAL_CFG_USE_LWIP
    address.sin_len = sizeof(address);
#endif

    DBG_MESSAGE_TELNET(DBG_MESSAGE_SEVERITY_INFO,
                "TELNE : : starting telnet server on port %d...",
                arg);

    while (!_telenetserver_quit) {


        //if (network_state_wait_enter(NETWORK_STATE_IP_UP, 500) != EOK) {
        //     continue ;
       //}

        /*
         * Create a listener socket.
         */
        _telnet_server_sock = socket (AF_INET, SOCK_STREAM, 0) ;
        if (_telnet_server_sock < 0) {
            DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_WARNING,
                    "TELNE :W: telenetserver on port %d  t_socket FAIL %d!",
                    arg, _telnet_server_sock);
            break ;
        }

        /*
         * Bind it to the local address (0.0.0.0) on the specified port.
         */
        status = bind (_telnet_server_sock, (struct sockaddr *)&address, sizeof (address)) ;
        if (status < 0) {
            DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_WARNING,
                    "TELNE :W: telenetserver on port %d  t_bind FAILED!",
                    arg);
            closesocket (_telnet_server_sock) ;
            break ;
        }

        while (!_telenetserver_quit)  {
            /*
             * Start listening for incoming sockets.
             * This call is asynchronous and we do a select on the socket to check for incoming clients.
             * t_listen() needs to be called everytime we want to accept a new client.
             *
             */
            status = listen (_telnet_server_sock, 1) ;
            if (status < 0 ) {
                DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_WARNING,
                            "TELNE :W: telenetserver on port %d failed to put socket in listen mode",
                            arg);
                break ;
            }

            do {
                DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_DEBUG,
                            "TELNE : : Waiting for client to connect...");
                /*
                 * Here we wait for connecting clients
                 */
                FD_ZERO(&fdread) ;
                FD_ZERO(&fdex) ;
                FD_SET(_telnet_server_sock, &fdread);
                FD_SET(_telnet_server_sock, &fdex);
                tv.tv_sec = 2 ;
                tv.tv_usec = 0 ;

                status = select(_telnet_server_sock+1, &fdread, 0, &fdex, &tv) ;

                if (FD_ISSET(_telnet_server_sock, &fdex)) {
                    DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_REPORT,
                                "TELNE : : Exception on listening socket...");
                    status = -1 ;
                }

            } while (!_telenetserver_quit && (status == 0)) ;

            if ((status < 0) || _telenetserver_quit) {
                break ;
            }

            /*
             * Accept the client trying to connect.
             */
            len = sizeof(recv_from) ;
            _telnet_client_sock = accept (_telnet_server_sock, (struct sockaddr *) &recv_from, &len) ;

            if (_telnet_client_sock < 0) {
                /*
                 * The connection failed, we continue and call t_listen() again to wait for a new client.
                 */
                DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_WARNING,
                            "TELNE :W: telenetserver on port %d failed connecting to client",
                            arg);
                continue ;
            }
#if 0
            status = t_setsockopt(athwifi_get_handle(), client, SOL_SOCKET, SO_LINGER,
                        (unsigned char *) &linger, sizeof(linger));

            if (status ==A_ERROR) {
                DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_ERROR,
                            "t_setsockopt SO_LINGER on client failed") ;
           }
#endif
            DBG_MESSAGE_TELNET (DBG_MESSAGE_SEVERITY_LOG,
                            "TELNE : : telenetserver on port %d client connected",
                            arg) ;

            telenetserver_reset () ;
            _telnetserver_hello (_telnet_client_sock) ;

            timeout = TELENTSERVER_SESSION_TIMEOUT ;
            status = 0 ;
            while (!_telenetserver_quit && (timeout > 0) && (status >= 0))  {

                FD_ZERO(&fdread) ;
                FD_ZERO(&fdex) ;
                FD_SET(_telnet_client_sock, &fdread);
                FD_SET(_telnet_client_sock, &fdex);
                tv.tv_sec = 2 ;
                tv.tv_usec = 0 ;


                status = select (_telnet_client_sock+1, &fdread, 0, &fdex, &tv);
                timeout -= tv.tv_sec ;

                if (_telenetserver_quit || (status < 0) || (FD_ISSET(_telnet_client_sock, &fdex))) {
                    closesocket (_telnet_client_sock) ;
                    _telnet_client_sock = -1 ;
                    status = -2 ;
                }

                while (status > 0) {
                    status = recv (_telnet_client_sock, buffer, sizeof(buffer), MSG_DONTWAIT);

                    if ((status < 0)) {
                        break ;
                    }

                    if (status > 0) {

                        timeout = TELENTSERVER_SESSION_TIMEOUT ;
                        //DBG_MESSAGE (DBG_MESSAGE_SEVERITY_ERROR, ("telenetserver %d: Client received %d bytes...", param, len));
                        /*
                         * Do not try to transmit a received buffer directly as it may
                         * lead to deadlocks. Always copy it into a TX buffer.
                         */



                        status = telenetserver_shellcmd (_telnet_client_sock, buffer, status) ;



                    }
                }




            }  ;

            _qshell_prompt (_telnet_client_sock, "bye!") ;


            if (_telnet_client_sock>=0) {
                shutdown (_telnet_client_sock, 0) ;
                closesocket (_telnet_client_sock) ;
                _telnet_client_sock = -1 ;

            }
            DBG_MESSAGE_SERVICES (DBG_MESSAGE_SEVERITY_LOG,
                    "TELNE :  : telenetserver on port %d client disconnected",
                    arg) ;


        }

        if (_telnet_server_sock>=0) {
            shutdown (_telnet_server_sock, 0) ;
            closesocket (_telnet_server_sock) ;
            _telnet_client_sock = -1 ;

        }

    }
    _telnet_client_sock = -1 ;
    _telnet_server_sock = -1 ;

    DBG_MESSAGE_TELNET(DBG_MESSAGE_SEVERITY_LOG,
                "TELNE : : telenetserver on port %d terminated.",
                arg);

    return EOK ;

}


/**
 * @brief       power_service_ctrl
 * @details
 * @note
 *
 * @param[in] code
 * @param[in] arg
 *
 * @return              status
 *
 * @power
 */
int32_t     telnetserver_service_ctrl (uint32_t code, uintptr_t arg)
{
    int32_t res = E_NOIMPL ;

    switch (code) {
    case SVC_SERVICE_CTRL_INIT:
        res = telenetserver_init (arg) ;
        break ;

    case SVC_SERVICE_CTRL_START:
        res = telenetserver_start(arg) ;
        break ;

    case SVC_SERVICE_CTRL_STOP:
        res = telenetserver_stop(arg) ;
        break ;

    case SVC_SERVICE_CTRL_STATUS:
    default:
        break ;

    }

    return res ;
}

int32_t
telenetserver_init (uintptr_t arg)
{

    return EOK ;
}


int32_t
telenetserver_start (uintptr_t port)
{

    _telenetserver_quit = 0 ;
    return EOK ;
}


int32_t
telenetserver_stop (uintptr_t arg)
{
    DBG_MESSAGE_TELNET(DBG_MESSAGE_SEVERITY_INFO,
            "TELNE : : telenetserver stopping...");
    if (_telnet_server_sock>=0) {
        int s = _telnet_server_sock ;
        _telnet_server_sock = -1 ;
        shutdown (s, 0) ;
        close (s) ;
    }
    if (_telnet_client_sock>=0) {
        int s = _telnet_client_sock ;
        _telnet_client_sock = -1 ;
        shutdown (s, 0) ;
        close (s) ;
    }

    _telenetserver_quit = 1 ;
    return EOK ;
}


#endif /* */
