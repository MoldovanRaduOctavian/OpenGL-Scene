#ifndef Shader_hpp
#define Shader_hpp

#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

namespace gps {

class Shader
{
public:
    GLuint shaderProgram;
	GLuint id;
    void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
    void useShaderProgram();


    std::string readShaderFile(std::string fileName);
    void shaderCompileLog(GLuint shaderId);
    void shaderLinkLog(GLuint shaderProgramId);

	void setInt(const char* name, int val);
	void setFloat(const char* name, float val);
	void setDouble(const char* name, double val);
	void setVec2f(const char* name, glm::vec2 val);
	void setVec3f(const char* name, glm::vec3 val);
	void setVec4f(const char* name, glm::vec4 val);
	void setMat2f(const char* name, glm::mat2 val);
	void setMat3f(const char* name, glm::mat3 val);
	void setMat4f(const char* name, glm::mat4 val);
};

}

#endif /* Shader_hpp */
