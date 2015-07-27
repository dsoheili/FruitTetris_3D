/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// the index of current tile
int currTileIdx = 0;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = 
	{{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},     
	{vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

vec2 allRotationsIshape[2][4] = {
    {vec2(0,0), vec2(-1,0), vec2(1,0), vec2(-2,0)},
    {vec2(0,1), vec2(0,0), vec2(0,-1), vec2(0,-2)}};

vec2 allRotationsSshape[2][4] = {
    {vec2(0,0), vec2(0,-1), vec2(1,0), vec2(-1,-1)},
    {vec2(0,1), vec2(0,0), vec2(1,0), vec2(1,-1)}};

// colors
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
vec4 red    = vec4(1.0, 0.0, 0.0, 1.0); 
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); 
vec4 green  = vec4(0.0, 1.0, 0.0, 1.0);
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
// *6 because it's in 3D now
vec4 boardcolours[7200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

// Shader transformation matrices
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/#Cumulating_transformations___the_ModelViewProjection_matrix
mat4 Model;		//M
mat4 View;  	//V
mat4 Projection;//P
GLuint MatrixID;

vec2 newtilepos; // To be used for placing the tile at the tip of the robot arm

//--------------------- Imported from RobotArm/example1.cpp ---------------------
/*
 _____       _           _                          
 |  __ \     | |         | |                         
 | |__) |___ | |__   ___ | |_    __ _ _ __ _ __ ___  
 |  _  // _ \| '_ \ / _ \| __|  / _` | '__| '_ ` _ \ 
 | | \ \ (_) | |_) | (_) | |_  | (_| | |  | | | | | |
 |_|  \_\___/|_.__/ \___/ \__|  \__,_|_|  |_| |_| |_|
                                                     
*/
//-------------------------------------------------------------------------------- 

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 0.5, 0.0, 0.5, 1.0 ),  // purple
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// Shader transformation matrices
mat4  model_view;
// Luint ModelView;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  Quit = 4;
int Index = 0;

GLuint vao; // Moved here from initRobot() to make it a global variable

void
quad( int a, int b, int c, int d )
{
	// Modified to make the arm grey
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void
colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base(mat4 transform)
{
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
				BASE_HEIGHT,
				BASE_WIDTH ) );

    glUniformMatrix4fv( MatrixID, 1, GL_TRUE, transform * model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void upper_arm(mat4 transform)
{
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     	UPPER_ARM_HEIGHT,
			     	UPPER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( MatrixID, 1, GL_TRUE, transform * model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void lower_arm(mat4 transform)
{
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     	LOWER_ARM_HEIGHT,
			     	LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( MatrixID, 1, GL_TRUE, transform * model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
initRobot( void )
{
    colorcube();
    
    // Create a vertex array object
    // GLuint vao; Moved outside of function to make it global
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		  NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" ); //removed the 81 at the end of the names since we have different shader names
    glUseProgram( program );
    
  //  GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

 //   GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

/* Not needed since we do this rocess elsewhere later

    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
*/
}
//--------------------- Imported from RobotArm/example1.cpp ---------------------
/*
  _____       _           _                            ______ _   _ _____  
 |  __ \     | |         | |                          |  ____| \ | |  __ \ 
 | |__) |___ | |__   ___ | |_    __ _ _ __ _ __ ___   | |__  |  \| | |  | |
 |  _  // _ \| '_ \ / _ \| __|  / _` | '__| '_ ` _ \  |  __| | . ` | |  | |
 | | \ \ (_) | |_) | (_) | |_  | (_| | |  | | | | | | | |____| |\  | |__| |
 |_|  \_\___/|_.__/ \___/ \__|  \__,_|_|  |_| |_| |_| |______|_| \_|_____/ 
                                                                                                                                                    
*/
//-------------------------------------------------------------------------------- 
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// Before we place a tile, we need to compute where the "hand" or tip of the robot arm is

vec3 armPosition = vec3(-10, 0 ,0);

void tilePosition(){

	float pi = 3.14159265359;

	newtilepos.x = - cos(pi/180 * (180/2 - (Theta[LowerArm] + Theta[UpperArm]))) * UPPER_ARM_HEIGHT
                   - sin(pi/180 * Theta[LowerArm]) * LOWER_ARM_HEIGHT
	               + armPosition.x/2;
				 
				 

	newtilepos.y = 	sin(pi/180 * (180/2 - (Theta[LowerArm] + Theta[UpperArm]))) * UPPER_ARM_HEIGHT
				  + cos((-pi/180) * Theta[LowerArm]) * LOWER_ARM_HEIGHT
	 			  + armPosition.y + BASE_HEIGHT;

}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	tilePosition();
	tilepos = newtilepos;

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		// Since we are dealing with cubes now, we need more than just 4 corners, we need 8.
		// The new 4 vec will act as the back
		// for the third value in the vec4, we use 16.5 because 33/2 = 16.5 which is the centre
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1); // bottom left
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1); // top left
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1); // bottom right
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1); // top right

		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); // bottom left
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1); // top left
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); // bottom right
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1); // top right

		// Two points are used by two triangles each
		//(6 faces)(2 triangles/face)(3 vertices/triangle)
		// To understand this better, imagine the two sets of vec4s as two parallel squares
		// The following constructs triangles on the 6 sides of it to make a cube
		vec4 newpoints[36] = {p1, p2, p3, p2, p3, p4, // ◣, ◥ on the front
							  p3, p4, p7, p4, p7, p8, // right side of the cube
							  p5, p6, p7, p6, p7, p8, // behind the cube
							  p1, p2, p5, p2, p6, p5, // left side
							  p1, p5, p3, p5, p7, p3, // bottom of the cube
							  p2, p6, p4, p6, p8, p4, // top of the cube
							 }; 



		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*36*sizeof(vec4), 36*sizeof(vec4), newpoints); 
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	tilePosition();
	tilepos = newtilepos;//vec2(5 , 10); // Put the tile at the top of the board

	// Update the geometry VBO of current tile
	for (int i = 0; i < 4; i++)
		tile[i] = allRotationsLshape[0][i]; // Get the 4 pieces of the new tile
	updatetile(); 

	// Update the color VBO of current tile
	// Only a minor change here from skeleton code
	// Multiply 2 by 6 because we have 6 faces to deal with
	vec4 newcolours[6*24];
	for (int i = 0; i < 6*24; i++)
		newcolours[i] = orange; // You should randomlize the color
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[590];
	vec4 gridcolours[590];

	// Vertical lines for the front
	for (int i = 0; i < 11; i++){
		gridpoints[2*i 	  ] = vec4((33.0 + (33.0 * i)), 33.0, 16.50, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 16.50, 1);
	}

	// Vertical lines for the back
	for (int i = 0; i < 11; i++){
		gridpoints[64 + 2*i] = vec4((33.0 + (33.0 * i)), 33.0, -16.50, 1);
		gridpoints[65 + 2*i] = vec4((33.0 + (33.0 * i)), 693.0, -16.50, 1);
	}

	// Horizontal lines for the front
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 16.50, 1);
	}
	// Horizontal lines or the back
	for (int i = 0; i < 21; i++){
		gridpoints[64 + 22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), -16.50, 1);
		gridpoints[65 + 22 + 2*i] = vec4(363.0, (33.0 + (33.0 * i)), -16.50, 1);
	}

	// Add lines to connect the two boards
	for (int i = 0; i < 20 + 1; i++){
		for (int k = 0; k < 10 + 1; k++) {
			gridpoints[2*(64 + 11*i + k) + 0] = vec4(33.0 + (k * 33.0), 33.0 + (i * 33.0), 16.50, 1);
			gridpoints[2*(64 + 11*i + k) + 1] = vec4(33.0 + (k * 33.0), 33.0 + (i * 33.0), -16.50, 1);
		}
	}
	// Make all grid lines coloured
	for (int i = 0; i < 590; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, (590)*sizeof(vec4), gridpoints, GL_DYNAMIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition); // Enable the attribute

	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, (590)*sizeof(vec4), gridcolours, GL_DYNAMIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
// *** Generate the geometric data
	// 6 faces, hence 6*1200 = 7200
	vec4 boardpoints[7200];
	for (int i = 0; i < 7200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			// Since we are dealing with cubes, set the z value to 33 instead of 0 which was used for 2D
			// Now split it in half evenly since we have two sets of points
			// Give one set 33/2 = 16.5, and the other -16.5
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), 16.5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), 16.5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), 16.5, 1);

			vec4 p5 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.5, 1);
			vec4 p6 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), -16.5, 1);
			vec4 p7 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), -16.5, 1);
			vec4 p8 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), -16.5, 1);

			
			// Two points are reused
			// Since we are dealing with cubes, we need 6 faces instead of one
			// Each face consists of two triangles

			// Reminder, the way it works is that there are two parallel squares that when connected form a cube
			// Front square:
			// p1 = bottom left; p2 = top left; p3 = bottom right; p4 = top right;
			// Back square:
			// p5 = bottom left; p6 = top left; p7 = bottom right; p8 = top right;
			
			int count = 0; // using this we move to the next point in the loop from 0 to 35 (6*6 = 36)
			
			// Front
			boardpoints[36*(10*i + j) + count] = p1; count++;
			boardpoints[36*(10*i + j) + count] = p2; count++;
			boardpoints[36*(10*i + j) + count] = p3; count++;
			boardpoints[36*(10*i + j) + count] = p2; count++;
			boardpoints[36*(10*i + j) + count] = p3; count++;
			boardpoints[36*(10*i + j) + count] = p4; count++;

			// Back
			boardpoints[36*(10*i + j) + count] = p5; count++;
			boardpoints[36*(10*i + j) + count] = p6; count++;
			boardpoints[36*(10*i + j) + count] = p7; count++;
			boardpoints[36*(10*i + j) + count] = p6; count++;
			boardpoints[36*(10*i + j) + count] = p7; count++;
			boardpoints[36*(10*i + j) + count] = p8; count++;

			// Left side
			boardpoints[36*(10*i + j) + count] = p1; count++;
			boardpoints[36*(10*i + j) + count] = p2; count++;
			boardpoints[36*(10*i + j) + count] = p5; count++;
			boardpoints[36*(10*i + j) + count] = p2; count++;
			boardpoints[36*(10*i + j) + count] = p5; count++;
			boardpoints[36*(10*i + j) + count] = p6; count++;

			// Right side
			boardpoints[36*(10*i + j) + count] = p3; count++;
			boardpoints[36*(10*i + j) + count] = p4; count++;
			boardpoints[36*(10*i + j) + count] = p7; count++;
			boardpoints[36*(10*i + j) + count] = p4; count++;
			boardpoints[36*(10*i + j) + count] = p7; count++;
			boardpoints[36*(10*i + j) + count] = p8; count++;

			// Top
			boardpoints[36*(10*i + j) + count] = p2; count++;
			boardpoints[36*(10*i + j) + count] = p4; count++;
			boardpoints[36*(10*i + j) + count] = p6; count++;
			boardpoints[36*(10*i + j) + count] = p4; count++;
			boardpoints[36*(10*i + j) + count] = p6; count++;
			boardpoints[36*(10*i + j) + count] = p8; count++;

			// Bottom
			boardpoints[36*(10*i + j) + count] = p1; count++;
			boardpoints[36*(10*i + j) + count] = p3; count++;
			boardpoints[36*(10*i + j) + count] = p5; count++;
			boardpoints[36*(10*i + j) + count] = p3; count++;
			boardpoints[36*(10*i + j) + count] = p5; count++;
			boardpoints[36*(10*i + j) + count] = p7; count++;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 6*24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // 6 faces so multiply by 6
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 6*24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // multiply by 6 here as well
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/#Cumulating_transformations___the_ModelViewProjection_matrix
	MatrixID = glGetUniformLocation(program, "vMVP");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();
	initRobot();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Robot arm angles
	Theta[LowerArm] = -10; // Give the arm its initial angles
	Theta[UpperArm] = -55;

	// Set up camera
	vec3 eye	= vec3(-25, 25, 25);
	vec3 center = vec3(0, 10, 0);
	vec3 up 	= vec3(0, 1, 0);
	View = LookAt(eye, center, up);

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
    currTileIdx = (currTileIdx + 1) % 4; // Get the next rotation value
    for (int i = 0; i < 4; i++)
        tile[i] = allRotationsLshape[currTileIdx][i]; // Change the 4 cells of the tile to correspond to the new orientation
        
    updatetile(); // Update the current tile vertex position VBO
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{

}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	// ------------- Imported from Assignment 1 and then modified  --------------------
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]); // Bind the VBO containing current tile vertex colours

	// Colour the board similar to the way it was done on initBoard() but with the tile colours
	for (int i = 0; i < 4; i++){

		// (From updateTile)
		// Calculate the grid coordinates of the cell
		int x = tilepos.x + tile[i].x; 
		int y = tilepos.y + tile[i].y;

		// (From initBoard)
		// Paint the board with each of the block's tiles/squares
		for (int j = 0; j < 36; j++)
			boardcolours[36*(10*y + x) + j] = orange;


		// Now set the values on board[][] to true to indicatee the blocks are occupied
		board[x][y] = true;

	}
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);



	//---------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	return false;
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	initBoard(); // re-initialize the game board
	newtile();   // drop a new tile for the player
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat left = -10.0, right = 10.0, bottom = -15, top = 15, zNear = 50, zFar = -40; 
	Projection = Ortho( left, right, bottom, top, zNear, zFar );

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	mat4 MVPmatrixRobot = Projection * View * Translate(armPosition);

	// From RobotArm/ example1.cpp
	//	-------------------------------------------------
	glBindVertexArray(vao);

	model_view = RotateY(Theta[Base] );
    base(MVPmatrixRobot); // Added a parameter to the robot parts functions to make multiplication easier

    model_view *= ( Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LowerArm]) );
    lower_arm(MVPmatrixRobot);

    model_view *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) * RotateZ(Theta[UpperArm]) );
    upper_arm(MVPmatrixRobot);

	model_view *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0);
	// -------------------------------------------------

	// Do the same for the board
	mat4 Model = mat4();
	Model *= ( Translate(0, 10, 0) * Scale(1.0/33, 1.0/33, 1.0/33) * Translate(-198, -363, 0) );


	mat4 MVPmatrix = Projection * View * Model;
	glUniformMatrix4fv(MatrixID, 1, GL_TRUE, MVPmatrix);

	// Display the board, tiles, and the grid
	glBindVertexArray(vaoIDs[1]);
	glDrawArrays(GL_TRIANGLES, 0, 7200);
	glBindVertexArray(vaoIDs[2]);
	glDrawArrays(GL_TRIANGLES, 0, 6*24);
	glBindVertexArray(vaoIDs[0]);
	glDrawArrays(GL_LINES, 0, 590);


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            rotate(); // Up key rotates the current tile
            break;
        case GLUT_KEY_RIGHT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View = View * RotateY(25);
			break;
		case GLUT_KEY_LEFT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View = View * RotateY(-25);
		break;
    }
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case 'a':
			Theta[LowerArm] = Theta[LowerArm] + 10;
			updatetile();
			break;
		case 'd':
			Theta[LowerArm] = Theta[LowerArm] - 10;
			updatetile();
			break;
		case 'w':
			Theta[UpperArm] = Theta[UpperArm] + 10;
			updatetile();
			break;
		case 's':
			Theta[UpperArm] = Theta[UpperArm] - 10;
			updatetile();
			break;
		case ' ':
			settile();
			newtile();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris 3D");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
