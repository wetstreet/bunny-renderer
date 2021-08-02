
all: opengl rasterizer raytracer
	del *.o

opengl:
	g++ -c src/opengl/glad.c -Iinclude
	g++ -c src/opengl/main.cpp -Iinclude -o opengl.o
	g++ -o opengl.exe opengl.o glad.o -Iinclude -Llib -lmingw32 -lglfw3 -lopengl32 -lgdi32 -luser32
	del *.o

rasterizer:
	g++ -c src/tgaimage.cpp -Iinclude
	g++ -c src/rasterizer/main.cpp -o rasterizer.o -Iinclude
	g++ -c src/rasterizer/model.cpp -Iinclude
	g++ -c src/rasterizer/our_gl.cpp -Iinclude
	g++ -c src/rasterizer/geometry.cpp -Iinclude
	g++ -o rasterizer.exe rasterizer.o model.o our_gl.o geometry.o tgaimage.o
	del *.o

raytracer:
	g++ -c src/tgaimage.cpp -Iinclude
	g++ -c src/raytracer/main.cpp -o raytracer.o -Iinclude
	g++ -o raytracer.exe raytracer.o tgaimage.o
	del *.o

clean:
	del *.o
	del *.exe
	del *.tga