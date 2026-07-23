import java.io.*;
import java.util.*;
import java.nio.file.*;
import java.util.stream.*;
import java.util.logging.*;

public class Response {
    private static final Map<String, String> MIME_TYPES = new HashMap<String, String>() {{
        put("html", "text/html; charset=UTF-8");
        put("htm", "text/html; charset=UTF-8");
        put("css", "text/css; charset=UTF-8");
        put("js", "application/javascript; charset=UTF-8");
        put("json", "application/json; charset=UTF-8");
        put("png", "image/png");
        put("jpg", "image/jpeg");
        put("jpeg", "image/jpeg");
        put("gif", "image/gif");
        put("svg", "image/svg+xml");
        put("ico", "image/x-icon");
        put("woff", "font/woff");
        put("woff2", "font/woff2");
        put("ttf", "font/ttf");
        put("eot", "application/vnd.ms-fontobject");
        put("otf", "font/otf");
        put("txt", "text/plain; charset=UTF-8");
        put("cc8", "text/html; charset=UTF-8");
    }};

    public void sendData(Logger LOGGER, PrintStream dataOut, Integer nThreadServer, Object request) throws Exception {
        if (!(request instanceof HttpRequest)) {
            sendError(dataOut, 500, "Internal Server Error: Invalid request object", "text/plain; charset=UTF-8");
            return;
        }

        HttpRequest req = (HttpRequest) request;
        String method = req.method;
        String path = req.path;

        if (!"GET".equals(method) && !"POST".equals(method)) {
            sendError(dataOut, 405, "<html><body><h1>405 Method Not Allowed</h1></body></html>", "text/html; charset=UTF-8");
            return;
        }

        // Normalize path
        if (path == null || path.isEmpty() || "/".equals(path)) {
            path = "/index.html";
        }

        // Remove query string
        int qIdx = path.indexOf('?');
        if (qIdx >= 0) {
            path = path.substring(0, qIdx);
        }

        // Resolve file path under www/
        Path wwwRoot = Paths.get("www").toAbsolutePath().normalize();
        Path filePath = wwwRoot.resolve(path.substring(1)).normalize();

        // Security: prevent path traversal
        if (!filePath.startsWith(wwwRoot)) {
            sendError(dataOut, 404, "<html><body><h1>404 Not Found</h1></body></html>", "text/html; charset=UTF-8");
            return;
        }

        // If path ends with / or is a directory, try index.html
        if (path.endsWith("/") || (Files.exists(filePath) && Files.isDirectory(filePath))) {
            filePath = filePath.resolve("index.html").normalize();
            
            // Security check again
            if (!filePath.startsWith(wwwRoot)) {
                sendError(dataOut, 404, "<html><body><h1>404 Not Found</h1></body></html>", "text/html; charset=UTF-8");
                return;
            }
        }

        // Check if file exists
        if (!Files.exists(filePath)) {
            sendError(dataOut, 404, "<html><body><h1>404 Not Found</h1></body></html>", "text/html; charset=UTF-8");
            return;
        }

        String mimeType = getMimeType(filePath);

        // Handle .cc8 template files
        if (filePath.toString().endsWith(".cc8")) {
            // Merge queryParams and formParams for template
            Map<String, String> templateParams = new HashMap<String, String>();
            if (req.queryParams != null) templateParams.putAll(req.queryParams);
            if (req.formParams != null) templateParams.putAll(req.formParams);
            serveTemplate(dataOut, filePath, templateParams, mimeType);
        } else {
            serveStaticFile(dataOut, filePath, mimeType);
        }
    }

    private void serveTemplate(PrintStream dataOut, Path filePath, Map<String, String> params, String mimeType) throws Exception {
        String template = new String(Files.readAllBytes(filePath), "UTF-8");

        String name = params != null ? params.get("name") : null;
        String email = params != null ? params.get("email") : null;

        if (name == null) name = "";
        if (email == null) email = "";

        template = template.replace("{name}", htmlEscape(name));
        template = template.replace("{email}", htmlEscape(email));

        byte[] bodyBytes = template.getBytes("UTF-8");

        dataOut.println("HTTP/1.1 200 OK");
        dataOut.println("Content-Type: " + mimeType);
        dataOut.println("Content-Length: " + bodyBytes.length);
        dataOut.println("Connection: close");
        dataOut.println();
        dataOut.write(bodyBytes);
        dataOut.flush();
    }

    private void serveStaticFile(PrintStream dataOut, Path filePath, String mimeType) throws Exception {
        byte[] fileBytes = Files.readAllBytes(filePath);

        dataOut.println("HTTP/1.1 200 OK");
        dataOut.println("Content-Type: " + mimeType);
        dataOut.println("Content-Length: " + fileBytes.length);
        dataOut.println("Connection: close");
        dataOut.println();
        dataOut.write(fileBytes);
        dataOut.flush();
    }

    private void sendError(PrintStream dataOut, int code, String body, String mimeType) throws Exception {
        byte[] bodyBytes = body.getBytes("UTF-8");
        dataOut.println("HTTP/1.1 " + code + " " + getStatusText(code));
        dataOut.println("Content-Type: " + mimeType);
        dataOut.println("Content-Length: " + bodyBytes.length);
        dataOut.println("Connection: close");
        dataOut.println();
        dataOut.write(bodyBytes);
        dataOut.flush();
    }

    private String getMimeType(Path path) {
        String fileName = path.getFileName().toString();
        int dotIdx = fileName.lastIndexOf('.');
        if (dotIdx > 0) {
            String ext = fileName.substring(dotIdx + 1).toLowerCase();
            return MIME_TYPES.getOrDefault(ext, "application/octet-stream");
        }
        return "application/octet-stream";
    }

    private String getStatusText(int code) {
        switch (code) {
            case 200: return "OK";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 500: return "Internal Server Error";
            default: return "Unknown";
        }
    }

    private String htmlEscape(String s) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&"); break;
                case '<': sb.append("<"); break;
                case '>': sb.append(">"); break;
                case '"': sb.append("\""); break;
                case '\'': sb.append("'"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }
}