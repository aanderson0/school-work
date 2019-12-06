import java.io.*;
import java.text.DecimalFormat;
import java.util.*;

import opennlp.tools.stemmer.PorterStemmer; 

public class Retriever { 
   
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
   
        System.out.println("Loading...");

        List<String> dicData = readFile("dictionary.txt");
        List<String> postData = readFile("postings.txt");
        List<String> didData = readFile("docids.txt");

        List<TermStruct> terms = new ArrayList<TermStruct>();
        List<PostingStruct> postings = new ArrayList<PostingStruct>();
        List<DidStruct> docids = new ArrayList<DidStruct>();

        List<String> stopWords = readFile("stopwords.txt");
        PorterStemmer stemmer = new PorterStemmer();

        int numPosting = 0;
        int numDocs = 0;

        int offset = 0;
        for(String line : dicData) {
            String[] data = line.split(" ");
            if(data.length == 2){
                terms.add(new TermStruct(data[0], offset));
                offset += Integer.parseInt(data[1]);
            }
        }
        for(String line : postData) {
            String[] data = line.split(" ");
            if(data.length == 2){
                postings.add(new PostingStruct(Integer.parseInt(data[0]), Integer.parseInt(data[1])));
                numPosting++;
            }
        }
        for(String line : didData) { 
            String[] data = line.split(" ");
            if(data.length > 1){
                docids.add(new DidStruct(data[0], Integer.parseInt(data[1]), line.substring(line.indexOf(data[1],data[0].length() + 1) + data[1].length() + 1)));
                numDocs++;
            }
        }


        BufferedReader buf = new BufferedReader(new InputStreamReader(System.in));
        System.out.println("Ready!");
        System.out.println("Type \"quit\" to leave the program");
        while(true){
            
            //get input
            System.out.println("");  
            System.out.print("Enter a query: ");    
            String input = buf.readLine();
            while(input != null && input.isEmpty()) input = buf.readLine();
            if(input == null || input.equalsIgnoreCase("quit") || input.equalsIgnoreCase("q")) break;

            //normalize and stem input
            input = input.toLowerCase();
            String[] words = input.split(" ");
            input = "";
            for(String word : words){
                if(word.matches("(.*[a-zA-Z]+.*)*") && stemmer.stem(word).matches("(.*[a-zA-Z]+.*)*")){
                    boolean containsStop = false;
                    for(String stopWord : stopWords) { 
                        if(stopWord.equals(word)) containsStop = true;
                        if(stopWord.equals(stemmer.stem(word))) containsStop = true;
                    }
                    if(!containsStop){
                        word = stemmer.stem(word);
                        input = input + " " + word;
                    }
                }
            }

            //process query

            List<Result> results = new ArrayList<Result>();

            for(String term : input.split(" ")) {
                //find term in dictionary
                for(TermStruct dicTerm : terms){

                    if(term.equals(dicTerm.term)){
                        int df = 0;
                        int endOffset = 0;
                        if(terms.indexOf(dicTerm) == terms.size() - 1){
                             df = numPosting - dicTerm.offset;
                             endOffset = numPosting;
                        }
                        else {
                            df = terms.get(terms.indexOf(dicTerm) + 1).offset - dicTerm.offset;
                            endOffset = terms.get(terms.indexOf(dicTerm) + 1).offset;
                        }
                        double idf = Math.log((double)numDocs / (double)df);
                        for(PostingStruct posting : postings.subList(dicTerm.offset, endOffset)) {
                            int tf = posting.tf;
                            double w = (double)tf * idf;
                            boolean found = false;
                            for(Result r : results) {
                                if(r.did == posting.did){
                                    r.similarity += w;
                                    found = true;
                                    break;
                                }
                            }
                            if(!found) {
                                Result r = new Result(posting.did, w);
                                results.add(r);
                            }
                        }
                    }
                }
            }

            //print results
            Collections.sort(results, new SortResult());
            for(Result r : results.subList(0, Math.min(10, results.size()))) {
                System.out.print(new DecimalFormat("0.00").format(r.similarity) + ":\t");
                    System.out.println(docids.get(r.did - 1).did + " " + new DecimalFormat("0000000").format(docids.get(r.did - 1).start) + " " + docids.get(r.did - 1).title);
            }
        }
   }
}

class Result{
    int did;
    double similarity;

    Result(int did, double similarity){
        this.did = did;
        this.similarity = similarity;
    }
}

class SortResult implements Comparator<Result>{
    public int compare(Result a, Result b){
        double diff = a.similarity - b.similarity;
        if(diff < 0) return 1;
        else if(diff > 0) return -1;
        return 0;
    }
}

class TermStruct {
    public String term;
    public int offset;
    public int df = -1;

    TermStruct(String term, int offset) {
        this.term = term;
        this.offset = offset;
    }
}

class PostingStruct {
    public int did;
    public int tf;

    PostingStruct(int did, int tf) {
        this.did = did;
        this.tf = tf;
    }
}

class DidStruct {
    public String did;
    public int start;
    public String title;

    DidStruct(String did, int start, String title) {
        this.did = did;
        this.start = start;
        this.title = title;
    }
}
