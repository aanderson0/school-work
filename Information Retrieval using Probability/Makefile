JAVAC=javac
CLASSPATH=opennlp-tools-1.9.1.jar:.
JFLEX=jflex

all: Token.class Lexer.class Scanner.class Preprocessor.class Indexer.class Retriever.class

%.class: %.java
	$(JAVAC) -cp $(CLASSPATH) $^

Lexer.java: tiny.flex
	$(JFLEX) tiny.flex

clean:
	rm -f *.class *~ *.tokenized *.processed dictionary.txt postings.txt docids.txt

tokenizer:
ifndef input
	java Scanner < documents.txt > documents.tokenized
else
	java Scanner < $(input).txt > $(input).tokenized
endif

preprocessor:
ifndef input
	java -cp opennlp-tools-1.9.1.jar:. Preprocessor < documents.tokenized > documents.processed
else
	java -cp opennlp-tools-1.9.1.jar:. Preprocessor < $(input).tokenized > $(input).processed
endif

indexer:
ifndef input
	java Indexer documents.processed
else
	java Indexer $(input).processed
endif

retriever:
	java -cp opennlp-tools-1.9.1.jar:. Retriever