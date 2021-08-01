

main: main.o glad.o
	g++ -o main.exe main.o glad.o -ILibraries/include -LLibraries/lib -lmingw32 -lglfw3 -lopengl32 -lgdi32 -luser32
	del main.o
	del glad.o

main.o: main.cpp
	g++ -c main.cpp -ILibraries/include

glad.o: glad.c
	g++ -c glad.c -ILibraries/include

.PHONY : clean
clean:
	del main.o
	del glad.o