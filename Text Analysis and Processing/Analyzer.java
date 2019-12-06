import java.io.FileInputStream; 
import java.io.InputStream; 
import java.io.InputStreamReader;
import java.io.BufferedReader;

public class Analyzer { 
   public static void main(String args[]) throws Exception{     

      float numDocs = 0;
      float minSenLength = 100;
      float totalSen = 0;
      float maxSenLength = 0;
      float minTokLength = 100;
      float totalTok = 0;
      float maxTokLength = 0;
      float docSen = 0;
      float docTok = 0;

      String line;
      BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

      while((line = reader.readLine()) != null) {
        if(line.contains("$DOC")){
            if(numDocs != 0){
                if(docSen < minSenLength) minSenLength = docSen;
                if(docSen > maxSenLength) maxSenLength = docSen;
                if(docTok < minTokLength) minTokLength = docTok;
                if(docTok > maxTokLength) maxTokLength = docTok;

                if(docTok != 0) System.out.println("Document average sentence length (token): " + docTok / docSen);

                docSen = 0;
                docTok = 0;
            }
            numDocs++;
        }
        if(!line.contains("$DOC") && !line.contains("$TITLE") && !line.contains("$TEXT")){
            docSen++;
            totalSen++;
            // Split text into space-separated tokens
            String tokens[] = line.split("[ ]+");  
            for(String tok : tokens) {
                docTok++;
                totalTok++;
            }
        }     
      }

      System.out.println("-----------------RESULTS----------------");
      System.out.println("Number of documents: " + numDocs);
      System.out.println("Minimum length (sentence): " + minSenLength);
      System.out.println("Average length (sentence): " + totalSen / numDocs);
      System.out.println("Maximum length (sentence): " + maxSenLength);
      System.out.println("Minimum length (token): " + minTokLength);
      System.out.println("Average length (token): " + totalTok / numDocs);
      System.out.println("Maximum length (token): " + maxTokLength);
      System.out.println("Average sentence length (token): " + totalTok / totalSen);
   } 
} 

