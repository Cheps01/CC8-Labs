# Laboratorio SpeedTest - Ciencias de la Computación VIII

Medición de latencia y velocidad de subida (upload) usando un cliente TCP
implementado con **socket RAW** en C. El cliente construye a mano los
segmentos TCP (SYN, ACK, DATA, FIN) y calcula los números SEQ/ACK y el checksum.

## Requisitos

- Linux (no probado en macOS)
- `gcc` y `make`
- Permisos de root (los sockets RAW lo requieren)

## Compilación

```bash
make
```

Genera el ejecutable `speed_test` en la raíz del proyecto.

## Ejecución

1. **Bloquear los RST del kernel.** El kernel no conoce la conexión creada con
   el socket RAW y respondería RST al SYN-ACK, cerrando el handshake. Se
   bloquean con (reemplazar `<ip>` y `<puerto>` por los del servidor):

   ```bash
   sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -d <ip> --dport <puerto> -j DROP
   ```

2. **Levantar el servidor** del Laboratorio 1 en `<ip>:<puerto>`.

3. **Ejecutar el cliente:**

   ```bash
   sudo ./speed_test <ip> <puerto>
   ```

   Para una prueba local se puede usar `127.0.0.1` (el RTT será de
   microsegundos y la velocidad resultante no es representativa).

4. **Quitar la regla de iptables** al terminar:

   ```bash
   sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST -d <ip> --dport <puerto> -j DROP
   ```

## Qué hace el programa

1. **Latencia:** se mide el tiempo de ida y vuelta hacia el servidor (ping).
2. **Conexión TCP (RAW):** handshake de tres vías (SYN, SYN-ACK, ACK).
3. **Upload:** se envían 10 archivos de 1 KB bajo la misma conexión; por cada
   uno se mide el RTT desde que sale el segmento hasta que llega su ACK.
4. **Cierre:** FIN y ACK final.
5. **Resultado:** se imprime la latencia, el RTT promedio de los 10 envíos y la
   velocidad de subida con la fórmula del enunciado:

   ```
   velocidad = (bytes * 8) / (2 * RTT_promedio)
   ```

   donde el factor 2 corresponde al RTT del handshake más el RTT de la
   transferencia.

## Estructura

| Archivo | Contenido |
|---|---|
| `main.c` | Flujo principal: handshake, 10 envíos, cierre y resultados |
| `app/speed_test.c` / `app/speed_test.h` | Funciones auxiliares: checksum, armado y envío de segmentos, recepción con filtrado |
| `Makefile` | Compilación |

## Notas y limitaciones

- No se implementan retransmisiones, control de congestión ni opciones TCP;
  si un segmento no llega en 2 segundos el programa aborta.
- Con archivos de 1 KB el tiempo medido está dominado por el RTT, por lo que
  la velocidad calculada es una aproximación.
- El programa debe ejecutarse con `sudo` por el uso de sockets RAW.