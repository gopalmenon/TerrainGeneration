#define _CRT_SECURE_NO_DEPRECATE

#include <time.h>
#include <iostream>
#include <vector>
#include "Angel.h"
#include "Terrain.hpp"

// The following line is apparently necessary to allow the glew
// lib to link correctly for Visual Studios. You may need to 
// comment it out if you are using a different IDE.
#pragma comment(lib, "glew32.lib")

// Initialize the arrays for the vertices and triangles
long nvertices, nindices;
std::vector<vec3> vertices;
std::vector<int> indices;
std::vector<vec3> triangles;
std::vector<vec3> normals;
vec3 vertex;

GLfloat Theta[3] = {20.0, 180.0, 0.0}; // Array of rotaion
GLuint theta; // Location of "theta" shader uniform variable
GLfloat ModelView[4] = {0.0, 0.0, 0.0, 0.0};
GLuint model_view;
GLfloat Zoom[4] = {1.0, 1.0, 1.0, 1.0};
GLuint zoom;

// Texture objects and storage for texture image
GLuint textures[2];
const int  TextureSize  = 64;
GLubyte image[TextureSize][TextureSize][3];
std::vector<vec2> tex_coords;

char selectedTerrain, startLocation;
int screenWidth = 640, screenHeight = 480;

// Makes the triangles from the vertices and indices
void make_triangles() 
{
	int vCount = 0;
	for(unsigned int i = 0; i<indices.size(); i++){
		triangles.push_back(vertices[indices[i]]);
		//std::cout << "Storing vertex " << vertices[indices[i]].x << ", " << vertices[indices[i]].y << ", " << vertices[indices[i]].z << std::endl;

		vCount++;
		if(vCount == 3){
			vec3 normal = normalize(cross(triangles[i-1]-triangles[i-2],
							triangles[i]-triangles[i-1]));
			
			normals.push_back(normal);
			normals.push_back(normal);
			normals.push_back(normal);
			vCount = 0;
		}
	}
}

//Drape the texture on the terrain
void make_texture()
{

	float xVertexCoordinate = 0.0, yVertexCoordinate = 0.0, zVertexCoordinate = 0.0, xTextureCoordinate = 0.0, yTextureCoordinate = 0.0;
	for(unsigned int i = 0; i<indices.size(); i++) {

		xVertexCoordinate = vertices.at(indices.at(i)).x;
		yVertexCoordinate = vertices.at(indices.at(i)).y;
		zVertexCoordinate = vertices.at(indices.at(i)).z;

		xTextureCoordinate = (xVertexCoordinate - 0.75 * zVertexCoordinate + 1.5) / 3.0;
		yTextureCoordinate = (yVertexCoordinate - 0.75 * zVertexCoordinate + 1.5) / 3.0;

		tex_coords.push_back(vec2(xTextureCoordinate, yTextureCoordinate));

	}
}

//Read texture file into array
unsigned char* readTextureFile(const char * filename, int width, int height){
	unsigned char* data;
	FILE* file;

	file = fopen(filename, "rb");

	if (file == NULL){
		return 0;
	}

	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);
	fclose(file);

	for (int i = 0; i < width * height; ++i){
		int index = i * 3;
		unsigned char B, R;
		B = data[index];
		R = data[index + 2];

		data[index] = R;
		data[index + 2] = B;
	}
	return data;
}

void createIndicesAndVertices(unsigned int terrainDimension, std::vector<float> heightMap) {
	//Find the step value for x and z coordinates based on a range of -1 to +1
	float stepValue = 2.0 / terrainDimension;

	//Set the x and z coordinates to the beginning of the height map
	GLfloat xCoordinate = -1.0, zCoordinate = -1.0;

	//Loop through the height map and create vertices. The row and columns give the x and z coordinate steps. The value gives the height or y coordinate.
	unsigned int numberOfVertices = terrainDimension * terrainDimension, row, column;
	for (auto counter = 0; counter < numberOfVertices; ++counter) {
		//Compute row and column from offset
		row = counter / terrainDimension;
		column = counter % terrainDimension;
		//Reset coordinates when moving to next row
		if (counter % terrainDimension == 0 && counter != 0) {
			xCoordinate = -1.0;
			zCoordinate += stepValue;
		}
		//Put vertex into vertex array
		vertices.push_back(vec3(xCoordinate, heightMap.at(counter), zCoordinate));
		//std::cout << "Storing vertex " << counter << " at height " << heightMap.at(counter) << " for x " << xCoordinate << " and z " << zCoordinate << std::endl;
		//Move to the next element
		xCoordinate += stepValue;
	}

	//Populate the indices from the constructed terrain
	unsigned int currentVertex, vertexBelow, afterVertexBelow, nextVertex;
	for (auto rowCounter = 0; rowCounter < terrainDimension - 1; ++rowCounter) {
		for (auto cellCounter = 0; cellCounter < terrainDimension - 1; ++cellCounter) {
			//Compute the vertices for the triangles to be added
			currentVertex = rowCounter * terrainDimension + cellCounter;
			nextVertex = currentVertex + 1;
			vertexBelow = currentVertex + terrainDimension;
			afterVertexBelow = vertexBelow + 1;
			//Add vertices for two triangles
			indices.push_back(currentVertex);
			indices.push_back(vertexBelow);
			indices.push_back(afterVertexBelow);
			//std::cout << "Stored triangle 1 vertices " << currentVertex << ", " << vertexBelow << " and " << afterVertexBelow << std::endl;
			//Next triangle for the cell
			indices.push_back(currentVertex);
			indices.push_back(afterVertexBelow);
			indices.push_back(nextVertex);
			//std::cout << "Stored triangle 2 vertices " << currentVertex << ", " << afterVertexBelow << " and " << nextVertex << std::endl;
		}
	}

}

//Create terrain based on particle deposition
void createParticleDepositionTerrain(char startLocation) {

	unsigned int terrainDimension = 257;
	//Construct the terrain
	ParticleDepositionTerrain particleDepositionTerrain(terrainDimension, startLocation);
	particleDepositionTerrain.makeTerrain();

	//Populate the vertices from the constructed terrain
	std::vector<float> heightMap = particleDepositionTerrain.getTerrain();

	createIndicesAndVertices(terrainDimension, heightMap);
}

//Create terrain based on roll down particle deposition
void createRollDownParticleDepositionTerrain(char startLocation) {

	unsigned int terrainDimension = 257;
	//Construct the terrain
	RollDownParticleDepositionTerrain rollDownParticleDepositionTerrain(terrainDimension, startLocation);
	rollDownParticleDepositionTerrain.makeTerrain();

	//Populate the vertices from the constructed terrain
	std::vector<float> heightMap = rollDownParticleDepositionTerrain.getTerrain();

	createIndicesAndVertices(terrainDimension, heightMap);
}

//Create terrain based on step faults
void createStepFaultTerrain() {

	unsigned int terrainDimension = 257;
	//Construct the terrain
	StepFaultTerrain stepFaultTerrain(terrainDimension);
	stepFaultTerrain.makeTerrain();

	//Populate the vertices from the constructed terrain
	std::vector<float> heightMap = stepFaultTerrain.getTerrain();

	createIndicesAndVertices(terrainDimension, heightMap);

}

//Create terrain based on cosine bumps in random locations
void createCosineBumpTerrain() {

	unsigned int terrainDimension = 513;
	//Construct the terrain
	BumpTerrain bumpTerrain(terrainDimension);
	bumpTerrain.makeTerrain();

	//Populate the vertices from the constructed terrain
	std::vector<float> heightMap = bumpTerrain.getTerrain();

	createIndicesAndVertices(terrainDimension, heightMap);

}

//Create square diamond terrain
void createSquareDiamondTerrain() {

	unsigned int terrainDimension = 1025;
	//Construct the terrain
	SquareDiamondTerrain squareDiamondTerrain(terrainDimension);
	squareDiamondTerrain.makeTerrain();

	//Populate the vertices from the constructed terrain
	std::vector<float> heightMap = squareDiamondTerrain.getTerrain();

	createIndicesAndVertices(terrainDimension, heightMap);

}

//Get user selection
void getUserSelection() {

	std::cout << "Select the type of terrain:" << std::endl;
	std::cout << "1 = Sticky Particle Deposition" << std::endl;
	std::cout << "2 = Roll Down Particle Deposition" << std::endl;
	std::cout << "3 = Square - Diamond" << std::endl;
	std::cout << "4 = Step Fault" << std::endl;
	std::cout << "5 = Sine Fault" << std::endl;
	std::cout << "6 = Cosine Fault" << std::endl;
	std::cout << "7 = Cosine Bump" << std::endl;
	std::cout << "Q = Quit" << std::endl << std::endl;;
	
	//Get user selection for terrain to show
	bool validSelection = false;
	while (!validSelection) {
		std::cin >> selectedTerrain;
		if (selectedTerrain != '1' && selectedTerrain != '2' && selectedTerrain != '3' && selectedTerrain != '4' &&
			selectedTerrain != '5' && selectedTerrain != '6' && selectedTerrain != '7' && selectedTerrain != 'q' && selectedTerrain != 'Q') {
			std::cout << "Incorrect selection. Please try again" << std::endl;
		}
		else {
			validSelection = true;
			if (selectedTerrain == 'q' || selectedTerrain == 'Q') {
				exit(EXIT_SUCCESS);
			}
			break;
		}
	}

	//Get start location from user for deposition terrains
	if (selectedTerrain == '1' || selectedTerrain == '2') {
		std::cout << std::endl << "Select the start location:" << std::endl;
		std::cout << "C = Center of terrain" << std::endl;
		std::cout << "R = Random point on the terrain" << std::endl << std::endl;
		validSelection = false;
		while (!validSelection) {
			std::cin >> startLocation;
			if (startLocation != 'c' && startLocation != 'C' && startLocation != 'r' && startLocation != 'R') {
				std::cout << "Incorrect selection. Please try again" << std::endl;
			}
			else {
				validSelection = true;
				break;
			}
		}
	}

}

// Init Function
void init(void)
{
	getUserSelection();

	switch (selectedTerrain) {

	case '1':
		createParticleDepositionTerrain(startLocation);
		break;

	case '2':
		createRollDownParticleDepositionTerrain(startLocation);
		break;

	case '3':
		createSquareDiamondTerrain();
		break;

	case '4':
		createStepFaultTerrain();
		break;

	case '5':
		std::cout << "Sorry this has not been implemented yet" << std::endl;
		exit(EXIT_SUCCESS);
		break;

	case '6':
		std::cout << "Sorry this has not been implemented yet" << std::endl;
		exit(EXIT_SUCCESS);
		break;

	case '7':
		createCosineBumpTerrain();
		break;

	default:
		exit(EXIT_SUCCESS);
		break;

	}
		

	make_triangles();
	make_texture();


    // Initialize texture objects
    glGenTextures( 1, textures );

	//Load texture from image file
	int textureWidth = 655, textureHeight = 491;
	unsigned char* textureData = readTextureFile("rock.bmp", textureWidth, textureHeight);

    glBindTexture( GL_TEXTURE_2D, textures[0] );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, textureData );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    // Create and initialize buffer objects
    GLuint vbuffer;
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, triangles.size()*sizeof(vec3), triangles.data(), GL_STATIC_DRAW);

    GLuint tbuffer;
    glGenBuffers(1, &tbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tbuffer);
	glBufferData(GL_ARRAY_BUFFER, tex_coords.size()*sizeof(vec2), tex_coords.data(), GL_STATIC_DRAW);

    GLuint nbuffer;
    glGenBuffers(1, &nbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, nbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(vec3), normals.data(), GL_STATIC_DRAW);


    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);


    // Set up the arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint tPosition = glGetAttribLocation(program, "tPosition");
    glEnableVertexAttribArray(tPosition);
	glBindBuffer(GL_ARRAY_BUFFER, tbuffer);
	glVertexAttribPointer(tPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint nPosition = glGetAttribLocation(program, "nPosition");
    glEnableVertexAttribArray(nPosition);
	glBindBuffer(GL_ARRAY_BUFFER, nbuffer);
	glVertexAttribPointer(nPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));


    theta = glGetUniformLocation( program, "theta" );
	model_view = glGetUniformLocation( program, "model_view" );
	zoom = glGetUniformLocation( program, "zoom" );

    // Initialize shader lighting parameters
    vec4 light_ambient( 0.3, 0.3, 0.3, 1.0 );
    vec4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    vec4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    vec4 material_ambient( 0.5, 0.5, 0.5, 1.0 );
    vec4 material_diffuse( 0.5, 0.5, 0.5, 1.0 );
    vec4 material_specular( 0.7, 0.7, 0.7, 1.0 );
    float  material_shininess = 10.0;

    vec4 ambient_product = light_ambient * material_ambient;
    vec4 diffuse_product = light_diffuse * material_diffuse;
    vec4 specular_product = light_specular * material_specular;

    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );
    glUniform1f( glGetUniformLocation(program, "Shininess"),
		 material_shininess );

	glUniform1i( glGetUniformLocation(program, "texture"), 0 );
	glEnable( GL_DEPTH_TEST );
	glClearColor(0.05, 0.05, 0.1, 1.0);
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform3fv(theta, 1, Theta);
	glUniform4fv(model_view, 1, ModelView);
	glUniform4fv(zoom, 1, Zoom);

	glDrawArrays(GL_TRIANGLES, 0, triangles.size());
    glFlush();
}



void keyboard(unsigned char keyPressed, int x, int y)
{
	int keyModifier = glutGetModifiers();

	switch (keyPressed) {

    case 033:              // escape key
        exit(EXIT_SUCCESS);
        break;

	case 'l':
		ModelView[0] -= 0.025;
		break;
	case 'r':
		ModelView[0] += 0.025;
		break;
	case 'u':
		ModelView[1] += 0.025;
		break;
	case 'd':
		ModelView[1] -= 0.025;
		break;

	case 'z':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Zoom[0] *= 0.95;
			Zoom[1] *= 0.95;
			Zoom[2] *= 0.95;
		}
		else {
			Zoom[0] *= 1.05;
			Zoom[1] *= 1.05;
			Zoom[2] *= 1.05;
		}
		break;


	case 'Z':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Zoom[0] *= 0.95;
			Zoom[1] *= 0.95;
			Zoom[2] *= 0.95;
		}
		else {
			Zoom[0] *= 1.05;
			Zoom[1] *= 1.05;
			Zoom[2] *= 1.05;
		}
		break;

	case 'p':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[0] -= 1.5;
		}
		else {
			Theta[0] += 1.5;
		}
		break;


	case 'P':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[0] -= 1.5;
		}
		else {
			Theta[0] += 1.5;
		}
		break;

	case 'y':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[1] -= 1.5;
		}
		else {
			Theta[1] += 1.5;
		}
		break;

	case 'Y':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[1] -= 1.5;
		}
		else {
			Theta[1] += 1.5;
		}
		break;

	case 'o':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[2] -= 1.5;
		}
		else {
			Theta[2] += 1.5;
		}
		break;

	case 'O':
		if (keyModifier == GLUT_ACTIVE_SHIFT) {
			Theta[2] -= 1.5;
		}
		else {
			Theta[2] += 1.5;
		}
		break;

	}
}

void idle( void )
{
    glutPostRedisplay();
}



int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA );
	glutInitWindowSize(screenWidth, screenHeight);

	glutCreateWindow( "Terrains" );

    glewInit();
    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
	glutIdleFunc( idle );

    glutMainLoop();
    return 0;
}