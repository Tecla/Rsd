# This is here for reference; you should use the waf build system instead!

CC ?= gcc

.PHONY: all clean

all: grammar

grammar: ../GrammarMain.cpp ../GrammarReference.cpp

../GrammarMain.cpp: lemon ../GrammarMain.ly
	./lemon -l -q -s ../GrammarMain.ly
	mv ../GrammarMain.c ../GrammarMain.cpp

../GrammarReference.cpp: lemon ../GrammarReference.ly
	./lemon -l -q -s ../GrammarReference.ly
	mv ../GrammarReference.c ../GrammarReference.cpp

lemon: lemon.c lempar.c.template
	$(CC) -o $@ lemon.c
	cp lempar.c.template lempar.c

clean:
	rm -f lemon lempar.c

distclean:
	rm -f lemon lempar.c
	rm -f ../GrammarMain.cpp ../GrammarReference.cpp ../GrammarMain.h ../GrammarReference.h

