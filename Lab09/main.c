#include "speed_test.h"

#define N_ENVIOS 10
#define TAM 1000

// Cálculo de latencia
static double latency_ms(const char *ip) {
    char cmd[128], line[256];
    double avg = -1.0;

    snprintf(cmd, sizeof cmd, "ping -c 4 -q %s 2>/dev/null", ip);
    FILE *fp = popen(cmd, "r");
    if (!fp)
        return avg;
    while (fgets(line, sizeof line, fp)) {
        /* línea típica: rtt min/avg/max/mdev = 0.045/0.052/0.061/0.007 ms */
        char *eq = strchr(line, '=');
        if (strstr(line, "min/avg/max") && eq) {
            double mn, av, mx;
            if (sscanf(eq + 1, " %lf/%lf/%lf", &mn, &av, &mx) == 3)
                avg = av;
        }
    }
    pclose(fp);
    return avg;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        return 1;
    }

    conn_t c;
    memset(&c, 0, sizeof c);
    if (inet_pton(AF_INET, argv[1], &c.dst_ip) != 1) {
        fprintf(stderr, "IP invalida\n");
        return 1;
    }
    c.dport = (uint16_t)atoi(argv[2]);

    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    c.sport = (uint16_t)(40000 + rand() % 20000);
    uint32_t isn = ((uint32_t)rand() << 16) ^ (uint32_t)rand();

    if (local_ip_for(c.dst_ip, &c.src_ip) < 0) {
        perror("local_ip_for");
        return 1;
    }
    c.sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (c.sock < 0) {
        perror("socket (necesitas root)");
        return 1;
    }

    // Calcular latencia
    double latencia_ms = latency_ms(argv[1]);

    // Handshake 
    struct tcphdr h;
    const uint8_t *pl;
    size_t plen;
    int64_t t;

    if (send_segment(&c, isn, 0, F_SYN, NULL, 0) < 0) {
        perror("enviar SYN");
        return 1;
    }
    if (wait_for(&c, F_SYN | F_ACK, &h, &pl, &plen, &t) < 0) {
        fprintf(stderr, "No llegó el SYN-ACK\n");
        return 1;
    }
    uint32_t my_seq = isn + 1;
    uint32_t my_ack = ntohl(h.th_seq) + 1;
    if (send_segment(&c, my_seq, my_ack, F_ACK, NULL, 0) < 0) {
        perror("enviar ACK");
        return 1;
    }

    // 10 envíos de 1 KB bajo la misma conexión
    uint8_t datos[TAM];
    memset(datos, 'A', sizeof datos); /* o cargar aquí el archivo de 1 KB con fread */

    double suma_s = 0.0;
    for (int i = 0; i < N_ENVIOS; i++) {
        int64_t t0 = now_us();
        if (send_segment(&c, my_seq, my_ack, F_PSH | F_ACK, datos, TAM) < 0) {
            perror("enviar datos");
            return 1;
        }

        /* esperar el ACK que confirma TODO lo enviado (my_seq + TAM) */
        int ok = 0;
        while (wait_for(&c, F_ACK, &h, &pl, &plen, &t) == 0) {
            if (ntohl(h.th_ack) == my_seq + TAM) {
                ok = 1;
                break;
            }
        }
        if (!ok) {
            fprintf(stderr, "Sin ACK para el envío %d\n", i + 1);
            return 1;
        }

        double rtt_s = (double)(t - t0) / 1e6;
        printf("Envío %2d: RTT = %.3f ms\n", i + 1, rtt_s * 1000.0);
        suma_s += rtt_s;
        my_seq += TAM;
    }

    // Cierre conexion
    send_segment(&c, my_seq, my_ack, F_FIN | F_ACK, NULL, 0);
    my_seq += 1; /* el FIN consume 1 */
    if (wait_for(&c, F_ACK, &h, &pl, &plen, &t) == 0) {
        if (!(h.th_flags & F_FIN))
            wait_for(&c, F_FIN, &h, &pl, &plen, &t); /* el FIN puede venir aparte */
        if (h.th_flags & F_FIN)
            send_segment(&c, my_seq, ntohl(h.th_seq) + 1, F_ACK, NULL, 0);
    }
    close(c.sock);

    // Resultados
    double rtt_prom_s = suma_s / N_ENVIOS;
    double kbps = ((double)TAM * 8.0) / (2.0 * rtt_prom_s) / 1000.0;

    printf("\nLatencia (ping, promedio): %.3f ms\n", latencia_ms);
    printf("RTT promedio de %d envíos: %.3f ms\n", N_ENVIOS, rtt_prom_s * 1000.0);
    printf("Velocidad de subida: %.2f Kbps\n", kbps);
    return 0;
}