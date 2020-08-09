// g++ main.cc -lGL -lEGL `pkg-config opencv --cflags` `pkg-config opencv --libs` 
#include <opencv2/opencv.hpp>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
// #include <opencv2/core.hpp>



static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE};

static const GLint width = 320;
static const GLint height = 240;


/// EGL display
EGLDisplay eglDisplay;

/// EGL context
EGLContext eglContext;

/// EGL surface
EGLSurface eglSurface;

GLuint programObject;


static const EGLint pbufferAttribs[] = {
    EGL_WIDTH,
    width,
    EGL_HEIGHT,
    height,
    EGL_NONE,
};

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;

   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   {
      return 0;
   }

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );

   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled )
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char *)malloc ( sizeof ( char ) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
        ( "Error compiling shader:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

///
// Initialize the shader and program object
//
int Init ()
{
   char vShaderStr[] =
      "#version 300 es                          \n"
      "layout(location = 0) in vec4 vPosition;  \n"
      "void main()                              \n"
      "{                                        \n"
      "   gl_Position = vPosition;              \n"
      "}                                        \n";

   char fShaderStr[] =
      "#version 300 es                              \n"
      "precision mediump float;                     \n"
      "out vec4 fragColor;                          \n"
      "void main()                                  \n"
      "{                                            \n"
      "   fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );  \n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char *) malloc ( sizeof ( char ) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return false;
   }

   glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
   return true;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( )
{

   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                            -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f
                         };

   // Set the viewport
   glViewport ( 0, 0, width, height );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( programObject );

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

void Shutdown ( )
{

   glDeleteProgram ( programObject );
}

int main(int argc, char *argv[])
{


    // 1. Initialize EGL
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint major, minor;

    eglInitialize(eglDisplay, &major, &minor);

    std::cout << "major: " << major << " " << "minor: " << minor << std::endl;


    // 2. Select an appropriate configuration
    EGLint numConfigs;
    EGLConfig eglCfg;

    eglChooseConfig(eglDisplay, configAttribs, &eglCfg, 1, &numConfigs);

    // 3. Create a surface
    eglSurface = eglCreatePbufferSurface(eglDisplay, eglCfg,
                                                 pbufferAttribs);

    // 4. Bind the API
    // eglBindAPI(EGL_OPENGL_API);

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    // 5. Create a context and make it current
    eglContext = eglCreateContext(eglDisplay, eglCfg, EGL_NO_CONTEXT, contextAttribs);

    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    // from now on use your OpenGL context

    Init();

    Draw();

	/*
	 * Render something.
	 */
	// glClearColor(0.9, 0.8, 0.5, 1.0);
	// glClear(GL_COLOR_BUFFER_BIT);    

    glFlush();


    GLint size;
    size = width * height * 4;
    GLubyte *data = (GLubyte *)malloc(size);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);


cv::Mat img;
std::vector<cv::Mat> imgPlanes;
img.create(height, width, CV_8UC3); //确定图片通道和尺寸
cv::split(img, imgPlanes);                //将图像按照通道数拆分，三个单通道序列

for (int i = 0; i < height; i++)
{
    unsigned char *plane0Ptr = imgPlanes[0].ptr<unsigned char>(i); //B
    unsigned char *plane1Ptr = imgPlanes[1].ptr<unsigned char>(i); //G
    unsigned char *plane2Ptr = imgPlanes[2].ptr<unsigned char>(i); //R
    //opencv里面以BGR存储的，而Mac上opengl是RGBA，所以需要改变顺序保存
    for (int j = 0; j < width; j++)
    {
        int k = 4 * (i * width + j);   //RGBA数据结构，不需要A，跳过，所以步长乘以4
        plane2Ptr[j] = data[k];     //R
        plane1Ptr[j] = data[k + 1]; //G
        plane0Ptr[j] = data[k + 2]; //B
    }
}


cv::merge(imgPlanes, img);                  //合并多通道图像
cv::flip(img, img, 0);                      // 反转图像，因为opengl和opencv的坐标系y轴是相反的
// cv::cvtColor(img, img, cv::COLOR_RGB2GRAY); //转换为灰度图
// cv::namedWindow("openglGrab");
// cv::imshow("openglGrab", img);
cv::waitKey();

cv::imwrite("tmp.jpg", img); //保存图片

    Shutdown();


    // 6. Terminate EGL when finished
    eglTerminate(eglDisplay);
    return 0;
}