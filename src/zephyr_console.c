// console_uart_zephyr.c
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include "qoraal/qoraal.h"

/* ===== Config ===== */
#define RX_RB_SZ   64
#define TX_RB_SZ   512
#define TX_CHUNK   16

/* ===== State ===== */
static const struct device *zephyr_console_uart = 0;

/* RX */
static struct ring_buf rx_rb;
static uint8_t rx_mem[RX_RB_SZ];
/* counting semaphore: posted on every RX chunk */
static K_SEM_DEFINE(rx_sem, 0, K_SEM_MAX_LIMIT);

/* TX */
static struct ring_buf tx_rb;
static uint8_t tx_mem[TX_RB_SZ];
/* binary “space freed / TX idle” edge from ISR */
static K_SEM_DEFINE(tx_space, 0, 1);
/* binary writer lock (mutual exclusion) */
static K_SEM_DEFINE(tx_lock, 1, 1);

/* ===== ISR ===== */
static void usart_isr(const struct device *dev, void *user)
{
    ARG_UNUSED(user);

    /* RX: drain FIFO -> ring, wake reader */
    while (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
        uint8_t tmp[32];
        int n = uart_fifo_read(dev, tmp, sizeof(tmp));
        if (n <= 0) break;
        (void)ring_buf_put(&rx_rb, tmp, (uint32_t)n);
        k_sem_give(&rx_sem);               /* ISR-safe */
    }

    /* TX: ring -> FIFO while ready */
    while (uart_irq_tx_ready(dev)) {
        uint8_t *blk;
        uint32_t avail = ring_buf_get_claim(&tx_rb, &blk, TX_CHUNK);
        if (!avail) {
            /* Ring empty: stop TX IRQ, signal any waiters */
            uart_irq_tx_disable(dev);
            k_sem_give(&tx_space);         /* ISR-safe; stays at 1 (binary) */
            break;
        }

        int sent = uart_fifo_fill(dev, blk, (int)avail);
        if (sent > 0) {
            ring_buf_get_finish(&tx_rb, (uint32_t)sent);
            k_sem_give(&tx_space);         /* some space freed */
        }
        if (sent < (int)avail) {
            /* HW FIFO full; try again on next TX-ready */
            break;
        }
    }
}

/* ===== Core init (callable & used by SYS_INIT) ===== */
int console_uart_init(void)
{
    /* Prefer the chosen console if present; else fallback to uart0 */
#if DT_NODE_HAS_STATUS(DT_CHOSEN(zephyr_console), okay)
    zephyr_console_uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#else
    zephyr_console_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
#endif
    if (!device_is_ready(zephyr_console_uart)) {
        return -ENODEV;
    }

    ring_buf_init(&rx_rb, sizeof(rx_mem), rx_mem);
    ring_buf_init(&tx_rb, sizeof(tx_mem), tx_mem);

    uart_irq_callback_user_data_set(zephyr_console_uart, usart_isr, NULL);
    uart_irq_rx_enable(zephyr_console_uart);  /* TX IRQ enabled on demand */

    return 0;
}

/* ===== Zephyr init hook ===== */
static int console_uart_zephyr_init(const struct device *unused)
{
    ARG_UNUSED(unused);
    return console_uart_init();
}
/* After drivers are ready (adjust prio if you like) */
SYS_INIT(console_uart_zephyr_init, APPLICATION, 60);

/* ===== RX: one char, no polling ===== */
int32_t console_get_char(uint32_t timeout_ms)
{
    uint8_t c;

	if (!zephyr_console_uart) {
		return EOF ;

	}

    if (ring_buf_get(&rx_rb, &c, 1) == 1) {
        return (int32_t)c;
    }

    if (k_sem_take(&rx_sem, K_MSEC(timeout_ms)) != 0) {
        return 0; /* timeout */
    }

    return (ring_buf_get(&rx_rb, &c, 1) == 1) ? (int32_t)c : 0;
}

/* ===== TX: serialize writers + enqueue + ISR drains ===== */
size_t console_write(const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    size_t written = 0;

	if (!zephyr_console_uart) {
		return written ;

	}

    /* binary semaphore as a mutex (no PI needed; HW-bound) */
    if (k_sem_take(&tx_lock, K_MSEC(timeout_ms)) != 0) {
        return 0; /* couldn't get the lock */
    }

    for (;;) {
        if (written < len) {
            uint32_t put = ring_buf_put(&tx_rb, &data[written],
                                        (uint32_t)(len - written));
            written += put;
        }

        /* Kick TX so ISR starts draining */
        uart_irq_tx_enable(zephyr_console_uart);

        if (written >= len) {
            break; /* everything queued */
        }

        /* Ring full: wait for ISR to free space or timeout */
        if (k_sem_take(&tx_space, K_MSEC(timeout_ms)) != 0) {
            /* timeout => partial write */
            break;
        }
        /* loop and try to enqueue more */
    }

    k_sem_give(&tx_lock);
    return written;
}

/* ===== Optional: tiny “shell” shims when Zephyr shell is off ===== */
#if !IS_ENABLED(CONFIG_SHELL)
#include <stdarg.h>

struct shell; /* fwd-declare */

static inline void write_str(const char *s)
{
    (void)console_write((const uint8_t *)s, strlen(s), 500);
}

void shell_print(const struct shell *sh, const char *fmt, ...)
{
    ARG_UNUSED(sh);
    va_list ap; va_start(ap, fmt);
    svc_logger_type_vlog (SVC_LOGGER_TYPE(SVC_LOGGER_SEVERITY_LOG, 0), 0, fmt, ap);
    va_end(ap);
    write_str("\n");
}

void shell_warn(const struct shell *sh, const char *fmt, ...)
{
    ARG_UNUSED(sh);
    write_str("WARN: ");
    va_list ap; va_start(ap, fmt);
    svc_logger_type_vlog (SVC_LOGGER_TYPE(SVC_LOGGER_SEVERITY_WARNING, 0), 0, fmt, ap);
    //vwrite_fmt(fmt, ap);
    va_end(ap);
    //write_str("\n");
}

void shell_error(const struct shell *sh, const char *fmt, ...)
{
    ARG_UNUSED(sh);
    write_str("ERR: ");
    va_list ap; va_start(ap, fmt);
    svc_logger_type_vlog (SVC_LOGGER_TYPE(SVC_LOGGER_SEVERITY_ERROR, 0), 0, fmt, ap);
    //vwrite_fmt(fmt, ap);
    va_end(ap);
    //write_str("\n");
}

void shell_fprintf_normal(const struct shell *sh, const char *fmt, ...)
{
    ARG_UNUSED(sh);
    va_list ap; va_start(ap, fmt);
    svc_logger_type_vlog (SVC_LOGGER_TYPE(SVC_LOGGER_SEVERITY_LOG, 0), 0, fmt, ap);
    va_end(ap);
}

void shell_fprintf_error(const struct shell *sh, const char *fmt, ...)
{
    ARG_UNUSED(sh);
    write_str("ERR: ");
    va_list ap; va_start(ap, fmt);
    svc_logger_type_vlog (SVC_LOGGER_TYPE(SVC_LOGGER_SEVERITY_ERROR, 0), 0, fmt, ap);
    va_end(ap);
}
#endif /* !CONFIG_SHELL */
