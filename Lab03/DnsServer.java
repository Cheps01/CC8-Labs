import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class DnsServer {
    public static void main(String[] args) throws Exception {
        int port = 5300;
        int threads = 4;
        int delay = 5;
        List<String> upstreams = new ArrayList<>();

        for (int i = 0; i + 1 < args.length; i += 2) {
            String k = args[i].toLowerCase();
            String v = args[i + 1];
            switch (k) {
                case "port":
                    port = Integer.parseInt(v);
                    break;
                case "threads":
                    threads = Integer.parseInt(v);
                    break;
                case "delay":
                    delay = Integer.parseInt(v);
                    break;
                case "upstream":
                    upstreams.add(v);
                    break;
                default:
                    System.out.println("Unknown arg: " + k);
            }
        }

        // Create logs directory
        Path logs = Path.of("logs");
        Files.createDirectories(logs);

        // Initialize cache (empty on startup)
        DnsCache cache = new DnsCache();

        System.out.println("Starting DnsServer");
        System.out.println("Port: " + port + " Threads: " + threads + " Delay: " + delay);
        if (!upstreams.isEmpty()) System.out.println("Upstreams: " + upstreams);

        // Create DnsHandler with cache and upstreams
        DnsHandler handler = new DnsHandler(cache, upstreams);

        // Create per-thread loggers (files) - create for configured threads
        java.nio.file.Path logsDir = java.nio.file.Path.of("logs");
        java.util.List<java.util.logging.Logger> loggers = new java.util.ArrayList<>();
        for (int i = 0; i < threads; i++) {
            try {
                loggers.add(DnsLogger.createLogger(i, logsDir));
            } catch (Exception e) {
                System.out.println("Failed to create logger for thread " + i + ": " + e.getMessage());
                loggers.add(null);
            }
        }

        // Start UDP server using the handler and per-thread loggers
        UdpServer server = new UdpServer(port, handler, loggers, threads);
        server.start();
    }
}
