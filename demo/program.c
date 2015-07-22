#include "Program.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ERR_MSG(X) printf("In file: %s line: %d\n\t%s\n", __FILE__, __LINE__, X);

/*
 ** reads contents of a file and returns a zero terminated string. or NULL if
 ** the file could not be opened.
 */
static char* readFile(const char* filename)
{
	FILE* f = NULL;
	size_t numChars = 1; /* # of chars in the file */
	char* contents = NULL;
	char ch = 0;
	size_t count  = 0;
	
	f = fopen(filename, "r");
	
	if (!f)
	{
		return NULL;
	}
	
	/* count # of chars in the file */
	while (EOF != fgetc(f))
	{
		numChars++;
	}
	
	contents = (char*)malloc(numChars);
	
	if (!contents)
	{
		fclose(f);
		return NULL;
	}
	
	/* rewind and load the file's contents */
	rewind(f);
	
	while(1)
	{
		ch = fgetc(f);
		
		/* break if end of file is reached, don't forget to finish the string */
		if (ch == EOF)
		{
			contents[count] = '\0';
			break;
		}
		
		contents[count] = ch;
		count++;
	}
	
	/* clean up and return the contents */
	fclose(f);
	
	return contents;
}

/*
 ** Attach shader from file
 */
int glueProgramAttachShaderWithFile(GLuint program, GLenum type,
	const char* file)
{
	char* contents = NULL; /* contents of the file */
	int succ = 0;
	
	contents = readFile(file);
	
	if (!contents)
	{
		ERR_MSG("Could not load shader file")
		return 0;
	}
	
	succ = glueProgramAttachShaderWithSource(program, type, contents);
	
	
	assert(NULL != contents);
	
	/* clean up */
	free(contents);
	
	return succ;
}

/*
 ** Attach shader from source
 */
int glueProgramAttachShaderWithSource(GLuint program, GLenum type,
	const char* source)
{
	GLuint shader = 0;
	GLint status;
	GLint infoLogLength; /* length of the info log */
	GLchar* infoLog; 	 /* the info log */
	
	shader = glCreateShader(type);
	
	if (!shader)
	{
		ERR_MSG("Failed to create shader");
		return 0;
	}
	
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	/* check if the shader compiled correctly */
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	
	/* complain if something went wrong */
	if (status == GL_FALSE)
	{
	  	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		
		infoLog = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
		
		ERR_MSG(infoLog);
		free(infoLog);
		
		glDeleteShader(shader);
		
		return 0;
	}
	
	glAttachShader(program, shader);
	
	return 1;
}

int glueProgramLink(GLuint program)
{
	GLint status = 0;
	GLint infoLogLength; /* length of the info log */
	GLchar* infoLog; 	 /* the info log */
	GLint numShaders; 	 /* # of shader attached to the program */
	GLuint* shaders;     /* shaders attached to the program */
	int i = 0;
	
	/* link the program */
	glLinkProgram(program);
	
	/* check if everything went well */
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	
	/* complain if s.th went wrong */
	if (status == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		infoLog = (char*)malloc(infoLogLength + 1);
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
		ERR_MSG(infoLog)
		free(infoLog);
		return 0;
	}
	
	/* detach and delete the shaders from the program */
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &numShaders);
	shaders = (GLuint*)malloc(sizeof(GLuint)*numShaders);
	glGetAttachedShaders(program, numShaders, NULL, shaders);
	
	for (i = 0; i < numShaders; i++)
	{
		glDetachShader(program, shaders[i]);
		glDeleteShader(shaders[i]);
	}
	
	return 1;
}


int glueProgramUniform1i(GLuint program, const char* name, GLint value)
{
	GLint loc = glGetUniformLocation(program, name);
	
	glUseProgram(program);
	
	if (loc < 0)
	{
		ERR_MSG("program var not found")
		return 0;
	}
	
	glUniform1i(loc, value);
	
	return 1;
}

int glueProgramUniform1f(GLuint program, const char* name, GLfloat value)
{
	GLint loc = glGetUniformLocation(program, name);
	
	glUseProgram(program);
	
	if (loc < 0)
	{
		ERR_MSG("program var not found")
		return 0;
	}
	
	glUniform1f(loc, value);
	
	return 1;
}

int glueProgramUniform2f(GLuint program, const char* name, GLfloat v0,
	GLfloat v1)
{
	GLint loc = glGetUniformLocation(program, name);
	
	glUseProgram(program);
	
	if (loc < 0)
	{
		ERR_MSG("program var not found")
		return 0;
	}
	
	glUniform2f(loc, v0, v1);
	
	return 1;
}

int glueProgramUniform3f(GLuint program, const char* name, GLfloat v0,
	GLfloat v1, GLfloat v2)
{
	GLint loc = glGetUniformLocation(program, name);
	
	glUseProgram(program);
	
	if (loc < 0)
	{
		ERR_MSG("program var not found")
		return 0;
	}
	
	glUniform3f(loc, v0, v1, v2);
	
	return 1;
}

int glueProgramUniformMatrix4(GLuint program, const char* name,
			      const GLfloat* value, GLboolean transpose)
{
	GLint loc = glGetUniformLocation(program, name);
	
	glUseProgram(program);
	
	if (loc < 0)
	{
		ERR_MSG("program var not found")
		return 0;
	}
	
	glUniformMatrix4fv(loc, 1, transpose, value);
	
	return 1;
}
