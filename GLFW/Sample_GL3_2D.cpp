#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
  glfwDestroyWindow(window);
  glfwTerminate();
  //    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
      0,                  // attribute 0. Vertices
      3,                  // size (x,y,z)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
      1,                  // attribute 1. Color
      3,                  // size (r,g,b)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
  GLfloat* color_buffer_data = new GLfloat [3*numVertices];
  for (int i=0; i<numVertices; i++) {
    color_buffer_data [3*i] = red;
    color_buffer_data [3*i + 1] = green;
    color_buffer_data [3*i + 2] = blue;
  }

  return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Enable Vertex Attribute 1 - Color
  glEnableVertexAttribArray(1);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

typedef struct object_type
{
  float height,width,length;
  float rotate;
  int rotate_check;
  glm ::vec3 center ;
  // glm :: vec3 translate_vector;
  glm :: vec3 rotate_vector;
  glm :: vec3 color;
  VAO * coordinates;
}object_type;

object_type block,board_pieces[10][10],cam;

int no_of_moves=0;
int level=1;
int switch_check=0;
int x_check=-1;
int z_check=-1;
int x1_check=-1;
int z1_check=-1;
int game_check=0;
int v_eye[]={4,4,6};

int board[10][10];
int board_1[10][10]={
  //0 1 2 3 4 5 6 7 8 9
  { 1,1,1,1,1,1,0,0,0,0}, //0
  { 1,1,1,1,1,1,0,0,6,0},//1
  { 1,1,0,0,1,1,0,0,1,0}, //2
  { 1,1,0,0,1,1,1,1,1,0},//3
  { 1,1,0,0,0,1,0,1,1,1}, //4
  { 1,1,0,0,0,1,0,1,1,1}, //5
  { 1,1,0,0,0,0,0,1,1,1} ,//6
  { 1,1,0,0,0,0,0,1,1,0} ,//7
  { 1,1,1,1,1,1,1,1,1,0} ,//8
  { 1,1,1,1,1,1,1,1,1,0} //9
};

int board_2[10][10]={
  //0 1 2 3 4 5 6 7 8 9
  {0,0,0,0,0,0,0,0,0,0}, //0
  {0,1,1,1,1,1,1,0,6,0}, //1
  {0,1,1,1,1,1,1,0,1,0}, //2
  {1,1,1,0,0,3,0,1,1,1}, //3
  {1,1,1,0,0,3,0,1,1,1}, //4
  {1,1,1,0,0,1,0,1,1,1}, //5
  {0,1,1,0,0,1,0,1,1,1}, //6
  {0,1,1,0,0,4,0,1,1,1}, //7
  {0,1,1,1,1,1,1,1,1,0}, //8
  {0,1,1,1,1,1,1,1,1,0} //9
};


int board_3[10][10]={
  //0 1 2 3 4 5 6 7 8 9
  {0,0,0,1,1,1,0,0,0,0},//0
  {0,0,1,1,1,1,0,0,6,0},//1
  {1,1,1,1,1,1,0,0,1,0},//2
  {2,2,1,0,0,3,0,1,1,0},//3
  {1,1,1,0,0,3,0,1,1,0},//4
  {0,1,1,0,0,1,0,1,1,0},//5
  {1,1,1,0,0,4,0,1,1,0},//6
  {2,2,1,1,1,1,1,1,1,0},//7
  {1,1,1,1,1,1,1,1,1,1},//8
  {0,0,0,1,1,0,0,0,0,0}
};



/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Function is called first on GLFW_PRESS.

  int i;

  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_C:
        rectangle_rot_status = !rectangle_rot_status;
        break;
      case GLFW_KEY_P:
        triangle_rot_status = !triangle_rot_status;
        break;
      case GLFW_KEY_X:
        // do something ..
        break;
      case GLFW_KEY_LEFT:
        block.rotate_check=-1;
        block.rotate_vector= glm :: vec3(0,0,1);
        block.rotate=0;
        break;
      case GLFW_KEY_RIGHT:
        block.rotate_check=1;
        block.rotate_vector= glm :: vec3(0,0,-1);
        block.rotate=0;
        break;
      case GLFW_KEY_UP:
        block.rotate_check=2;
        block.rotate_vector= glm :: vec3(-1,0,0);
        block.rotate=0;
        break;
      case GLFW_KEY_DOWN:
        block.rotate_check=-2;
        block.rotate_vector= glm :: vec3(1,0,0);
        block.rotate=0;
        break;
      case GLFW_KEY_D:
        v_eye[0]=4;
        v_eye[1]=4;
        v_eye[2]=6;
        break;
      case GLFW_KEY_A:
        v_eye[0]=-4;
        v_eye[1]=4;
        v_eye[2]=6;
        break;
      case GLFW_KEY_W:
        v_eye[0]=4;
        v_eye[1]=4;
        v_eye[2]=-6;
        break;
      case GLFW_KEY_S:
        v_eye[0]=-4;
        v_eye[1]=4;
        v_eye[2]=-6;
        break;
      default:
        break;

        /*
           case GLFW_KEY_D:
           cam.rotate_check=1;
           angle=angle+90;
           if(angle>180);
           angle=180-angle;
           if(angle<-180);
           angle=180+angle;
           break;
           case GLFW_KEY_A:
           cam.rotate_check=-1;
           angle=angle-90;
           break;
           */
    }
  }
  else if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        quit(window);
        break;
      default:
        break;
    }
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
      quit(window);
      break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      if (action == GLFW_RELEASE)
        triangle_rot_dir *= -1;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_RELEASE) {
        rectangle_rot_dir *= -1;
      }
      break;
    default:
      break;
  }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
  // Perspective projection for 3D views
  // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

  // Ortho projection for 2D views
  Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCam ()
{
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };
  cam.center= glm :: vec3(4,4,6);
  cam.rotate=0;
  cam.rotate_check=0;

  // create3DObject creates and returns a handle to a VAO that can be used later
  cam.coordinates = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}



void createblock(float x, float y,float z, float l ,float h, float b,float c1, float c2,float c3)
{

  GLfloat vertex_buffer_data[] = {

    -l/2,-h/2,b/2,   //1
    l/2,-h/2,b/2,     //2
    l/2,h/2,b/2,     //5
    l/2,h/2,b/2,     //5
    -l/2,h/2,b/2,     //6
    -l/2,-h/2,b/2,   //1


    l/2,-h/2,b/2,     //2
    l/2,-h/2,-b/2,     //3
    l/2,h/2,-b/2,     //8
    l/2,h/2,-b/2,     //8
    l/2,h/2,b/2,     //5
    l/2,-h/2,b/2,     //2

    l/2,h/2,-b/2,     //8
    -l/2,h/2,-b/2,     //7
    -l/2,-h/2,-b/2,     //4
    -l/2,-h/2,-b/2,     //4
    l/2,-h/2,-b/2,     //3
    l/2,h/2,-b/2,     //8

    -l/2,h/2,-b/2,     //7
    -l/2,h/2,b/2,     //6
    -l/2,-h/2,b/2,   //1
    -l/2,-h/2,b/2,   //1
    -l/2,-h/2,-b/2,     //4
    -l/2,h/2,-b/2,     //7


    -l/2,-h/2,b/2,   //1
    l/2,-h/2,b/2,     //2
    l/2,-h/2,-b/2,     //3
    l/2,-h/2,-b/2,     //3
    -l/2,-h/2,-b/2,     //4
    -l/2,-h/2,b/2,   //1


    -l/2,h/2,b/2,     //6
    l/2,h/2,b/2,     //5
    l/2,h/2,-b/2,     //8
    l/2,h/2,-b/2,     //8
    -l/2,h/2,-b/2,     //7
    -l/2,h/2,b/2     //6


  };

  GLfloat color_buffer_data[]=
  {
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,


    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,

    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,


    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,

    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,


    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2,
    0.2,1,0.2
  };

  block.coordinates=create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  block.center= glm :: vec3(x,y,z);
  block.length=l;
  block.height=h;
  block.width=b;
  block.color= glm :: vec3(c1,c2,c3);
  block.rotate_vector= glm :: vec3(0,0,1);
}


VAO * createPiece(float a,int type)
{

  GLfloat vertex_buffer_data[] =
  {

    -a/2,-a/4,a/2,   //1
    a/2,-a/4,a/2,     //2
    a/2,a/4,a/2,     //5
    a/2,a/4,a/2,     //5
    -a/2,a/4,a/2,     //6
    -a/2,-a/4,a/2,   //1


    a/2,-a/4,a/2,     //2
    a/2,-a/4,-a/2,     //3
    a/2,a/4,-a/2,     //8
    a/2,a/4,-a/2,     //8
    a/2,a/4,a/2,     //5
    a/2,-a/4,a/2,     //2

    a/2,a/4,-a/2,     //8
    -a/2,a/4,-a/2,     //7
    -a/2,-a/4,-a/2,     //4
    -a/2,-a/4,-a/2,     //4
    a/2,-a/4,-a/2,     //3
    a/2,a/4,-a/2,     //8

    -a/2,a/4,-a/2,     //7
    -a/2,a/4,a/2,     //6
    -a/2,-a/4,a/2,   //1
    -a/2,-a/4,a/2,   //1
    -a/2,-a/4,-a/2,     //4
    -a/2,a/4,-a/2,     //7


    -a/2,-a/4,a/2,   //1
    a/2,-a/4,a/2,     //2
    a/2,-a/4,-a/2,     //3
    a/2,-a/4,-a/2,     //3
    -a/2,-a/4,-a/2,     //4
    -a/2,-a/4,a/2,   //1


    -a/2,a/4,a/2,     //6
    a/2,a/4,a/2,     //5
    a/2,a/4,-a/2,     //8
    a/2,a/4,-a/2,     //8
    -a/2,a/4,-a/2,     //7
    -a/2,a/4,a/2     //6

  };
  int i;
  if(type==1)
  {
    GLfloat color_buffer_data[]=
    {
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      1,0,0,
      0,0,0,
      1,0,0,
      1,0,0,
      0,0,0,
      1,0,0,

      1,0,0,
      0,0,0,
      1,0,0,
      1,0,0,
      0,0,0,
      1,0,0
    };
    return create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  }
  else if(type==6)
  {
    GLfloat color_buffer_data[]=
    {
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,
      0.6,0,0,

      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0,

      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0,
      0,0,0
    };
    return create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  }
  else if(type==4)
  {
    GLfloat color_buffer_data[]=
    {
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,

      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,

      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,

      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,
      0,0.6,0,

      0.2,1,0.2,
      0,0,0,
      0.2,1,0.2,
      0.2,1,0.2,
      0,0,0,
      0.2,1,0.2,

      0.2,1,0.2,
      0,0,0,
      0.2,1,0.2,
      0.2,1,0.2,
      0,0,0,
      0.2,1,0.2,

    };
    return create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  }
  else if(type==3)
  {
    GLfloat color_buffer_data[]=
    {
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,

      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,

      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,

      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,
      0,0,0.6,

      0.2,0.2,1,
      0,0,0,
      0.2,0.2,1,
      0.2,0.2,1,
      0,0,0,
      0.2,0.2,1,

      0.2,0.2,1,
      0,0,0,
      0.2,0.2,1,
      0.2,0.2,1,
      0,0,0,
      0.2,0.2,1,

    };
    return create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  }
  else if(type==2)
  {
    GLfloat color_buffer_data[]=
    {
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,

      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,

      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,

      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,
      0.5,0.5,0.5,

      0.9,0.9,0.9,
      0,0,0,
      0.9,0.9,0.9,
      0.9,0.9,0.9,
      0,0,0,
      0.9,0.9,0.9,

      0.9,0.9,0.9,
      0,0,0,
      0.9,0.9,0.9,
      0.9,0.9,0.9,
      0,0,0,
      0.9,0.9,0.9,

    };
    return create3DObject(GL_TRIANGLES,6*2*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  }

}

void createBoard()
{
  int i;
  int j;
  if(level==1)
  {
    for(i=0;i<10;i++)
      for(j=0;j<10;j++)
        board[i][j]=board_1[i][j];
  }
  else if(level==2)
  {
    for(i=0;i<10;i++)
      for(j=0;j<10;j++)
        board[i][j]=board_2[i][j];
  }
  else if(level==3)
  {
    for(i=0;i<10;i++)
      for(j=0;j<10;j++)
        board[i][j]=board_3[i][j];
  }


  for(i=0;i<10;i++)
  {
    for(j=0;j<10;j++)
    {
      if(board[i][j]!=0)
      {
        board_pieces[i][j].coordinates=createPiece(0.5,board[i][j]);
        board_pieces[i][j].center=glm :: vec3(-2.5+i*0.5,-0.5-0.125,2.5-j*0.5);
      }
    }
  }
}



float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
int base_check=1;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(v_eye[0],v_eye[1],v_eye[2]), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  //  Matrices.view = glm::lookAt(eye,target,glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  int i=0;

  if(block.rotate_check==0 && game_check>-1)
  {
    Matrices.model = glm::mat4(1.0f);

    glm::mat4 translateBlock = glm::translate (block.center);        // glTranslatef
    glm::mat4 rotateBlock = glm::rotate((float)(block.rotate*M_PI/180.0f), block.rotate_vector); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBlock * rotateBlock);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(block.coordinates);
    Matrices.model = glm::mat4(1.0f);
  }

  if(block.rotate_check==-1 && game_check>-1)
  {
    Matrices.model = glm::mat4(1.0f);
    //  printf("center_left_draw: %.2f %.2f %.2f\n",block.center[0],block.center[1],block.center[2]);
    glm::mat4 translateBlock = glm::translate (block.center);        // glTranslatef
    glm::mat4 translateBlock1 = glm::translate (glm:: vec3 (block.length/2,block.height/2,-block.width/2));        // glTranslatef
    glm::mat4 translateBlock2 = glm::translate (glm:: vec3 (-block.length/2,-block.height/2,block.width/2));        // glTranslatef
    glm::mat4 rotateBlock = glm::rotate((float)(block.rotate*M_PI/180.0f), block.rotate_vector); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBlock* translateBlock2* rotateBlock*translateBlock1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(block.coordinates);
    Matrices.model = glm::mat4(1.0f);
  }

  if(block.rotate_check==1 && game_check>-1)
  {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBlock = glm::translate (block.center);        // glTranslatef
    glm::mat4 translateBlock1 = glm::translate (glm:: vec3 (-block.length/2,block.height/2,block.width/2));        // glTranslatef
    glm::mat4 translateBlock2 = glm::translate (glm:: vec3 (block.length/2,-block.height/2,-block.width/2));        // glTranslatef
    glm::mat4 rotateBlock = glm::rotate((float)(block.rotate*M_PI/180.0f), block.rotate_vector); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBlock* translateBlock2* rotateBlock*translateBlock1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(block.coordinates);
    Matrices.model = glm::mat4(1.0f);
  }

  if(block.rotate_check==2 && game_check>-1)
  {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBlock = glm::translate (block.center);        // glTranslatef
    glm::mat4 translateBlock1 = glm::translate (glm:: vec3 (block.length/2,block.height/2,block.width/2));        // glTranslatef
    glm::mat4 translateBlock2 = glm::translate (glm:: vec3 (-block.length/2,-block.height/2,-block.width/2));        // glTranslatef
    glm::mat4 rotateBlock = glm::rotate((float)(block.rotate*M_PI/180.0f), block.rotate_vector); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBlock* translateBlock2* rotateBlock*translateBlock1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(block.coordinates);
    Matrices.model = glm::mat4(1.0f);
  }

  if(block.rotate_check==-2 && game_check>-1)
  {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateBlock = glm::translate (block.center);        // glTranslatef
    glm::mat4 translateBlock1 = glm::translate (glm:: vec3 (-block.length/2,block.height/2,-block.width/2));        // glTranslatef
    glm::mat4 translateBlock2 = glm::translate (glm:: vec3 (block.length/2,-block.height/2,block.width/2));        // glTranslatef
    glm::mat4 rotateBlock = glm::rotate((float)(block.rotate*M_PI/180.0f), block.rotate_vector); // rotate about vector (-1,1,1)
    Matrices.model *= (translateBlock* translateBlock2* rotateBlock*translateBlock1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(block.coordinates);
    Matrices.model = glm::mat4(1.0f);
  }


  if(block.rotate<90 && block.rotate_check!=0)
  {
    block.rotate=block.rotate+5*2;
  }
  else
  {
    //change coordinates
    if(block.rotate_check==-1)
    {
      block.center[0]=block.center[0]-block.length/2-block.height/2;
      if(block.length!=block.height)
        block.center[1]=block.center[1]-(0.25)*base_check;
      block.height=block.length- block.height;
      block.length=block.length -block.height;
      block.height=block.height+ block.length;
      if(block.height!=1)
        base_check=-1;
      else
        base_check=1;
      block.rotate_check=0;
      block.rotate=0;
      if(block.height==1)
      {
        x_check=(block.center[0]+2.5)/0.5;
        z_check=(-block.center[2]+2.5)/0.5;

        if(board[x_check][z_check]==0 || board[x_check][z_check]==2)
        {
          no_of_moves++;
          game_check=-1;
          printf("Game Over\n");
          printf("%d %d\n",x_check,z_check);
          printf("You Lost\n");
          // kill_bloc();
          printf("End_Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==6)
        {
          game_check=1;
          level++;
          system("play stage_clear.wav");
          no_of_moves++;
          //show display
          printf("End_Score:%d\n",no_of_moves);
          printf("Next_Level:%d\n",level);
          // createBoard();
        }
        else if(board[x_check][z_check]==4)
        {
          if(switch_check==0)
            switch_check=1;
          else if (switch_check==1)
            switch_check=0;
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==1)
        {
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
      }
      if(block.height==0.5)
      {
        if(block.length==1)
        {
          x_check=(block.center[0]+0.25+2.5)/0.5;
          z_check=(-block.center[2]+2.5)/0.5;
          x1_check=(block.center[0]-0.25+2.5)/0.5;
          z1_check=(-block.center[2]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            // printf("lenght:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
            printf("You Lost!!\n");
            printf("End_Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              //  printf("lenght:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
        if(block.width==1)
        {
          z_check=(-block.center[2]+0.25+2.5)/0.5;
          x_check=(block.center[0]+2.5)/0.5;
          z1_check=(-block.center[2]-0.25+2.5)/0.5;
          x1_check=(block.center[0]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            // printf("width:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
            printf("You Lost\n");
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              //  printf("lenght:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
      }
      createblock(block.center[0],block.center[1],block.center[2],block.length,block.height,block.width,1,1,1);
      //  printf("new_rotate:%f\n",block.rotate);                      
    }
    if(block.rotate_check==1)
    {
      //    printf("length,height:%f %f\n",block.length,block.height);                      
      //   printf("rotate:%f\n",block.rotate);                      
      block.center[0]=block.center[0]+block.length/2+block.height/2;
      if(block.length!=block.height)
        block.center[1]=block.center[1]-(0.25)*base_check;
      //  printf("center_right: %.2f %.2f %.2f\n",block.center[0],block.center[1],block.center[2]);
      block.height=block.length- block.height;
      block.length=block.length -block.height;
      block.height=block.height+ block.length;
      // printf("new l,h,b:%f %f %f\n",block.length,block.height,block.width);                      
      if(block.height!=1)
        base_check=-1;
      else
        base_check=1;
      block.rotate_check=0;
      block.rotate=0;
      if(block.height==1)
      {
        x_check=(block.center[0]+2.5)/0.5;
        z_check=(-block.center[2]+2.5)/0.5;

        if(board[x_check][z_check]==0 || board[x_check][z_check]==2)
        {
          no_of_moves++;
          game_check=-1;
          printf("Game Over\n");
          //  printf("%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
          printf("You Lost\n");
          printf("End_Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==6)
        {
          game_check=1;
          level++;
          system("play stage_clear.wav");
          no_of_moves++;
          //show display
          printf("End_Score:%d\n",no_of_moves);
          printf("Next_Level:%d\n",level);
          // createBoard();
        }
        else if(board[x_check][z_check]==1)
        {
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==4)
        {
          if(switch_check==0)
            switch_check=1;
          else if (switch_check==1)
            switch_check=0;
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
      }
      if(block.height==0.5)
      {
        if(block.length==1)
        {
          x_check=(block.center[0]+0.25+2.5)/0.5;
          z_check=(-block.center[2]+2.5)/0.5;
          x1_check=(block.center[0]-0.25+2.5)/0.5;
          z1_check=(-block.center[2]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            // printf("%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
            printf("You Lost\n");
            printf("End_Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              // printf("lenght:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
        if(block.width==1)
        {
          z_check=(-block.center[2]+0.25+2.5)/0.5;
          x_check=(block.center[0]+2.5)/0.5;
          z1_check=(-block.center[2]-0.25+2.5)/0.5;
          x1_check=(block.center[0]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            // printf("%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
            printf("You Lost\n");
          }
          if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              // printf("lenght:%d %d %d %d\n",x_check,z_check,x1_check,z1_check);
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
      }
      createblock(block.center[0],block.center[1],block.center[2],block.length,block.height,block.width,1,1,1);
    }
    if(block.rotate_check==2)
    {
      //    printf("length,height:%f %f\n",block.length,block.height);                      
      //   printf("rotate:%f\n",block.rotate);                      
      block.center[2]=block.center[2]-block.width/2-block.height/2;
      if(block.width!=block.height)
        block.center[1]=block.center[1]-(0.25)*base_check;
      //  printf("center_up: %.2f %.2f %.2f\n",block.center[0],block.center[1],block.center[2]);
      block.height=block.width- block.height;
      block.width=block.width -block.height;
      block.height=block.height+ block.width;
      //  printf("new l,h,b:%f %f %f\n",block.length,block.height,block.width);                      
      if(block.height!=1)
        base_check=-1;
      else
        base_check=1;
      block.rotate_check=0;
      block.rotate=0;
      if(block.height==1)
      {
        x_check=(block.center[0]+2.5)/0.5;
        z_check=(-block.center[2]+2.5)/0.5;

        if(board[x_check][z_check]==0 || board[x_check][z_check]==2)
        {
          no_of_moves++;
          game_check=-1;
          printf("Game Over\n");
          printf("You Lost\n");
          printf("End_Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==6)
        {
          game_check=1;
          level++;
          system("play stage_clear.wav");
          no_of_moves++;
          //show display
          printf("End_Score:%d\n",no_of_moves);
          printf("Next_Level:%d\n",level);
          //createBoard();
        }
        else if(board[x_check][z_check]==1)
        {
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==4 )
        {
          if(switch_check==0)
            switch_check=1;
          else if (switch_check==1)
            switch_check=0;
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
      }
      if(block.height==0.5)
      {
        if(block.length==1)
        {
          x_check=(block.center[0]+0.25+2.5)/0.5;
          z_check=(-block.center[2]+2.5)/0.5;
          x1_check=(block.center[0]-0.25+2.5)/0.5;
          z1_check=(-block.center[2]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            printf("You Lost\n");
            printf("End_Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
        if(block.width==1)
        {
          z_check=(-block.center[2]+0.25+2.5)/0.5;
          x_check=(block.center[0]+2.5)/0.5;
          z1_check=(-block.center[2]-0.25+2.5)/0.5;
          x1_check=(block.center[0]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            printf("You Lost\n");
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
      }
      createblock(block.center[0],block.center[1],block.center[2],block.length,block.height,block.width,1,1,1);
    }
    if(block.rotate_check==-2)
    {
      //    printf("length,height:%f %f\n",block.length,block.height);                      
      //   printf("rotate:%f\n",block.rotate);                      
      block.center[2]=block.center[2]+block.width/2+block.height/2;
      if(block.width!=block.height)
        block.center[1]=block.center[1]-(0.25)*base_check;
      block.height=block.width- block.height;
      block.width=block.width -block.height;
      block.height=block.height+ block.width;
      if(block.height!=1)
        base_check=-1;
      else
        base_check=1;
      block.rotate_check=0;
      block.rotate=0;
      if(block.height==1)
      {
        x_check=(block.center[0]+2.5)/0.5;
        z_check=(-block.center[2]+2.5)/0.5;

        if(board[x_check][z_check]==0 || board[x_check][z_check]==2)
        {
          no_of_moves++;
          game_check=-1;
          printf("Game Over\n");
          printf("End_Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==6)
        {
          game_check=1;
          level++;
          system("play stage_clear.wav");
          no_of_moves++;
          //show display
          printf("End_Score:%d\n",no_of_moves);
          printf("Next_Level:%d\n",level);
          // createBoard();
        }
        else if(board[x_check][z_check]==4)
        {
          if(switch_check==0)
            switch_check=1;
          else if (switch_check==1)
            switch_check=0;
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
        else if(board[x_check][z_check]==1)
        {
          no_of_moves++;
          printf("Score:%d\n",no_of_moves);
        }
      }
      if(block.height==0.5)
      {
        if(block.length==1)
        {
          x_check=(block.center[0]+0.25+2.5)/0.5;
          z_check=(-block.center[2]+2.5)/0.5;
          x1_check=(block.center[0]-0.25+2.5)/0.5;
          z1_check=(-block.center[2]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
            printf("End_Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
        if(block.width==1)
        {
          z_check=(-block.center[2]+0.25+2.5)/0.5;
          x_check=(block.center[0]+2.5)/0.5;
          z1_check=(-block.center[2]-0.25+2.5)/0.5;
          x1_check=(block.center[0]+2.5)/0.5;

          if(board[x_check][z_check]==0 || board[x1_check][z1_check]==0)
          {
            no_of_moves++;
            game_check=-1;
            printf("Game Over\n");
          }
          else if(board[x_check][z_check]==4 || board[x1_check][z1_check]==4)
          {
            if(switch_check==0)
              switch_check=1;
            else if (switch_check==1)
              switch_check=0;
            no_of_moves++;
            printf("Score:%d\n",no_of_moves);
          }
          else if(board[x_check][z_check]==2 || board[x1_check][z1_check]==2)
          {
            if(board[x_check][z_check]*board[x1_check][z1_check]==4)
              no_of_moves++;
            else if(board[x_check][z_check]*board[x1_check][z1_check]!=4)
            {
              no_of_moves++;
              game_check=-1;
              printf("Game Over\n");
              printf("You Lost!!\n");
              printf("End_Score:%d\n",no_of_moves);
            }
          }
          else
            no_of_moves++;
        }
      }
      createblock(block.center[0],block.center[1],block.center[2],block.length,block.height,block.width,1,1,1);
      // printf("new_rotate:%f\n",block.rotate);                      
    }

  }

  if(game_check==1)
  {
    createBoard();
    block.length=0.5;
    block.height=1;
    block.width=0.5;
    block.center= glm:: vec3(0,0,0);
    block.rotate_check=0;
    block.rotate=0;
    game_check=0;
    switch_check=0;
    createblock(block.center[0],block.center[1],block.center[2],block.length,block.height,block.width,1,1,1);
  }




  int j=0;
  for(i=0;i<10;i++)
  {
    for(j=0;j<10;j++)
    {
      if(board[i][j]==3 && switch_check==1)
      {
        glm::mat4 translateTile = glm::translate (board_pieces[i][j].center); // glTranslatef
        glm::mat4 rotateTile = glm::rotate((float)(board_pieces[i][j].rotate*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 tileTransform = translateTile* rotateTile;
        Matrices.model *= tileTransform; 
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(board_pieces[i][j].coordinates);
        Matrices.model = glm::mat4(1.0f);
      }
      else if(board[i][j]!=0 && board[i][j]!=3)
      {
        glm::mat4 translateTile = glm::translate (board_pieces[i][j].center); // glTranslatef
        glm::mat4 rotateTile = glm::rotate((float)(board_pieces[i][j].rotate*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 tileTransform = translateTile* rotateTile;
        Matrices.model *= tileTransform; 
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(board_pieces[i][j].coordinates);

        // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
        // glPopMatrix ();
        Matrices.model = glm::mat4(1.0f);
      }
    }
  }
  /*
     glm::mat4 translateCam = glm::translate (cam.center); // glTranslatef
     glm::mat4 rotateCam = glm::rotate((float)(cam.rotate_check*cam.rotate*M_PI/180.0f), glm::vec3(0,1,0));  // rotate about vector (1,0,0)
     glm::mat4 camTransform = translateCam* rotateCam;
     Matrices.model *= camTransform; 
     MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(board_pieces[i][j].coordinates);
  Matrices.model = glm::mat4(1.0f);

  if(cam.rotate<90)
  cam.rotate+=5;
  else{
  if(angle==90)
  cam.center=glm ::vec3(4,4,-6);
  else if (angle==180 || angle==-180)
  cam.center=glm ::vec3(-4,4,-6);
  else if(angle==-90)
  cam.center=glm ::vec3(-4,4,6);
  }
  */



  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
  GLFWwindow* window; // window desciptor/handle

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    //        exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

  if (!window) {
    glfwTerminate();
    //        exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval( 1 );

  /* --- register callbacks with GLFW --- */

  /* Register function to handle window resizes */
  /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
  glfwSetFramebufferSizeCallback(window, reshapeWindow);
  glfwSetWindowSizeCallback(window, reshapeWindow);

  /* Register function to handle window close */
  glfwSetWindowCloseCallback(window, quit);

  /* Register function to handle keyboard input */
  glfwSetKeyCallback(window, keyboard);      // general keyboard input
  glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

  /* Register function to handle mouse click */
  glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

  return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  /* Objects should be created before any other gl function and shaders */
  // Create the models
  //	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
  //	createRectangle ();

  createblock(0,0,0,0.5,1,0.5,0,0,0);

  createBoard();
  // createCam();
  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


  reshapeWindow (window, width, height);

  // Background color of the scene
  glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  int width = 920;
  int height = 1200;
  v_eye[0]=4;
  v_eye[1]=4;
  v_eye[2]=6;
  printf("Score:%d\n",no_of_moves);

  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

  /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {

    // OpenGL Draw commands
    draw();
    // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

    // Poll for Keyboard and mouse events
    glfwPollEvents();

    // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    current_time = glfwGetTime(); // Time in seconds
    if ((current_time - last_update_time) >=1) { // atleast 0.5s elapsed since last frame
      if(game_check==-1)
      {
        printf("You Lost!!!\n");
        system("play gameover.wav");
        break;
      }
      if(level==4)
      {
        printf("You Did It!!!\n");
        printf("Total of %d Moves !!!\n",no_of_moves);
        system("play world_clear.wav");
        break;
      }
      // do something every 0.5 seconds ..
      last_update_time = current_time;
    }
  }

  glfwTerminate();
  //    exit(EXIT_SUCCESS);
}
