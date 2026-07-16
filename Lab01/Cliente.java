import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Cliente {

    private static final DateTimeFormatter FORMATO_TIMESTAMP = DateTimeFormatter.ofPattern("dd-MM-yyyy HH:mm:ss");

    private String ipServidor;
    private String protocolo;
    private int puerto;

    public Cliente(String ipServidor, String protocolo, int puerto) {
        this.ipServidor = ipServidor;
        this.protocolo = protocolo;
        this.puerto = puerto;
    }

    private static void registrarLog(String simbolo, String ip, String rol, String protocolo, String mensaje) {
        String timestamp = LocalDateTime.now().format(FORMATO_TIMESTAMP);
        System.out.println(simbolo + " " + ip + " " + rol + " " + timestamp + " " + protocolo + ": " + mensaje);
    }

    public void conectarTCP() throws IOException {
        String ipCliente = InetAddress.getLocalHost().getHostAddress();

        try (Socket socket = new Socket(ipServidor, puerto);
             PrintWriter salida = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             BufferedReader teclado = new BufferedReader(new InputStreamReader(System.in))) {

            System.out.println("Conectado al servidor TCP " + ipServidor + ":" + puerto);

            String mensaje;
            while (true) {
                System.out.print("Ingrese operación a realizar: ");
                mensaje = teclado.readLine();
                if (mensaje == null) {
                    mensaje = "EXIT";
                }

                salida.println(mensaje);
                registrarLog("<", ipCliente, "client", protocolo, mensaje);

                String respuesta = entrada.readLine();
                if (respuesta == null) {
                    break;
                }
                registrarLog(">", ipServidor, "server", protocolo, respuesta);

                if (mensaje.equalsIgnoreCase("EXIT")) {
                    break;
                }
            }
        }
    }

    public void conectarUDP() throws IOException {
        String ipCliente = InetAddress.getLocalHost().getHostAddress();

        try (DatagramSocket socket = new DatagramSocket();
             BufferedReader teclado = new BufferedReader(new InputStreamReader(System.in))) {

            InetAddress direccionServidor = InetAddress.getByName(ipServidor);
            System.out.println("Cliente UDP listo para enviar a " + ipServidor + ":" + puerto);

            String mensaje;
            while (true) {
                System.out.print("Ingrese operación: ");
                mensaje = teclado.readLine();
                if (mensaje == null) {
                    mensaje = "EXIT";
                }

                byte[] datosEnvio = mensaje.getBytes();
                DatagramPacket paqueteEnvio = new DatagramPacket(datosEnvio, datosEnvio.length, direccionServidor, puerto);
                socket.send(paqueteEnvio);
                registrarLog("<", ipCliente, "client", protocolo, mensaje);

                byte[] datosRecibo = new byte[1024];
                DatagramPacket paqueteRecibo = new DatagramPacket(datosRecibo, datosRecibo.length);
                socket.receive(paqueteRecibo);
                String respuesta = new String(paqueteRecibo.getData(), 0, paqueteRecibo.getLength());
                registrarLog(">", ipServidor, "server", protocolo, respuesta);

                if (mensaje.equalsIgnoreCase("EXIT")) {
                    break;
                }
            }
        }
    }

    public static void main(String args[]) throws IOException {
        String protocolo = null;
        String ipServidor = null;
        Integer puerto = null;

        for (int i = 0; i + 1 < args.length; i += 2) {
            switch (args[i].toLowerCase()) {
                case "protocol":
                    protocolo = args[i + 1];
                    break;
                case "server":
                    ipServidor = args[i + 1];
                    break;
                case "port":
                    try {
                        puerto = Integer.parseInt(args[i + 1]);
                    } catch (NumberFormatException e) {
                        puerto = null;
                    }
                    break;
            }
        }

        if (protocolo == null || ipServidor == null || puerto == null) {
            System.out.println("Uso: java Cliente protocol [TCP/UDP] server [IP] port [PUERTO]");
            return;
        }

        Cliente cliente = new Cliente(ipServidor, protocolo, puerto);

        try {
            if (protocolo.equalsIgnoreCase("TCP")) {
                cliente.conectarTCP();
            } else if (protocolo.equalsIgnoreCase("UDP")) {
                cliente.conectarUDP();
            } else {
                System.out.println("Protocolo no reconocido: " + protocolo);
            }
        } catch (IOException e) {
            System.out.println("No se pudo conectar el cliente al servidor: " + e.getMessage());
        }
    }
}