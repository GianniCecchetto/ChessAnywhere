#ifndef UART_FIFO_H
#define UART_FIFO_H

#include <stdint.h>
#include <stddef.h>

#define UART_FIFO_SIZE 256  // ajuste si besoin

typedef struct {
    uint8_t buf[UART_FIFO_SIZE];
    volatile uint16_t len;      // nombre d'octets actuellement dans le buffer
    volatile uint32_t overflow; // compteur d'overflow
} uart_fifo_t;

/**
 * Initialise le FIFO (doit être appelé avant usage)
 */
void uart_fifo_init(uart_fifo_t *f);

/**
 * Pousser un octet dans le FIFO -- appelé depuis l'ISR.
 * Retourne 1 si succès, 0 si overflow (octet rejeté).
 * Doit être court et sû r pour ISR.
 */
int uart_fifo_push_isr(uart_fifo_t *f, uint8_t c);

/**
 * Si une commande terminée par '\n' est présente, la copie dans out (terminée par '\0'),
 * enlève la commande du FIFO (et décale le reste).
 *
 * - out_size : taille du buffer out (incluant le '\0')
 * - renvoie :
 *    > 0 : longueur de la commande copiée (sans le '\n' ni '\r'), succès
 *    0   : pas de commande complète disponible
 *   -1   : commande trouvée mais tronquée (copie jusqu'à out_size-1, renvoie -1)
 *
 * La fonction enlève la commande (et le '\n') du FIFO ; le contenu restant
 * est memmové au début du buffer.
 */
int uart_fifo_get_command(uart_fifo_t *f, char *out, size_t out_size);

#endif // UART_FIFO_H
