class Token {

  public final static int LABEL = 0;
  public final static int WORD = 1;
  public final static int NUMBER = 2;
  public final static int APOSTROPHIZED = 3;
  public final static int HYPHENATED = 4;
  public final static int NEWLINE = 5;
  public final static int PUNCTUATION = 6;

  public int m_type;
  public String m_value;
  public int m_line;
  public int m_column;
  
  Token (int type, String value, int line, int column) {
    m_type = type;
    m_value = value;
    m_line = line;
    m_column = column;
  }

  public String toString() {
    switch (m_type) {
      case LABEL:
        return m_value;
      case WORD:
        return m_value;
      case NUMBER:
        return m_value;
      case APOSTROPHIZED:
        int parts = 0;
        int length = 0;
        int noLengthParts = 0;
        int indexOfLastApo = 0;
        boolean split = false;
        boolean firstSingle = false;
        boolean lastS = false;
        for(int i = 0; i < m_value.length(); i++) {
           if(m_value.charAt(i) == '\'') {
              parts++;
              if(parts == 1 && i == 1){
                firstSingle = true;
              }
              indexOfLastApo = i;
              if(parts > 3){
                split = true;
              }
              if(length == 0) noLengthParts++;
              length = 0;
           }
           else {
             length++;
           }
        }
        if(length == 0) noLengthParts++;
        if(m_value.substring(indexOfLastApo, m_value.length()).toLowerCase().contains("s")){
          lastS = true;
        }
        if(parts == 1) {
          if(indexOfLastApo == 1 && m_value.length() >= 4){
            split = false;
          }
          else if(m_value.substring(indexOfLastApo, m_value.length()).length() <= 3){
            split = false;
          }
          else{
            split = true;
          }
        }
        if(parts == 2 && !(firstSingle && lastS)) {
            split = true;
        }
        if(length > 3 || (parts == 1 && length == 0)) split = true;
        if(parts == noLengthParts) split = false;

        String returnValue;

        if(split){
          returnValue = m_value.replaceAll("'", " ' ");
        }
        else if(m_value.substring(indexOfLastApo, m_value.length()).length() <= 3 && !lastS){
          returnValue = m_value.substring(0, indexOfLastApo) + " " + m_value.substring(indexOfLastApo, m_value.length());
        }
        else{
          returnValue = m_value;
        }

        if(returnValue.charAt(0) == ' ') returnValue = returnValue.substring(1, returnValue.length());
        if(returnValue.charAt(returnValue.length() - 1) == ' ') returnValue = returnValue.substring(0, Math.max(0, returnValue.length() - 1));
        return returnValue;
      case HYPHENATED:
        int pieces = 0;
        int size = 0;
        int noSizeParts = 0;
        boolean apart = false;
        for(int i = 0; i < m_value.length(); i++) {
          if(m_value.charAt(i) == '-') {
              pieces++;
              indexOfLastApo = i;
              if(pieces > 3){
                apart = true;
              }
              if(pieces == 2 && size > 2) {
                apart = true;
              }
              if(size == 0) noSizeParts++;
              size = 0;
          }
          else {
            size++;
          }
        }
        if(pieces == noSizeParts) apart = true;
        
        String returnString = m_value;

        if(apart){
          returnString = m_value.replaceAll("-", " - ");
          returnString = returnString.replaceAll("'", " ' ");
        }

        if(returnString.charAt(0) == ' ') returnString = returnString.substring(1, returnString.length());
        if(returnString.charAt(returnString.length() - 1) == ' ') returnString = returnString.substring(0, Math.max(0, returnString.length() - 1));

        return returnString;
      case NEWLINE:
        return "\n";
      case PUNCTUATION:
        return m_value;
      default:
        return "UNKNOWN(" + m_value + ")";
    }
  }
}

