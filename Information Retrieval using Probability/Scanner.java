import java.io.InputStreamReader;

public class Scanner {
  private Lexer scanner = null;

  public Scanner( Lexer lexer ) {
    scanner = lexer; 
  }

  public Token getNextToken() throws java.io.IOException {
    return scanner.yylex();
  }

  public static void main(String argv[]) {
    try {
      Scanner scanner = new Scanner(new Lexer(new InputStreamReader(System.in)));
      Token tok = scanner.getNextToken();
      Token nextTok = null;
      while( tok!= null ){
        nextTok = scanner.getNextToken();
	      if(tok.toString().equals("\n")) System.out.print(tok);        
        else if(nextTok != null && !nextTok.toString().equals("\n")) System.out.print(tok + " ");
        else System.out.print(tok);
        tok = nextTok;
      }
    }
    catch (Exception e) {
      System.out.println("Unexpected exception:");
      e.printStackTrace();
    }
  }
}
