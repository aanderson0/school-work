import java.io.FileInputStream; 
import java.io.InputStream;  
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.*;

import opennlp.tools.stemmer.PorterStemmer; 

public class Preprocessor { 
   
   public static List<String> readFile(String arg) throws Exception {
      InputStream infile;
      BufferedReader buf;
      if(arg.equals("stdin")) buf = new BufferedReader(new InputStreamReader(System.in));
      else {
        infile = new FileInputStream(arg);  
        buf = new BufferedReader(new InputStreamReader(infile));
      }
      String line = buf.readLine();
      List<String> lines = new ArrayList<String>();
      while(line != null) {
        lines.add(line);
        line = buf.readLine();
      } 
      buf.close();
      return lines;  
   }
  
   public static void main(String args[]) throws Exception { 
   
        List<String> lines = readFile("stdin");
        List<String> output = new ArrayList<String>();
        List<String> stopWords = readFile("stopwords.txt");

        //Create an instance of stemmer
        PorterStemmer stemmer = new PorterStemmer();

         for(String line : lines) {
            if(line.contains("$DOC") || line.contains("$TITLE") || line.contains("$TEXT")) {
               output.add(line);
            }
            else{
               line = line.toLowerCase();
               String[] words = line.split(" ");
               line = "";
               for(String word : words){
                    if(word.matches("(.*[a-zA-Z]+.*)*") && stemmer.stem(word).matches("(.*[a-zA-Z]+.*)*")){
                        boolean containsStop = false;
                        for(String stopWord : stopWords) { 
                            if(stopWord.equals(word)) containsStop = true;
                            if(stopWord.equals(stemmer.stem(word))) containsStop = true;
                        }
                        if(!containsStop){
                            word = stemmer.stem(word);
                            line = line + " " + word;
                        }
                    }
               }
               if(!line.equals("")) output.add(line);
            }
         }

        //Print output
        for(String sent : output)    
           System.out.println(sent);  
    } 
}
