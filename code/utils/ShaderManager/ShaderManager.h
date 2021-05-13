//
// Created by marwac-9 on 9/17/15.
//

class ShaderManager
{
	typedef unsigned int GLuint;
public:
    static ShaderManager* Instance();
    GLuint GetCurrentShaderID();
    void SetCurrentShader(GLuint id);
private:
    ShaderManager();
    ~ShaderManager();
    //copy
    ShaderManager(const ShaderManager&);
    //assign
    ShaderManager& operator=(const ShaderManager&);

    GLuint current_shader;

};