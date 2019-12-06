JAVAC=javac
CLASSPATH=opennlp-tools-1.9.1.jar:.

all: Splitter.class Token.class Lexer.class Scanner.class Tagger.class Analyzer.class

%.class: %.java
	$(JAVAC) -cp $(CLASSPATH) $^

Lexer.java: tiny.flex
	$(JFLEX) tiny.flex

clean:
	rm -f *.class *~
