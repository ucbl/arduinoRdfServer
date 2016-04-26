class Coap{
  public:
    Coap();
    char buffer[UDP_TX_PACKET_MAX_SIZE];
    int packetCursor;
    int payloadCursor;
    typedef struct{
      String path;
      const char* const_payload;
    } uripayload_t;
    uripayload_t uripayload[5];
    int tokenLength=0;
    int uri[2]={0,0};
    String path="/";
    int optDelta=0;
    // Request
    int method=0;
    int payloadStartIndex=0;
    String request="";
    int const_payload_index=-1;
    int payloadLen = 0;
    // Blocks
    int blockIndex=0;
    
    void resetVariables();
    void sendBuffer(int sendSize);
    void writeCode(char code);
    void writeType(char type);
    boolean readOptionDesc();
    void readPayload();
    void parseHeader();
    void writeHeader(int type, int block, int code, int blockNum, int payloadLength);
    void writePayload(const char* payloadStartIndex, int payloadLength);
    int analyzeBlock();
  private:
}
