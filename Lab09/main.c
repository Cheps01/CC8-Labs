#include "speed_test.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        return 1;
    }
 
    conn_t cnt;
    memset(&cnt, 0, sizeof cnt);
    if (inet_pton(AF_INET, argv[1], &cnt.dst_ip) != 1) {
        fprintf(stderr, "IP invalida\n");
        return 1;
    }
    cnt.dport = (uint16_t)atoi(argv[2]);
 
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    cnt.sport = (uint16_t)(40000 + rand() % 20000);
    uint32_t isn = ((uint32_t)rand() << 16) ^ (uint32_t)rand(); // numero de secuencia inicial
 
    if (local_ip_for(cnt.dst_ip, &cnt.src_ip) < 0) {
        perror("[local_ip_for] couldn't resolve ip");
        return 1;
    }
    cnt.sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (cnt.sock < 0) {
        perror("[socket] root privileges needed");
        return 1;
    }
 
    /* Evita warnings mientras las funciones aun no se usan. Borra esto luego. */
    (void)send_segment;
    (void)wait_for;
    (void)isn;
 
    /* TODO 0 - Latencia: ping al servidor (Net::Ping, o ejecutar `ping -c` y
     *          parsear la salida) y guardar el promedio en ms. */
 
    /* TODO 1 - Handshake de 3 vias.
     *   a) send_segment SYN con seq = isn
     *   b) wait_for(SYN|ACK) y guarda el seq del servidor (ntohl(hdr.th_seq))
     *   c) send_segment ACK con seq = isn + 1, ack = seq_servidor + 1
     *   (SYN y FIN consumen 1 numero de secuencia; los datos consumen `len`)
     */
 
    /* TODO 2 - 10 envios de 1 KB sobre la MISMA conexion.
     *   Por cada uno: t0 = now_us(); send_segment con PSH|ACK y los datos;
     *   wait_for(ACK) y verifica que su ack == tu seq + len; rtt = t_recv - t0.
     *   Avanza tu seq en len. Guarda los 10 RTT en un arreglo.
     */
 
    /* TODO 3 - Cierre: send_segment FIN|ACK, wait_for el ACK/FIN del servidor,
     *          y responde con el ACK final si hace falta. */
 
    /* TODO 4 - Imprimir latencia, RTT promedio de los 10 envios y la velocidad:
     *          (bytes * 8) / (2 * rtt_promedio)   segun la formula del enunciado. */
 
    close(cnt.sock);
    return 0;
}