import java.io.*;
import java.net.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Servidor {

    private static final Pattern PATRON_OPERACION = Pattern.compile(
            "^\\s*(-?\\d+(?:\\.\\d+)?)\\s*([+\\-*/%])\\s*(-?\\d+(?:\\.\\d+)?)\\s*$");
    private static final DateTimeFormatter FORMATO_TIMESTAMP = DateTimeFormatter.ofPattern("dd-MM-yyyy HH:mm:ss");

    private String protocolo;
    private int puerto;

    public Servidor(String protocolo, int puerto) {
        this.protocolo = protocolo;
        this.puerto = puerto;
    }

    private static void registrarLog(String simbolo, String ip, String rol, String protocolo, String mensaje) {
        String timestamp = LocalDateTime.now().format(FORMATO_TIMESTAMP);
        System.out.println(simbolo + " " + ip + " " + rol + " " + timestamp + " " + protocolo + ": " + mensaje);
    }

    public String procesarOperacion(String entrada) {
        Matcher matcher = PATRON_OPERACION.matcher(entrada);
        if (!matcher.matches()) {
            return "not possible to compute";
        }

        double numero1 = Double.parseDouble(matcher.group(1));
        String operando = matcher.group(2);
        double numero2 = Double.parseDouble(matcher.group(3));
        double resultado;

        switch (operando) {
            case "+":
                resultado = numero1 + numero2;
                break;
            case "-":
                resultado = numero1 - numero2;
                break;
            case "*":
                resultado = numero1 * numero2;
                break;
            case "/":
                if (numero2 == 0) {
                    return "not possible to compute";
                }
                resultado = numero1 / numero2;
                break;
            case "%":
                if (numero2 == 0) {
                    return "not possible to compute";
                }
                resultado = numero1 % numero2;
                break;
            default:
                return "not possible to compute";
        }

        if (resultado == Math.floor(resultado) && !Double.isInfinite(resultado)) {
            return String.valueOf((long) resultado);
        }
        return String.valueOf(resultado);
    }

    public void iniciarTCP() throws IOException {
        String ipServidorLocal = InetAddress.getLocalHost().getHostAddress();

        try (ServerSocket servidorSocket = new ServerSocket(puerto)) {
            System.out.println("Servidor TCP escuchando en el puerto " + puerto);

            try (Socket socket = servidorSocket.accept();
                 BufferedReader entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                 PrintWriter salida = new PrintWriter(socket.getOutputStream(), true)) {

                String ipCliente = socket.getInetAddress().getHostAddress();
                System.out.println("Cliente conectado: " + ipCliente);

                String mensaje;
                while ((mensaje = entrada.readLine()) != null) {
                    registrarLog(">", ipCliente, "client", protocolo, mensaje);

                    if (mensaje.equalsIgnoreCase("EXIT")) {
                        salida.println("EXIT");
                        registrarLog("<", ipServidorLocal, "server", protocolo, "EXIT");
                        break;
                    }

                    String respuesta = procesarOperacion(mensaje);
                    salida.println(respuesta);
                    registrarLog("<", ipServidorLocal, "server", protocolo, respuesta);
                }
            }
        }
    }

    public void iniciarUDP() throws IOException {
        String ipServidorLocal = InetAddress.getLocalHost().getHostAddress();

        try (DatagramSocket socket = new DatagramSocket(puerto)) {
            System.out.println("Servidor UDP escuchando en el puerto " + puerto);

            while (true) {
                byte[] datosRecibo = new byte[1024];
                DatagramPacket paqueteRecibo = new DatagramPacket(datosRecibo, datosRecibo.length);
                socket.receive(paqueteRecibo);

                String mensaje = new String(paqueteRecibo.getData(), 0, paqueteRecibo.getLength());
                String ipCliente = paqueteRecibo.getAddress().getHostAddress();
                registrarLog(">", ipCliente, "client", protocolo, mensaje);

                InetAddress direccionCliente = paqueteRecibo.getAddress();
                int puertoCliente = paqueteRecibo.getPort();

                if (mensaje.equalsIgnoreCase("EXIT")) {
                    byte[] datosSalida = "EXIT".getBytes();
                    DatagramPacket paqueteSalida = new DatagramPacket(datosSalida, datosSalida.length, direccionCliente, puertoCliente);
                    socket.send(paqueteSalida);
                    registrarLog("<", ipServidorLocal, "server", protocolo, "EXIT");
                    break;
                }

                String respuesta = procesarOperacion(mensaje);
                byte[] datosEnvio = respuesta.getBytes();
                DatagramPacket paqueteEnvio = new DatagramPacket(datosEnvio, datosEnvio.length, direccionCliente, puertoCliente);
                socket.send(paqueteEnvio);
                registrarLog("<", ipServidorLocal, "server", protocolo, respuesta);
            }
        }
    }

    public static void main(String args[]) throws IOException {
        String protocolo = null;
        Integer puerto = null;

        for (int i = 0; i + 1 < args.length; i += 2) {
            switch (args[i].toLowerCase()) {
                case "protocol":
                    protocolo = args[i + 1];
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

        if (protocolo == null || puerto == null) {
            System.out.println("Uso: java Servidor protocol [TCP/UDP] port [PUERTO]");
            return;
        }

        Servidor servidor = new Servidor(protocolo, puerto);

        if (protocolo.equalsIgnoreCase("TCP")) {
            servidor.iniciarTCP();
        } else if (protocolo.equalsIgnoreCase("UDP")) {
            servidor.iniciarUDP();
        } else {
            System.out.println("Protocolo no reconocido: " + protocolo);
        }
    }
}
