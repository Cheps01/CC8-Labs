import java.io.ByteArrayOutputStream;
import java.util.HashMap;
import java.util.Map;

public class DnsHandler {

    public static final int TYPE_A = 1;
    public static final int TYPE_AAAA = 28;
    public static final int TYPE_PTR = 5;
    public static final int CLASS_IN = 1;

    private final Map<String, byte[]> records = new HashMap<>();

    public DnsHandler() {
        records.put("localhost", new byte[]{(byte) 127, 0, 0, 1});
        records.put("www.example.com", new byte[]{(byte) 93, (byte) 184, (byte) 216, 34});
        records.put("example.com", new byte[]{(byte) 93, (byte) 184, (byte) 216, 34});
        records.put("1.0.0.127.in-addr.arpa", new byte[]{'l', 'o', 'c', 'a', 'l', 'h', 'o', 's', 't'});
    }

    public DnsQuery parseRequest(byte[] request) {
        if (request == null || request.length < 12) {
            throw new IllegalArgumentException("DNS request is too short");
        }

        int offset = 0;
        int transactionId = readUnsignedShort(request, offset);
        offset += 2;

        int flags = readUnsignedShort(request, offset);
        offset += 2;

        int questionCount = readUnsignedShort(request, offset);
        offset += 2;

        int answerCount = readUnsignedShort(request, offset);
        offset += 2;
        int authorityCount = readUnsignedShort(request, offset);
        offset += 2;
        int additionalCount = readUnsignedShort(request, offset);
        offset += 2;

        if (questionCount < 1) {
            throw new IllegalArgumentException("DNS request does not contain any questions");
        }

        int[] nextOffset = new int[1];
        String questionName = readDomainName(request, offset, nextOffset);
        offset = nextOffset[0];

        int questionType = readUnsignedShort(request, offset);
        offset += 2;

        int questionClass = readUnsignedShort(request, offset);

        if (answerCount != 0 || authorityCount != 0 || additionalCount != 0) {
            // The parser accepts the request but does not need the extra sections for a basic implementation.
        }

        return new DnsQuery(transactionId, flags, questionCount, questionName, questionType, questionClass);
    }

    public byte[] createResponse(byte[] request) {
        DnsQuery query;
        try {
            query = parseRequest(request);
        } catch (IllegalArgumentException ex) {
            return createErrorResponse(request, ex.getMessage());
        }

        ByteArrayOutputStream output = new ByteArrayOutputStream();

        writeUnsignedShort(output, query.transactionId);
        writeUnsignedShort(output, 0x8180); // Standard response + recursion available
        writeUnsignedShort(output, 1);
        writeUnsignedShort(output, 1);
        writeUnsignedShort(output, 0);
        writeUnsignedShort(output, 0);

        writeDomainName(output, query.questionName);
        writeUnsignedShort(output, query.questionType);
        writeUnsignedShort(output, query.questionClass);

        writeAnswer(output, query);

        return output.toByteArray();
    }

    private byte[] createErrorResponse(byte[] request, String message) {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        int transactionId = request != null && request.length >= 2 ? readUnsignedShort(request, 0) : 0;
        writeUnsignedShort(output, transactionId);
        writeUnsignedShort(output, 0x8183); // Format error / server failure depending on the error
        writeUnsignedShort(output, 1);
        writeUnsignedShort(output, 0);
        writeUnsignedShort(output, 0);
        writeUnsignedShort(output, 0);
        return output.toByteArray();
    }

    private void writeAnswer(ByteArrayOutputStream output, DnsQuery query) {
        byte[] rdata = resolveRdata(query.questionName, query.questionType);
        if (rdata == null) {
            return;
        }

        writeDomainName(output, query.questionName);
        writeUnsignedShort(output, query.questionType);
        writeUnsignedShort(output, query.questionClass);
        writeUnsignedInt(output, 60);

        if (query.questionType == TYPE_A) {
            writeUnsignedShort(output, 4);
            output.write(rdata, 0, rdata.length);
        } else if (query.questionType == TYPE_AAAA) {
            writeUnsignedShort(output, 16);
            output.write(rdata, 0, rdata.length);
        } else if (query.questionType == TYPE_PTR) {
            byte[] ptrName = encodeDomainName(new String(rdata, java.nio.charset.StandardCharsets.UTF_8));
            writeUnsignedShort(output, ptrName.length);
            output.write(ptrName, 0, ptrName.length);
        }
    }

    private byte[] resolveRdata(String questionName, int questionType) {
        String normalized = questionName.toLowerCase();
        if (questionType == TYPE_A) {
            if (normalized.equals("localhost")) {
                return new byte[]{127, 0, 0, 1};
            }
            if (normalized.equals("www.example.com") || normalized.equals("example.com")) {
                return new byte[]{(byte) 93, (byte) 184, (byte) 216, 34};
            }
        }

        if (questionType == TYPE_AAAA) {
            if (normalized.equals("localhost")) {
                return new byte[]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
            }
        }

        if (questionType == TYPE_PTR) {
            if (normalized.equals("1.0.0.127.in-addr.arpa")) {
                return "localhost".getBytes(java.nio.charset.StandardCharsets.UTF_8);
            }
        }

        return null;
    }

    private void writeDomainName(ByteArrayOutputStream output, String name) {
        byte[] encoded = encodeDomainName(name);
        output.write(encoded, 0, encoded.length);
    }

    private byte[] encodeDomainName(String name) {
        if (name == null || name.isEmpty()) {
            return new byte[]{0};
        }

        String normalized = name.replaceAll("\\.$", "").toLowerCase();
        if (normalized.isEmpty()) {
            return new byte[]{0};
        }

        String[] labels = normalized.split("\\.");
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        for (String label : labels) {
            if (label.isEmpty()) {
                continue;
            }
            output.write(label.length());
            for (int i = 0; i < label.length(); i++) {
                output.write(label.charAt(i));
            }
        }
        output.write(0);
        return output.toByteArray();
    }

    private void writeUnsignedShort(ByteArrayOutputStream output, int value) {
        output.write((value >>> 8) & 0xFF);
        output.write(value & 0xFF);
    }

    private void writeUnsignedInt(ByteArrayOutputStream output, long value) {
        output.write((int) ((value >>> 24) & 0xFF));
        output.write((int) ((value >>> 16) & 0xFF));
        output.write((int) ((value >>> 8) & 0xFF));
        output.write((int) (value & 0xFF));
    }

    private int readUnsignedShort(byte[] data, int offset) {
        if (offset + 1 >= data.length) {
            throw new IllegalArgumentException("Truncated DNS message");
        }
        return ((data[offset] & 0xFF) << 8) | (data[offset + 1] & 0xFF);
    }

    private String readDomainName(byte[] data, int offset, int[] nextOffset) {
        StringBuilder name = new StringBuilder();
        int currentOffset = offset;
        boolean jumped = false;
        int jumpOffset = -1;

        while (true) {
            if (currentOffset >= data.length) {
                throw new IllegalArgumentException("Truncated DNS name");
            }

            int length = data[currentOffset] & 0xFF;
            if (length == 0) {
                currentOffset++;
                break;
            }

            if ((length & 0xC0) == 0xC0) {
                if (currentOffset + 1 >= data.length) {
                    throw new IllegalArgumentException("Truncated DNS pointer");
                }
                int pointer = ((length & 0x3F) << 8) | (data[currentOffset + 1] & 0xFF);
                if (jumpOffset == -1) {
                    jumpOffset = currentOffset + 2;
                }
                currentOffset = pointer;
                jumped = true;
                continue;
            }

            if ((length & 0xC0) != 0) {
                throw new IllegalArgumentException("Unsupported DNS label type");
            }

            currentOffset++;
            if (name.length() > 0) {
                name.append('.');
            }
            for (int i = 0; i < length; i++) {
                if (currentOffset + i >= data.length) {
                    throw new IllegalArgumentException("Truncated DNS label");
                }
                name.append((char) (data[currentOffset + i] & 0xFF));
            }
            currentOffset += length;
        }

        nextOffset[0] = jumped ? jumpOffset : currentOffset;
        return name.toString();
    }
}