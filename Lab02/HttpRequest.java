import java.util.*;

public class HttpRequest {
    public String method;
    public String path;
    public String version;
    public Map<String, String> headers = new HashMap<String, String>();
    public Map<String, String> queryParams = new HashMap<String, String>();
    public Map<String, String> formParams = new HashMap<String, String>();
    public String rawBody;
    public String contentType;
}