#ifndef PROGRAM_H
#define PROGRAM_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#endif

/*
** Attaches a shader to the program.
**
** @param program Program the shader is attached to.
** @param type Type of the shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc.) 
** @param source Source of the shader as a zero terminated c array.
** 
** @return 0 in case of failure 1 otherwise.
*/ 
int glueProgramAttachShaderWithSource(
	GLuint program, 
	GLenum type, 
	const char* source
);


/*
** Attaches a shader to the program.
**
** @param program Program the shader is attached to.
** @param type Type of the shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc.) 
** @param file Filename of the shader to be attached to the program. 
** 
** @return 0 in case of failure 1 otherwise.
*/ 
int glueProgramAttachShaderWithFile(
	GLuint program, 
	GLenum type, 
	const char* file
);

/*
** Links the cgk_openglgl program.
**
** @return 0 in case of failure 1 otherwise.
*/ 
int glueProgramLink(GLuint program);

/*
** sets an integer scalar for the program
*/ 
int glueProgramUniform1i(
	GLuint program, 
	const char* name, 
	GLint value
);

/*
** sets a float scalar for the program
*/ 
int glueProgramUniform1f(
	GLuint program, 
	const char* name, 
	GLfloat value
);

/*
** sets a 2d vector for the program
*/ 
int glueProgramUniform2f(
	GLuint program, 
	const char* name, 
	GLfloat v0, 
	GLfloat v1 
);

/*
** sets a 3d vector for the program
*/ 
int glueProgramUniform3f(
	GLuint program, 
	const char* name, 
	GLfloat v0, 
	GLfloat v1, 
	GLfloat v2
);

/*
** sets a matrix for the program
*/ 
int glueProgramUniformMatrix4(
    GLuint program,
    const char* name, 
    const GLfloat* value, 
    GLboolean transpose	
);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: PROGRAM_H */
