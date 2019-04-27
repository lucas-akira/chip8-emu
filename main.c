#include "chip8.h"

static const char keycodes[] = {'0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const char *vertexShaderSource = 
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
    	"void main()\n"
    	"{\n"
    	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    	"}\0";

const char *fragmentShaderSource = 
	"#version 330 core\n"
    	"out vec4 FragColor;\n"
	"uniform vec4 color;\n"
    	"void main()\n"
    	"{\n"
    	"   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    	"}\n\0";

int createVertices(unsigned char gfx[WIDTH*HEIGHT], unsigned int **indices) {
	if (*indices != NULL) { 
		free(*indices);
	}
	*indices = calloc(HEIGHT*WIDTH*6, sizeof(unsigned int));
	unsigned int currentIndex = 0;
	int i, j;
	for (j = 0; j < HEIGHT ; j++) {
		for (i = 0; i < WIDTH; i++) {
			if (gfx[WIDTH*j+i]) { /* If the pixel is set to 1 */
				/* Create the pixel (rectangle composed of 2 triangles) */

				/* Triangle 1 */
				(*indices)[currentIndex++] = (WIDTH+1)*j + i;		/* Top left  */
				(*indices)[currentIndex++] = (WIDTH+1)*j + i + 1;	/* Top right */
				(*indices)[currentIndex++] = (WIDTH+1)*(j + 1) + i;	/* Bottom left */

				/* Triangle 2 */
				(*indices)[currentIndex++] = (WIDTH+1)*(j + 1) + i;	/* Bottom left */
				(*indices)[currentIndex++] = (WIDTH+1)*(j + 1) + i + 1;	/* Bottom right */
				(*indices)[currentIndex++] = (WIDTH+1)*j + i + 1;	/* Top right */
			}
		}
	}
	return currentIndex;
}


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	const char *key_name = glfwGetKeyName(key, scancode);
	int i;
	
	if ((action == GLFW_PRESS || action == GLFW_RELEASE) && key_name != NULL) {
		
		/* Check which key was pressed or released */
		for (i = 0; i < 16; i++) {
			if (keycodes[i] == key_name[0]) 
				break;
		}
		if (i != 16) {
			Chip8 *chip_ref = glfwGetWindowUserPointer(window);
			if (chip_ref == NULL) {
				return;
			}

			if (i > 0xF) {
				printf("Invalid keycode!\n");
				return;
			}
			
			if (action == GLFW_PRESS) {
				chip_ref->keypad[i] = 1;
			} else {
				chip_ref->keypad[i] = 0;
			}
		}
	}
}
				

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
}

int main() {
	
	Chip8 chip8;
	initialize(&chip8);
	/*
	printf("%d \n", chip8.pc);
	printf("%d \n",chip8.opcode);
	printf("%d \n",chip8.index_reg);
	printf("%d \n",chip8.sp);
	*/
	/*
	int i;
	for (i = 0;i < 64; i++)
		chip8.gfx[i] = 1;
	*/

	/* GLFW Initialization and configuration */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); /* Use version 3 */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  /* Use modern OpenGL (Core profile) */

	/* Line needed for MacOS X */
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* GLFW window creation */ 
	GLFWwindow *window = glfwCreateWindow(250,250, "OpenGL Test",NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); /* Make the context (state of OpenGL) of the window the main context of the thread */
	
	/* Call framebuffer_size_callback() each time the window changes in size */
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/* Call key_callback() each time a key is pressed or released */
	glfwSetKeyCallback(window, key_callback);

	/* Save chip struct to be used in other functions */
	glfwSetWindowUserPointer(window, &chip8);

	/* GLAD: Load all OpenGL function pointers (Operating System specific) */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	/* Vertex shader */
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); /* Attach shader source to the shader object */
	glCompileShader(vertexShader); /* Compile shader */
	
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("[ERROR] Vertex shader compilation failed!\n");
		printf("%s\n", infoLog);
	} else {
		printf("Vertex shader compiled\n");
	}

	/* Fragment shader */ 
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("[ERROR] Fragment shader compilation failed!\n");
		printf("%s\n", infoLog);
	} else {
		printf("Fragment shader compiled\n");
	}
	
	/* Shader program - linking vertex and fragment shaders together */
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, & success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("[ERROR] Shader program linking failed!\n");
		printf("%s\n", infoLog);
	} else {
		printf("Shader program created\n");
	}

	/* Delete objects after linking */
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	/* Data to send */
	/******************************************************/
	
	/* Calculate the normalized coordinates of each vertex */
	int y, x;
	int number_vertices = (WIDTH+1)*(HEIGHT+1);
	float width = (float)WIDTH;
	float height = (float)HEIGHT;
	float points[ 2*number_vertices ]; /* 2 times the number of vertices to store the x and y coordinates */
	for (y = 0; y < HEIGHT+1; y++) {
		for(x = 0; x < WIDTH+1; x++) {
			points[2*(WIDTH+1)*y + 2*x] = ( -1.0f + x * 2.0f / width );
			points[2*(WIDTH+1)*y + 2*x + 1] = ( (height/width) - y * 2.0f / width ); /* Only works if height <= width */
		}
	}
	unsigned int indices_len;
	unsigned int *indices = NULL;
	indices_len = createVertices(chip8.gfx, &indices);
	/******************************************************/

	/* Vertex Buffer Object (VBO) and Vertex Array Object (VAO)*/
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); /* Generate 1 Buffer object and store its ID in VBO */
	glGenBuffers(1, &EBO);
	
	glBindVertexArray(VAO); /* Bind VAO first */

	glBindBuffer(GL_ARRAY_BUFFER, VBO); /* Bind the vertex buffer to the GL_ARRAY_BUFFER target (type of a vertex buffer object) */
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); /* Load data into the buffer's memory */
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices_len, indices, GL_DYNAMIC_DRAW);

	/* Telling OpenGL how to interpret the data */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2* sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); /* Enable the vertex attribute */
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	
	/* Render loop */
	while(!glfwWindowShouldClose(window)) {
			
		/* Rendering */
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_len, indices, GL_DYNAMIC_DRAW);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices_len, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	
	if (indices) {
		free(indices);
		indices = NULL;
	}

	glfwTerminate();
	return 0;
}
