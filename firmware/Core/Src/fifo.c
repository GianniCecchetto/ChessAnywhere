#include "fifo.h"
#include <string.h>
#include "stm32g0xx_hal.h"

void uart_fifo_init(uart_fifo_t *f) {
    f->len = 0;
    f->overflow = 0;
}

int uart_fifo_push_isr(uart_fifo_t *f, uint8_t c) {
    // appelé depuis ISR : pas de désactivation d'interrupt ici
    uint16_t l = f->len;
    if (l >= UART_FIFO_SIZE) {
        f->overflow++;
        return 0; // overflow, on jette l'octet
    }
    f->buf[l] = c;
    // écrire len en dernier pour éviter de laisser état incohérent
    f->len = l + 1;
    return 1;
}

int uart_fifo_get_command(uart_fifo_t *f, char *out, size_t out_size) {
    if (out_size == 0) return 0;

    // courte section critique pour éviter que l'ISR ne modifie len pendant la recherche/copie.
    __disable_irq();

    uint16_t len = f->len;
    uint16_t pos = 0xFFFF;
    // chercher '\n'
    for (uint16_t i = 0; i < len; ++i) {
        if (f->buf[i] == '\n') {
            pos = i;
            break;
        }
    }
    if (pos == 0xFFFF) {
        // pas de commande complète
        __enable_irq();
        return 0;
    }

    // determine longueur de la commande sans \r\n
    uint16_t cmd_end = pos; // index du '\n'
    // si précédent est '\r', on le retire aussi
    if (cmd_end > 0 && f->buf[cmd_end - 1] == '\r') {
        cmd_end -= 1;
    }

    int ret;
    if ((size_t)cmd_end < out_size) {
        // on peut tout copier + '\0'
        memcpy(out, f->buf, cmd_end);
        out[cmd_end] = '\0';
        ret = (int)cmd_end;
    } else {
        // tronqué
        memcpy(out, f->buf, out_size - 1);
        out[out_size - 1] = '\0';
        ret = -1;
    }

    // décale le contenu restant (après le '\n') au début du buffer
    uint16_t remaining = len - (pos + 1);
    if (remaining > 0) {
        memmove(f->buf, f->buf + pos + 1, remaining);
    }
    f->len = remaining;

    __enable_irq();
    return ret;
}
