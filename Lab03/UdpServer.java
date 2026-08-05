import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Arrays;

public class UdpServer {

    private final int PORT;
    private final DnsHandler dnsHandler;

    public UdpServer(int port) {
        this.PORT = port;
        this.dnsHandler = new DnsHandler();
    }

    public void start() throws IOException {
        System.out.println("DNS UDP server running on port: " + PORT);

        try (DatagramSocket socket = new DatagramSocket(PORT)) {
            byte[] buffer = new byte[512];

            while (true) {
                DatagramPacket incomingPacket = new DatagramPacket(buffer, buffer.length);
                socket.receive(incomingPacket);

                byte[] requestData = Arrays.copyOf(incomingPacket.getData(), incomingPacket.getLength());
                InetAddress clientAddress = incomingPacket.getAddress();
                int clientPort = incomingPacket.getPort();

                byte[] responseData;
                try {
                    DnsQuery query = dnsHandler.parseRequest(requestData);
                    System.out.printf("Received DNS request from %s:%d - name=%s type=%s class=%d length=%d%n",
                            clientAddress.getHostAddress(),
                            clientPort,
                            query.questionName,
                            query.getQuestionTypeName(),
                            query.questionClass,
                            requestData.length);
                    responseData = dnsHandler.createResponse(requestData);
                } catch (Exception ex) {
                    System.out.printf("Received invalid DNS request from %s:%d length=%d error=%s%n",
                            clientAddress.getHostAddress(),
                            clientPort,
                            requestData.length,
                            ex.getMessage());
                    responseData = dnsHandler.createResponse(requestData);
                }

                System.out.printf("Sending DNS response to %s:%d length=%d%n",
                        clientAddress.getHostAddress(),
                        clientPort,
                        responseData.length);

                DatagramPacket outgoingPackage = new DatagramPacket(
                        responseData,
                        responseData.length,
                        clientAddress,
                        clientPort
                );

                socket.send(outgoingPackage);
            }
        }
    }

    public static void main(String[] args) throws IOException {
        Integer port = null;

        for (int i = 0; i + 1 < args.length; i += 2) {
            switch (args[i].toLowerCase()) {
                case "port":
                    try {
                        port = Integer.parseInt(args[i + 1]);
                    } catch (NumberFormatException e) {
                        port = null;
                    }
                    break;
            }
        }

        if (port == null) {
            System.out.println("Usage: java UdpServer port [PORT]");
            return;
        }

        new UdpServer(port).start();
    }
}
