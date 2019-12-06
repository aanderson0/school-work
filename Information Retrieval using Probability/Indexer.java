import java.io.*; 
import java.util.*;

public class Indexer { 
   
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
   
        List<String> lines = readFile(args[0]);
        List<DidStruct> docids = new ArrayList<DidStruct>();
        int docNum = 0;
        int totalPostingEntries = 0;
        TreeMap<String, ArrayList<DocStruct>> map = new TreeMap<String, ArrayList<DocStruct>>();

        //read original file for titles
        List<String> originalFile = readFile(args[0].substring(0,args[0].indexOf(".processed")) + ".txt");

         for(String line : lines) {

            if(line.contains("$DOC")) {
                docNum++;
                docids.add(new DidStruct(line.substring(5, line.length()), originalFile.indexOf(line) + 1, ""));
                for(int i = originalFile.indexOf(line) + 1; !originalFile.get(i).equals("$TEXT"); i++){
                    if(!originalFile.get(i).equals("$TITLE")){
                        docids.get(docids.size() - 1).title += originalFile.get(i);
                    }
                }
            }
            else if(line.contains("$TITLE")){
                ;
            }
            else if(line.contains("$TEXT")){
                ;
            }
            else{
                String[] words = line.split(" ");
                for(String word : words){
                    if(word.equals("")) {
                        continue;
                    }
                    if(!map.containsKey(word)) {
                        ArrayList<DocStruct> toAdd = new ArrayList<DocStruct>();
                        toAdd.add(new DocStruct(docNum, 1));
                        map.put(word, toAdd);
                        totalPostingEntries++;
                    }
                    else {
                        ArrayList<DocStruct> entries = map.get(word);
                        DocStruct lastEntry = entries.get(entries.size() - 1);
                        if(lastEntry.did == docNum) {
                            lastEntry.tf++;
                        }
                        else {
                            map.get(word).add(new DocStruct(docNum, 1));
                            totalPostingEntries++;
                        }
                    }
                }
            }
         }

        //Print output
        PrintWriter outDic = new PrintWriter("dictionary.txt");
        PrintWriter outPost = new PrintWriter("postings.txt");
        PrintWriter outDid = new PrintWriter("docids.txt");

        Set<String> keys = map.keySet();
        Iterator<String> iter = keys.iterator();

        outDic.println(keys.size());
        outPost.println(totalPostingEntries);
        while (iter.hasNext()) {
            String key = iter.next();
            outDic.println(key + " " + map.get(key).size());
            for(DocStruct doc : map.get(key)){
                outPost.println(doc.did + " " + doc.tf);
            }
        } 
        outDid.println(docNum);
        for(DidStruct doc : docids){
            outDid.println(doc.did + " " + doc.start + " " + doc.title);
        }        

        outDic.close();
        outPost.close();
        outDid.close();
    } 
}

class DocStruct {
    public int did;
    public int tf;

    DocStruct(int did, int tf) {
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