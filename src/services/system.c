#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "qoraal/qoraal.h"
#include "qoraal/svc/svc_services.h"
#include "qoraal/svc/svc_tasks.h"
#include "qoraal/svc/svc_shell.h"
#include "qoraal-flash/syslog.h"


/*===========================================================================*/
/* Macros and Defines                                                        */
/*===========================================================================*/

SVC_SERVICES_T      _system_service_id = SVC_SERVICES_INVALID ;   
#define DBG_MESSAGE_SYSTEM(severity, fmt_str, ...)   DBG_MESSAGE_T_LOG (SVC_LOGGER_TYPE(severity,0), _system_service_id, fmt_str, ##__VA_ARGS__)

/*===========================================================================*/
/* Service local variables and types.                                        */
/*===========================================================================*/

static p_sem_t              _system_stop_sem ;
static SVC_TASKS_DECL       (_system_startup_task) ;
static SVC_TASKS_DECL       (_system_periodic_task) ;

/*===========================================================================*/
/* Service local functions.                                                  */
/*===========================================================================*/

static void     system_startup_cb (SVC_TASKS_T *task, uintptr_t parm, uint32_t reason) ;
static void     system_periodic_cb (SVC_TASKS_T *task, uintptr_t parm, uint32_t reason) ;
static void     system_syslog_cb (void* channel, LOGGERT_TYPE_T type, uint8_t facility, const char* msg) ;

static int32_t  qshell_cmd_status (SVC_SHELL_IF_T * pif, char** argv, int argc) ;


SVC_SHELL_CMD_LIST_START(system, 0)
SVC_SHELL_CMD_LIST( "status", qshell_cmd_status,  "")
SVC_SHELL_CMD_LIST_END()

static LOGGER_CHANNEL_T     _system_log_channel = { .fp = system_syslog_cb, .user = (void*)0, .filter = { { .mask = SVC_LOGGER_MASK, .type = SVC_LOGGER_SEVERITY_LOG | SVC_LOGGER_FLAGS_NO_TIMESTAMP }, {0,0} } };


/*===========================================================================*/
/* Service Functions                                                         */
/*===========================================================================*/

/**
 * @brief       system_service_ctrl
 * @details
 * @note        For code SVC_SERVICE_CTRL_STATUS, if the return value is E_NOIMPL 
 *              the status will be determined by the svc_services module.
 *
 * @param[in] code
 * @param[in] arg
 *
 * @return              status
 *
 * @services
 */
int32_t
system_service_ctrl (uint32_t code, uintptr_t arg)
{
    int32_t res = EOK ;

    switch (code) {
    case SVC_SERVICE_CTRL_INIT:
        _system_service_id = svc_service_service ((SCV_SERVICE_HANDLE) arg ) ;
        break ;

    case SVC_SERVICE_CTRL_START:{
        svc_logger_channel_add (&_system_log_channel) ;
        DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_REPORT, 
                "SYS   : : system starting...");

        os_sem_create (&_system_stop_sem, 0) ;
        svc_tasks_schedule (&_system_startup_task, system_startup_cb, 0,
                SERVICE_PRIO_QUEUE2, SVC_TASK_MS2TICKS(5000)) ;        
        svc_tasks_schedule (&_system_periodic_task, system_periodic_cb, 0,
                SERVICE_PRIO_QUEUE4, SVC_TASK_S2TICKS(40)) ;    
        SVC_SHELL_CMD_LIST_INSTALL(system) ;
        } 
        break ;

    case SVC_SERVICE_CTRL_STOP: {
        DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_REPORT, 
                "SYS   : : system stopping...");
        svc_logger_channel_remove (&_system_log_channel) ;

        SVC_SHELL_CMD_LIST_UNINSTALL(system) ;
        svc_tasks_cancel (&_system_startup_task) ;
        svc_tasks_cancel (&_system_periodic_task) ;
        os_sem_signal (&_system_stop_sem) ;
        }
        break ;


    case SVC_SERVICE_CTRL_STATUS:
    default:
        res = E_NOIMPL ;
        break ;

    }
    return res ;
}

int32_t
system_service_run (uintptr_t arg)
{
    DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_INFO, "SYS   : : system started.");

    while (1) {
        if (os_sem_wait_timeout (&_system_stop_sem, OS_S2TICKS(120)) == EOK) {
            break ;
        }

        DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_REPORT, 
                "SYS   : : system running...");

    }

    os_sem_delete (&_system_stop_sem) ;
    
    DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_REPORT, 
                "SYS   : : system complete.");

    return EOK ;
}



static void
system_startup_cb (SVC_TASKS_T *task, uintptr_t parm, uint32_t reason)
{
    if (reason == SERVICE_CALLBACK_REASON_RUN) {

        DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_REPORT, 
                "SYS   : : system 'STARTUP TASK'...");

    }

    svc_tasks_complete (task) ;
}

static void
system_periodic_cb (SVC_TASKS_T *task, uintptr_t parm, uint32_t reason)
{
    if (reason == SERVICE_CALLBACK_REASON_RUN) {

        DBG_MESSAGE_SYSTEM (DBG_MESSAGE_SEVERITY_LOG, 
                "SYS   : : system 'PERIODIC TASK'...");

        svc_tasks_schedule (&_system_periodic_task, system_periodic_cb, 0,
                SERVICE_PRIO_QUEUE4, SVC_TASK_S2TICKS(90)) ;        

    }

    svc_tasks_complete (task) ;
}

int32_t
qshell_cmd_status (SVC_SHELL_IF_T * pif, char** argv, int argc)
{
    int32_t res = SVC_SHELL_CMD_E_OK ;

    svc_shell_print (pif, SVC_SHELL_OUT_STD, "'PERIODIC TASK' scheduled in %d seconds\r\n",
            SVC_TASK_TICKS2MS(svc_task_expire(&_system_periodic_task))/1000) ;
    
    return res ;
}


/**
 * @brief       system_syslog_cb
 * @details     Callback function for logging messages from the shell.
 *
 * @param[in]   channel     The logger channel.
 * @param[in]   type        The type of log message.
 * @param[in]   facility    The logging facility.
 * @param[in]   msg         The log message to display.
 */
void
system_syslog_cb (void* channel, LOGGERT_TYPE_T type, uint8_t facility, const char* msg)
{
    syslog_append (SYSLOG_INFO_LOG, facility, SVC_LOGGER_GET_SEVERITY(type), msg) ;
    if (SVC_LOGGER_GET_SEVERITY(type) < SVC_LOGGER_SEVERITY_ERROR) {
        syslog_append (SYSLOG_ASSERT_LOG, facility, SVC_LOGGER_GET_SEVERITY(type), msg) ;
    }
}
