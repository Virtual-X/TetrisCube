#ifndef BUFFER_H
#define BUFFER_H

#include "tdogl/Texture.h"
#include <memory>


class Buffer
{
public:
    Buffer(GLsizei width, GLsizei height);
    ~Buffer();

    void use();
    void stopUsing();

    std::shared_ptr<tdogl::Texture> texture;

private:
    Buffer(const Buffer&) = delete;
    Buffer& operator = (const Buffer&) = delete;

    GLuint buffer;
};

#endif // BUFFER_H
