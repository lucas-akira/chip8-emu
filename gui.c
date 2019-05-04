#include "gui.h"

static const char keycodes[2][16] = {{'x','1', '2', '3', 'q', 'w', 'e', 'a', 's', 'd', 'z', 'c', '4', 'r', 'f', 'v'}, {'x','1','2','3','a','z','e','q','s','d','w','c','4','r','f','v'}};

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
	Chip8 *chip_ref = glfwGetWindowUserPointer(window);
	chip_ref->update_screen = 1;
	if (height >= width/2) {
		glViewport(0, (height/2) - (width/4), width, width/2);
		glScissor(0,(height/2)- (width/4), width, width/2);
	} else {
		glViewport(0,0, width, height);
		glScissor(0,0, width, height);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	const char *key_name = glfwGetKeyName(key, scancode);
	int i;
	Chip8 *chip_ref = glfwGetWindowUserPointer(window);

	if (key == GLFW_KEY_TAB && action == GLFW_RELEASE) {
		if (chip_ref == NULL)
			return;
		/* Tab key: change current layout to QWERTY or AZERTY */	
		chip_ref->key_layout = !(chip_ref->key_layout);
	
	} else if ((action == GLFW_PRESS || action == GLFW_RELEASE) && key_name != NULL) {
		
		/* Check which key was pressed or released */
		for (i = 0; i < 16; i++) {
			if (keycodes[chip_ref->key_layout][i] == key_name[0]) 
				break;
		}
		if (i != 16) {
			if (chip_ref == NULL) {
				return;
			}	

			if (i > 0xF) {
				printf("Invalid keycode!\n");
				return;
			}
			
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
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

int runGUI(Chip8 *chip8) {
	
	/* GLFW Initialization and configuration */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); /* Use version 3.3 */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  /* Use modern OpenGL (Core profile) */

	/* Line needed for MacOS X */
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* GLFW window creation */ 
	GLFWwindow *window = glfwCreateWindow(800,600, "chip8-emu",NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	/* Make the context (state of OpenGL) of the window the main context of the thread */
	glfwMakeContextCurrent(window);

	/* Call framebuffer_size_callback() each time the window changes in size */
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/* Call key_callback() each time a key is pressed or released */
	glfwSetKeyCallback(window, key_callback);

	/* Save chip struct to be used in other functions */
	glfwSetWindowUserPointer(window, chip8);

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

	/* Calculate the normalized coordinates of each vertex */
	/******************************************************/
	int y, x;
	int number_vertices = (WIDTH+1)*(HEIGHT+1);
	float width = (float)WIDTH;
	float height = (float)HEIGHT;
	float points[ 2*number_vertices ]; /* 2 times the number of vertices to store the x and y coordinates */
	for (y = 0; y < HEIGHT+1; y++) {
		for(x = 0; x < WIDTH+1; x++) {
			points[2*(WIDTH+1)*y + 2*x] = ( -1.0f + x * 2.0f / width );
			points[2*(WIDTH+1)*y + 2*x + 1] = ( 1.0f  - y * 2.0f / height ); /* Only works if height <= width */
		}
	}
	unsigned int indices_len;
	unsigned int *indices = NULL;
	/* Data to send */
	indices_len = createVertices(chip8->gfx, &indices);
	/******************************************************/

	/* Vertex Buffer Object (VBO), Vertex Array Object (VAO) and Element Buffer Object (EBO) */
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); /* Generate 1 Buffer object and store its ID in VBO */
	glGenBuffers(1, &EBO);
	
	glBindVertexArray(VAO); /* Bind VAO first */

	/* Bind the vertex buffer to the GL_ARRAY_BUFFER target (type of a vertex buffer object) */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	/* Load data into the buffer's memory */
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); 

	/* Same with EBO */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices_len, indices, GL_DYNAMIC_DRAW);

	/* Telling OpenGL how to interpret the data */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2* sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); /* Enable the vertex attribute */

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	double previousTime = glfwGetTime();
	unsigned int frameCount = 0;
	unsigned int cycleCount = 0; 
	
	/* Enable V-Sync */
	glfwSwapInterval(1);

	/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/
	
	/* Initial settings */
	glViewport(0, 100, 800, 400);
	glScissor(0,100, 800, 400);
	
	/* Render loop */
	while(!glfwWindowShouldClose(window)) {

		emulateCycle(chip8);
		cycleCount++;
		/* Wait at least 15 cycles before updating the screen - if running at 60 fps, we are executing at least 900 operations per second */		
		if (chip8->update_screen && cycleCount >= 15) {
			indices_len = createVertices(chip8->gfx, &indices);
			chip8->update_screen = 0;
			cycleCount = 0;

 			/* Measure fps */
    			double currentTime = glfwGetTime();
    			frameCount++;
    			/* If a second has passed, display fps */
    			if ( currentTime - previousTime >= 1.0 ) {
		        	char title [256];
				title [255] = '\0';

				snprintf ( title, 255,"chip8-emu [FPS: %3.2f]", (float)frameCount );
				glfwSetWindowTitle (window, title);
				frameCount = 0;
        			previousTime = currentTime;
    			}

			/* Draw background */
			glDisable(GL_SCISSOR_TEST);
			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_SCISSOR_TEST);

			/* Rendering */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_len, indices, GL_DYNAMIC_DRAW);
	
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		
			glUseProgram(shaderProgram);
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices_len, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			glfwSwapBuffers(window);
		}
		glfwPollEvents();
	}

	/* Clear allocated memory */
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	
	if (indices) {
		free(indices);
		indices = NULL;
	}

	glfwTerminate();
	return 0;

}
