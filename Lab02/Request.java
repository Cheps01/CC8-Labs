import java.io.*;
import java.util.*;
import java.util.stream.*;
import java.util.logging.*;

public class Request {
    public Object getData(Logger LOGGER, BufferedReader dataIn, Integer nThreadServer) throws Exception {
        HttpRequest request = new HttpRequest();
        
        // Read request line
        String requestLine = dataIn.readLine();
        if (requestLine == null || requestLine.trim().isEmpty()) {
            return request;
        }
        
        String[] requestParts = requestLine.split(" ", 3);
        request.method = requestParts[0];
        request.path = requestParts.length > 1 ? requestParts[1] : "/";
        request.version = requestParts.length > 2 ? requestParts[2] : "HTTP/1.1";
        
        // Parse query string from path
        int qIdx = request.path.indexOf('?');
        if (qIdx >= 0) {
            String queryString = request.path.substring(qIdx + 1);
            request.path = request.path.substring(0, qIdx);
            parseQueryString(queryString, request.queryParams);
        }
        
        // Read headers
        String line;
        int contentLength = 0;
        while ((line = dataIn.readLine()) != null) {
            if (line.trim().isEmpty()) break;
            
            int colonIdx = line.indexOf(':');
            if (colonIdx > 0) {
                String headerName = line.substring(0, colonIdx).trim().toLowerCase();
                String headerValue = line.substring(colonIdx + 1).trim();
                request.headers.put(headerName, headerValue);
                
                if ("content-length".equals(headerName)) {
                    try {
                        contentLength = Integer.parseInt(headerValue);
                    } catch (NumberFormatException e) {
                        contentLength = 0;
                    }
                } else if ("content-type".equals(headerName)) {
                    request.contentType = headerValue;
                }
            }
        }
        
        // Read body
        if (contentLength > 0) {
            char[] buffer = new char[contentLength];
            int read = dataIn.read(buffer, 0, contentLength);
            request.rawBody = new String(buffer, 0, read);
            
            // Parse body based on content type
            if (request.contentType != null) {
                String ct = request.contentType.toLowerCase();
                if (ct.startsWith("application/x-www-form-urlencoded")) {
                    parseQueryString(request.rawBody, request.formParams);
                } else if (ct.startsWith("application/json")) {
                    parseJson(request.rawBody, request.formParams);
                } else if (ct.startsWith("multipart/form-data")) {
                    // Simple multipart parsing - just extract name=value pairs
                    parseMultipart(request.rawBody, request.contentType, request.formParams);
                } else if (ct.startsWith("text/plain")) {
                    // For text/plain, just store as raw body
                    request.formParams.put("raw", request.rawBody);
                }
            }
        }
        
        LOGGER.info("(" + nThreadServer + ") request: " + request.method + " " + request.path + " " + request.version);
        return request;
    }
    
    private void parseQueryString(String queryString, Map<String, String> params) {
        if (queryString == null || queryString.isEmpty()) return;
        
        String[] pairs = queryString.split("&");
        for (String pair : pairs) {
            int eqIdx = pair.indexOf('=');
            if (eqIdx >= 0) {
                String key = urlDecode(pair.substring(0, eqIdx));
                String value = urlDecode(pair.substring(eqIdx + 1));
                params.put(key, value);
            } else {
                params.put(urlDecode(pair), "");
            }
        }
    }
    
    private String urlDecode(String s) {
        try {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                if (c == '%') {
                    if (i + 2 < s.length()) {
                        String hex = s.substring(i + 1, i + 3);
                        sb.append((char) Integer.parseInt(hex, 16));
                        i += 2;
                    } else {
                        sb.append(c);
                    }
                } else if (c == '+') {
                    sb.append(' ');
                } else {
                    sb.append(c);
                }
            }
            return sb.toString();
        } catch (Exception e) {
            return s;
        }
    }
    
    private void parseJson(String json, Map<String, String> params) {
        // Simple JSON parser for {"key":"value"} format
        if (json == null || json.isEmpty()) return;
        
        String trimmed = json.trim();
        if (!trimmed.startsWith("{") || !trimmed.endsWith("}")) return;
        
        String content = trimmed.substring(1, trimmed.length() - 1).trim();
        if (content.isEmpty()) return;
        
        // Split by comma but respect quoted strings
        List<String> pairs = splitJsonPairs(content);
        for (String pair : pairs) {
            int colonIdx = pair.indexOf(':');
            if (colonIdx > 0) {
                String key = pair.substring(0, colonIdx).trim();
                String value = pair.substring(colonIdx + 1).trim();
                
                // Remove surrounding quotes
                key = unquote(key);
                value = unquote(value);
                
                params.put(key, value);
            }
        }
    }
    
    private List<String> splitJsonPairs(String content) {
        List<String> pairs = new ArrayList<String>();
        StringBuilder current = new StringBuilder();
        boolean inQuotes = false;
        char quoteChar = 0;
        
        for (int i = 0; i < content.length(); i++) {
            char c = content.charAt(i);
            
            if (!inQuotes && (c == '"' || c == '\'')) {
                inQuotes = true;
                quoteChar = c;
            } else if (inQuotes && c == quoteChar && (i == 0 || content.charAt(i - 1) != '\\')) {
                inQuotes = false;
                quoteChar = 0;
            }
            
            if (!inQuotes && c == ',') {
                pairs.add(current.toString());
                current = new StringBuilder();
            } else {
                current.append(c);
            }
        }
        
        if (current.length() > 0) {
            pairs.add(current.toString());
        }
        
        return pairs;
    }
    
    private String unquote(String s) {
        s = s.trim();
        if (s.length() >= 2) {
            char first = s.charAt(0);
            char last = s.charAt(s.length() - 1);
            if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
                return s.substring(1, s.length() - 1);
            }
        }
        return s;
    }
    
    private void parseMultipart(String body, String contentType, Map<String, String> params) {
        // Very simple multipart parsing - extract name="value" pairs
        // This is a minimal implementation for the test forms
        if (body == null) return;
        
        // Find boundary
        String boundary = null;
        int boundaryIdx = contentType.indexOf("boundary=");
        if (boundaryIdx >= 0) {
            boundary = contentType.substring(boundaryIdx + 9).trim();
        }
        
        if (boundary == null) return;
        
        String delimiter = "--" + boundary;
        String[] parts = body.split(delimiter);
        
        for (String part : parts) {
            part = part.trim();
            if (part.isEmpty() || part.equals("--") || part.equals("--\r\n")) continue;
            
            // Look for Content-Disposition header with name
            int nameStart = part.indexOf("name=\"");
            if (nameStart >= 0) {
                nameStart += 6;
                int nameEnd = part.indexOf("\"", nameStart);
                if (nameEnd > nameStart) {
                    String fieldName = part.substring(nameStart, nameEnd);
                    
                    // Find the body (after double newline)
                    int bodyStart = part.indexOf("\r\n\r\n");
                    if (bodyStart < 0) bodyStart = part.indexOf("\n\n");
                    if (bodyStart >= 0) {
                        String fieldValue = part.substring(bodyStart + 4).trim();
                        // Remove trailing boundary markers
                        if (fieldValue.endsWith("\r\n")) fieldValue = fieldValue.substring(0, fieldValue.length() - 2);
                        if (fieldValue.endsWith("\n")) fieldValue = fieldValue.substring(0, fieldValue.length() - 1);
                        
                        params.put(fieldName, fieldValue);
                    }
                }
            }
        }
    }
}