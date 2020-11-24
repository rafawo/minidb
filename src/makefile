FILES = querySemanticAnalyzer.c SemanticTypes.h ThrashCan.h
OBJECTS_NI = TableFunctions.o querySemanticAnalyzerNI.o ThrashCan.o
OBJECTS_I = TableFunctions.o querySemanticAnalyzerI.o ThrashCan.o

all: MiniDBShell intrinsics

intrinsics: intrinsics.o $(OBJECTS_I)
	gcc -o intrinsics intrinsics.o $(OBJECTS_I) 

MiniDBShell: MiniDBShell.o $(OBJECTS_NI)
	gcc -o MiniDBShell MiniDBShell.o $(OBJECTS_NI)
	
TableFunctions.o: TableFunctions.c TableFunctions.h TableTypes.h
	gcc -g -c TableFunctions.c -o TableFunctions.o

MiniDBShell.o: MiniDBShell.c TableFunctions.h queryParser.tab.c lex.yy.c
	gcc -g -c MiniDBShell.c -o MiniDBShell.o
	
intrinsics.o: MiniDBShell.c TableFunctions.h queryParser.tab.c lex.yy.c
	gcc -g -c MiniDBShell.c -o intrinsics.o
	
querySemanticAnalyzerNI.o: querySemanticAnalyzer.c querySemanticAnalyzer.h SemanticTypes.h
	gcc -g -c querySemanticAnalyzer.c -o querySemanticAnalyzerNI.o
	
querySemanticAnalyzerI.o: querySemanticAnalyzer.c querySemanticAnalyzer.h SemanticTypes.h
	gcc -g -c -DINTRINSICS querySemanticAnalyzer.c -o querySemanticAnalyzerI.o
	
ThrashCan.o: ThrashCan.c ThrashCan.h
	gcc -g -c ThrashCan.c -o ThrashCan.o
	
queryParser.tab.c: queryParser.y $(FILES)
	bison -d -g queryParser.y
	
lex.yy.c: queryParser.l
	flex queryParser.l

$PHONY:
documentation: Documentation *.c *.h
	doxygen Documentation
	
clean:
	rm intrinsics.o MiniDBShell.o querySemanticAnalyzerI.o querySemanticAnalyzerNI.o TableFunctions.o ThrashCan.o
