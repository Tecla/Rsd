grammar: GrammarMain.cpp GrammarReference.cpp

GrammarMain.cpp: lemon ../GrammarMain.ly
	./lemon -l -q -s ../GrammarMain.ly
	mv ../GrammarMain.c ../GrammarMain.cpp

GrammarReference.cpp: lemon ../GrammarReference.ly
	./lemon -l -q -s ../GrammarReference.ly
	mv ../GrammarReference.c ../GrammarReference.cpp

lemon: lemon.c lempar.c
	gcc -o lemon lemon.c

clean:
	rm -f lemon

