import java.io.IOException;
import java.nio.file.Path;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Formatter;
import java.util.logging.LogRecord;
import java.util.logging.Logger;

public class DnsLogger {
    public static Logger createLogger(int threadId, Path logsDir) throws IOException {
        Logger logger = Logger.getLogger("DnsThread-" + threadId + "-" + System.currentTimeMillis());
        logger.setUseParentHandlers(false);

        SimpleDateFormat df = new SimpleDateFormat("yyMMddHHmmss");
        String ts = df.format(new Date());
        String filename = logsDir.resolve("T" + threadId + "-" + ts + ".log").toString();

        FileHandler fh = new FileHandler(filename, true);
        fh.setFormatter(new Formatter() {
            @Override
            public String format(LogRecord record) {
                Date d = new Date(record.getMillis());
                String ts = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS").format(d);
                return String.format("%s %s %s%n", ts, record.getLevel(), record.getMessage());
            }
        });
        ConsoleHandler ch = new ConsoleHandler();
        ch.setFormatter(fh.getFormatter());

        logger.addHandler(fh);
        logger.addHandler(ch);

        return logger;
    }
}
