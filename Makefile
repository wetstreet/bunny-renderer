
renderer:
	@g++ -c include/glad/glad.c -Iinclude
	@g++ -c include/stb/stb_image.cpp -Iinclude
	@g++ -c include/imgui/imgui.cpp -Iinclude
	@g++ -c include/imgui/imgui_demo.cpp -Iinclude
	@g++ -c include/imgui/imgui_draw.cpp -Iinclude
	@g++ -c include/imgui/imgui_impl_glfw.cpp -Iinclude
	@g++ -c include/imgui/imgui_impl_opengl3.cpp -Iinclude
	@g++ -c include/imgui/imgui_tables.cpp -Iinclude
	@g++ -c include/imgui/imgui_widgets.cpp -Iinclude

	@g++ -c src/opengl/Camera.cpp -Iinclude
	@g++ -c src/opengl/Texture.cpp -Iinclude
	@g++ -c src/opengl/VAO.cpp -Iinclude
	@g++ -c src/opengl/VBO.cpp -Iinclude
	@g++ -c src/opengl/EBO.cpp -Iinclude
	@g++ -c src/opengl/Shader.cpp -Iinclude
	@g++ -c src/opengl/Mesh.cpp -Iinclude
	@g++ -c src/opengl/Scene.cpp -Iinclude
	@g++ -c src/opengl/Skybox.cpp -Iinclude
	
	@g++ -c include/tgaimage.cpp -Iinclude
	@g++ -c src/rasterizer/our_gl.cpp -Iinclude
	
	@g++ -c src/common/Object.cpp -Iinclude
	@g++ -c src/common/DirectionalLight.cpp -Iinclude
	@g++ -c src/main.cpp -Iinclude
	@g++ -c src/OpenGLRenderer.cpp -Iinclude
	@g++ -c src/RasterizerRenderer.cpp -Iinclude

	@g++ -o renderer.exe main.o OpenGLRenderer.o RasterizerRenderer.o Skybox.o Object.o DirectionalLight.o glad.o stb_image.o \
			imgui.o imgui_demo.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_tables.o imgui_widgets.o \
			Camera.o Texture.o VAO.o VBO.o EBO.o Shader.o Mesh.o Scene.o our_gl.o tgaimage.o \
			 -Iinclude -Llib -lmingw32 -lglfw3 -lopengl32 -lgdi32 -luser32 -limm32
	@del *.o

raytracer:
	@g++ -c include/tgaimage.cpp -Iinclude
	@g++ -c src/raytracer/main.cpp -o raytracer.o -Iinclude
	@g++ -o raytracer.exe raytracer.o tgaimage.o
	@del *.o

clean:
	@del *.o
	@del *.exe
	@del *.tga