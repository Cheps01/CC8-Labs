import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.io.ByteArrayOutputStream;

public class TestDnsClient {
    private static byte[] encodeName(String name) throws Exception {
        String[] labels = name.split("\\.");
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        for (String l : labels) {
            out.write(l.length());
            out.write(l.getBytes("UTF-8"));
        }
        out.write(0);
        return out.toByteArray();
    }

    public static void main(String[] args) throws Exception {
        String host = "127.0.0.1";
        int port = 5300;
        String name = "example.com";

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(0x12); out.write(0x34); // transaction ID
        out.write(0x01); out.write(0x00); // flags: standard query
        out.write(0x00); out.write(0x01); // QDCOUNT = 1
        out.write(0x00); out.write(0x00); // ANCOUNT
        out.write(0x00); out.write(0x00); // NSCOUNT
        out.write(0x00); out.write(0x00); // ARCOUNT

        out.write(encodeName(name));
        out.write(0x00); out.write(0x01); // QTYPE A
        out.write(0x00); out.write(0x01); // QCLASS IN

        byte[] req = out.toByteArray();

        try (DatagramSocket socket = new DatagramSocket()) {
            DatagramPacket p = new DatagramPacket(req, req.length, InetAddress.getByName(host), port);
            socket.send(p);

            byte[] buf = new byte[512];
            DatagramPacket resp = new DatagramPacket(buf, buf.length);
            socket.setSoTimeout(2000);
            socket.receive(resp);

            int len = resp.getLength();
            byte[] data = resp.getData();
            System.out.println("Received " + len + " bytes:");
            for (int i = 0; i < len; i++) {
                System.out.printf("%02X ", data[i]);
                if ((i+1)%16 == 0) System.out.println();
            }
            System.out.println();
        } catch (Exception e) {
            System.err.println("No response: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
