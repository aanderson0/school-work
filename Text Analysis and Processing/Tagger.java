import java.io.FileInputStream; 
import java.io.InputStream; 
import java.io.InputStreamReader;
import java.io.BufferedReader;

import opennlp.tools.postag.POSModel;
import opennlp.tools.postag.POSTaggerME;

public class Tagger { 
   public static void main(String args[]) throws Exception{     
     
      // Load and instantiate the POSTagger
      InputStream posTagStream = new FileInputStream("OpenNLP_models/en-pos-maxent.bin");
      POSModel posModel = new POSModel(posTagStream);
      POSTaggerME posTagger = new POSTaggerME(posModel);
   
      String line;
      BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

      while((line = reader.readLine()) != null) {
        // Split text into space-separated tokens
        String tokens[] = line.split("[ ]+");       
          
        //Tagging all tokens
        String tags[] = posTagger.tag(tokens);

        //Printing the token-tag pairs  
        for( int i = 0; i < tokens.length; i++ ) 
           System.out.print(tokens[i] + "/" + tags[i] + " "); 
        System.out.print("\n"); 
      }
   } 
} 

