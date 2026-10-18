import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Server {
    private final int port;
    private ServerSocket serverSocket;

    // Constructor: stores the port number
    public Server(int port) {
        this.port = port;
    }

    // Logs server activity with a timestamp
    public void log(String message) {
        String timestamp = LocalDateTime.now().format(
            DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")
        );
        System.out.println("[" + timestamp + "] " + message);
    }

    // Initializes the server and accepts client connections
    public void start() {
        try {
            serverSocket = new ServerSocket(port);
            log("TCP server is open on port " + port);

            boolean running = true;

            while (running) {
                // Wait for a client to connect
                Socket clientSocket = serverSocket.accept();

                log("Connection established with "
                    + clientSocket.getInetAddress().getHostAddress());

                // Read messages until EXIT, OFF, or disconnection
                boolean clientConnected = true;

                try (Socket client = clientSocket;
                     BufferedReader reader = new BufferedReader(new InputStreamReader(client.getInputStream()))) {

                    while (clientConnected) {
                        String message = reader.readLine();

                        // Client disconnected or closed its output stream
                        if (message == null) {
                            log("Client disconnected.");
                            break;
                        }
                        log("Received message: " + message);

                        if (message.contains("OFF")) {
                            log("OFF received. Closing connection and stopping server.");
                            clientConnected = false;
                            running = false;
                        } else if (message.contains("EXIT")) {
                            log("EXIT received. Closing connection; server keeps listening.");
                            clientConnected = false;
                        }
                    }
                } catch (IOException e) {
                    log("Error communicating with client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            log("Server error: " + e.getMessage());
        } finally {
            if (serverSocket != null && !serverSocket.isClosed()) {
                try {
                    serverSocket.close();
                    log("Server socket closed.");
                } catch (IOException e) {
                    log("Error closing server: " + e.getMessage());
                }
            }
        }
        log("Server terminated.");
    }

    // Main method: parses the -p port option
    public static void main(String[] args) {
        int port = 5000;
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-p")) {
                if (i + 1 >= args.length) {
                    System.err.println("Error: -p requires a port number.");
                    return;
                }
                try {
                    port = Integer.parseInt(args[++i]);

                    if (port < 1 || port > 65535) {
                        System.err.println(
                            "Error: port must be between 1 and 65535."
                        );
                        return;
                    }
                } catch (NumberFormatException e) {
                    System.err.println("Error: invalid port number.");
                    return;
                }
            } else {
                System.err.println("Unknown option: " + args[i]);
                System.err.println("Usage: java Server [-p <port>]");
                return;
            }
        }
        Server server = new Server(port);
        server.start();
    }
}