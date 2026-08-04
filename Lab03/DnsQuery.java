public class DnsQuery {
    public final int transactionId;
    public final int flags;
    public final int questionCount;
    public final String questionName;
    public final int questionType;
    public final int questionClass;

    public DnsQuery(int transactionId, int flags, int questionCount,
                    String questionName, int questionType, int questionClass) {
        this.transactionId = transactionId;
        this.flags = flags;
        this.questionCount = questionCount;
        this.questionName = questionName;
        this.questionType = questionType;
        this.questionClass = questionClass;
    }

    public String getQuestionTypeName() {
        switch (questionType) {
            case 1:
                return "A";
            case 28:
                return "AAAA";
            case 5:
                return "PTR";
            default:
                return "TYPE(" + questionType + ")";
        }
    }
}
