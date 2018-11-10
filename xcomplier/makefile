all:
	g++ -o xs -g  main.cpp xcomplier.cpp parser.cpp lexer.cpp i_code.cpp code_emit.cpp
	cp bin/T*.XSS .
	./xs T1.XSS -N
	./xs T2.XSS -N
	./xs T3.XSS -N
	./xs T4.XSS -N
	./xs T5.XSS -N
	mv xs ./bin
	mv T*.XASM bin/
	mv T*.XSS bin/

c:
	rm bin/xs	
