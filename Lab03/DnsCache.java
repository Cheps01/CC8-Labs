import java.util.concurrent.ConcurrentHashMap;

public class DnsCache {
    private static final class CachedResponse {
        final byte[] response;
        final long expiryMillis;

        CachedResponse(byte[] response, long expiryMillis) {
            this.response = response;
            this.expiryMillis = expiryMillis;
        }
    }

    private final ConcurrentHashMap<String, CachedResponse> cache = new ConcurrentHashMap<>();

    public void put(String key, byte[] response, int ttlSeconds) {
        long expiry = System.currentTimeMillis() + (long) ttlSeconds * 1000L;
        cache.put(key, new CachedResponse(response.clone(), expiry));
    }

    public byte[] get(String key) {
        CachedResponse cr = cache.get(key);
        if (cr == null) return null;
        if (System.currentTimeMillis() > cr.expiryMillis) {
            cache.remove(key);
            return null;
        }
        return cr.response.clone();
    }

    public void clear() {
        cache.clear();
    }
}
