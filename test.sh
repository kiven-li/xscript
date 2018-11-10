cd ./xasm
g++ -o xasm -g utility.cpp instruction_set.cpp lexer.cpp xasm.cpp main.cpp
mv xasm ../test/

cd ../xcomplier
g++ -o xcomplier -g  main.cpp xcomplier.cpp parser.cpp lexer.cpp i_code.cpp code_emit.cpp
mv xcomplier ../test/

cd ../console
g++ -o xvm -g -fpermissive -lrt xvm.cpp console.cpp
mv xvm ../test/

cd ../test
./xcomplier t.xss
./xvm t.xss.XSE
