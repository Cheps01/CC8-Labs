import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.logging.Logger;

public class UdpServer {

    private final int PORT;
    private final DnsHandler dnsHandler;
    private final List<Logger> loggers;
    private final int threads;
    private ThreadPoolExecutor pool;

    public UdpServer(int port, DnsHandler handler, List<Logger> loggers, int threads) {
        this.PORT = port;
        this.dnsHandler = handler;
        this.loggers = loggers;
        this.threads = Math.max(1, threads);
    }

    public void start() throws IOException {
        System.out.println("DNS UDP server running on port: " + PORT);
        if (loggers != null && !loggers.isEmpty() && loggers.get(0) != null)
            loggers.get(0).info("DNS UDP server running on port: " + PORT);

        try (DatagramSocket socket = new DatagramSocket(PORT)) {
            byte[] buffer = new byte[65535];
            BlockingQueue<PacketInfo> queue = new LinkedBlockingQueue<>();
            pool = (ThreadPoolExecutor) Executors.newFixedThreadPool(threads);

            for (int i = 0; i < threads; i++) {
                final int id = i;
                final Logger logger = (loggers != null && i < loggers.size()) ? loggers.get(i) : null;
                pool.execute(() -> {
                    while (!Thread.currentThread().isInterrupted()) {
                        try {
                            PacketInfo p = queue.take();
                            processPacket(p, socket, logger);
                        } catch (InterruptedException ie) {
                            Thread.currentThread().interrupt();
                            break;
                        } catch (Exception e) {
                            if (logger != null) logger.severe("Worker " + id + " error: " + e.getMessage());
                            else System.err.println("Worker " + id + " error: " + e.getMessage());
                        }
                    }
                });
            }

            while (true) {
                DatagramPacket incomingPacket = new DatagramPacket(buffer, buffer.length);
                socket.receive(incomingPacket);

                byte[] requestData = Arrays.copyOf(incomingPacket.getData(), incomingPacket.getLength());
                InetAddress clientAddress = incomingPacket.getAddress();
                int clientPort = incomingPacket.getPort();

                queue.put(new PacketInfo(requestData, clientAddress, clientPort));
            }
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
        }
    }

    private void processPacket(PacketInfo p, DatagramSocket socket, Logger logger) {
        try {
            DnsQuery query = dnsHandler.parseRequest(p.data);
            String recvMsg = String.format("Received DNS request from %s:%d - name=%s type=%s class=%d length=%d",
                    p.address.getHostAddress(),
                    p.port,
                    query.questionName,
                    query.getQuestionTypeName(),
                    query.questionClass,
                    p.data.length);
            System.out.println(recvMsg);
            if (logger != null) logger.info(recvMsg);

            byte[] responseData = dnsHandler.createResponse(p.data);

            String sendMsg = String.format("Sending DNS response to %s:%d length=%d",
                    p.address.getHostAddress(),
                    p.port,
                    responseData.length);
            System.out.println(sendMsg);
            if (logger != null) logger.info(sendMsg);

            DatagramPacket outgoingPackage = new DatagramPacket(
                    responseData,
                    responseData.length,
                    p.address,
                    p.port
            );

            socket.send(outgoingPackage);
        } catch (Exception ex) {
            String err = String.format("Error processing packet from %s:%d - %s",
                    p.address.getHostAddress(), p.port, ex.getMessage());
            System.err.println(err);
            if (logger != null) logger.severe(err);
        }
    }

    private static class PacketInfo {
        final byte[] data;
        final InetAddress address;
        final int port;

        PacketInfo(byte[] data, InetAddress address, int port) {
            this.data = data;
            this.address = address;
            this.port = port;
        }
    }

    public void shutdown() {
        if (pool != null) {
            pool.shutdownNow();
        }
    }
}
