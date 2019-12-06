import java.io.FileInputStream; 
import java.io.InputStream;  
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.*;

import opennlp.tools.sentdetect.SentenceDetectorME; 
import opennlp.tools.sentdetect.SentenceModel;  

public class Splitter { 
   
   public static String readString(String arg) throws Exception {
      InputStream infile = new FileInputStream(arg);
      BufferedReader buf = new BufferedReader(new InputStreamReader(infile));
      StringBuilder sb = new StringBuilder();
      String line = buf.readLine();
      while( line != null ) {
        sb.append(line + " ");
        line = buf.readLine();
      }
      return sb.toString();  
   }
  
   public static void main(String args[]) throws Exception { 
   
      //Load sentence detector model 
      InputStream modelData = new FileInputStream("OpenNLP_models/en-sent.bin"); 
      SentenceModel model = new SentenceModel(modelData); 
       
      //Instantiate SentenceDetectorME 
      SentenceDetectorME detector = new SentenceDetectorME(model);  
    
      //Allow multiple files to be processed
      for (String arg : args ) {
        //Split a file into sentences
        String sentences[] = detector.sentDetect(readString(arg)); 

         List<String> reformattedSentences = new ArrayList<String> ();
         for(int i = 0; i < sentences.length; i++) {
            if(sentences[i].contains("$DOC")) {
               int docIndex = sentences[i].indexOf("$DOC");
               int titleIndex = sentences[i].indexOf("$TITLE");
               reformattedSentences.add(sentences[i].substring(docIndex, titleIndex));
               reformattedSentences.add(sentences[i].substring(titleIndex, titleIndex + "$TITLE".length()));
               if(sentences[i].contains("$TEXT")) {
                  int textIndex = sentences[i].indexOf("$TEXT");
                  reformattedSentences.add(sentences[i].substring(textIndex, textIndex + "$TEXT".length()));
                  reformattedSentences.add(sentences[i].substring(textIndex + "$TEXT".length() + 1, sentences[i].length()));
               }
               else {
                  reformattedSentences.add(sentences[i].substring(titleIndex + "$TITLE".length() + 1, sentences[i].length()));
               }
            }
            else if(sentences[i].contains("$TEXT")){
               int textIndex = sentences[i].indexOf("$TEXT");
               reformattedSentences.add(sentences[i].substring(textIndex, textIndex + "$TEXT".length()));
               reformattedSentences.add(sentences[i].substring(textIndex + "$TEXT".length() + 1, sentences[i].length()));
            }
            else{
               reformattedSentences.add(sentences[i]);
            }
         }

        //Print the sentences 
        for(String sent : reformattedSentences)        
           System.out.println(sent);  
      }
   } 
}
