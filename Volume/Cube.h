#ifndef CUBE_H
#define CUBE_H

#include "tdogl/Program.h"
using tdogl::Program;

#include <memory>


class Cube
{
public:
    Cube();
    ~Cube();

    void Bind(
        const Program& program,
        const GLchar* positionName,
        const GLchar* textureName,
        const GLchar* normalName);

    void Draw();

private:
    Cube(const Cube&) = delete;
    Cube& operator = (const Cube&) = delete;

    std::shared_ptr<Program> program;
    GLuint vbo;
    GLuint vao;
};

#endif // CUBE_H
