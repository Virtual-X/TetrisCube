/*
 tdogl::Texture
 
 Copyright 2012 Thomas Dalling - http://tomdalling.com/
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "Texture.h"
#include <stdexcept>

using namespace tdogl;

static GLenum TextureFormatForBitmapFormat(Bitmap::Format format, bool srgb)
{
    switch (format) {
        case Bitmap::Format_Grayscale: return GL_LUMINANCE;
        case Bitmap::Format_GrayscaleAlpha: return GL_LUMINANCE_ALPHA;
        case Bitmap::Format_RGB: return (srgb ? GL_SRGB : GL_RGB);
        case Bitmap::Format_RGBA: return (srgb ? GL_SRGB_ALPHA : GL_RGBA);
        default: throw std::runtime_error("Unrecognised Bitmap::Format");
    }
}

Texture::Texture(GLuint object, GLfloat width, GLfloat height, GLfloat depth)
    : _object(object),
      _target(depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D),
    _width(width),
    _height(height),
    _depth(depth)
{
}

Texture::Texture(const Bitmap& bitmap, GLint minMagFiler, GLint wrapMode) :
    _target(GL_TEXTURE_2D),
    _width((GLfloat)bitmap.width()),
    _height((GLfloat)bitmap.height()),
    _depth(1)
{
    glGenTextures(1, &_object);
    glBindTexture(GL_TEXTURE_2D, _object);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMagFiler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, minMagFiler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexImage2D(GL_TEXTURE_2D,
                 0, 
                 TextureFormatForBitmapFormat(bitmap.format(), true),
                 (GLsizei)bitmap.width(), 
                 (GLsizei)bitmap.height(),
                 0, 
                 TextureFormatForBitmapFormat(bitmap.format(), false),
                 GL_UNSIGNED_BYTE, 
                 bitmap.pixelBuffer());
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(const std::vector<const Bitmap*>& bitmaps, GLint minMagFiler, GLint wrapMode) :
    _target(GL_TEXTURE_3D),
    _width((GLfloat)(*bitmaps[0]).width()),
    _height((GLfloat)(*bitmaps[0]).height()),
    _depth((GLfloat)bitmaps.size())
{
    glGenTextures(1, &_object);
    glBindTexture(GL_TEXTURE_3D, _object);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minMagFiler);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, minMagFiler);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode);
    const auto& bitmap = *bitmaps[0];
    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 TextureFormatForBitmapFormat(bitmap.format(), true),
                 (GLsizei)bitmap.width(),
                 (GLsizei)bitmap.height(),
                 bitmaps.size(),
                 0,
                 TextureFormatForBitmapFormat(bitmap.format(), false),
                 GL_UNSIGNED_BYTE,
                 0);
    int z = 0;
    for (const auto& b : bitmaps) {
        glTexSubImage3D(GL_TEXTURE_3D,
                     0,
                        0, 0, z++,
                        (GLsizei)b->width(),
                        (GLsizei)b->height(),
                        1,
                     TextureFormatForBitmapFormat(b->format(), false),
                     GL_UNSIGNED_BYTE,
                b->pixelBuffer());
    }
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture::~Texture()
{
    glDeleteTextures(1, &_object);
}

void Texture::use(GLenum textureUnit)
{
    _textureUnit = textureUnit;
    glActiveTexture(textureUnit);
    glEnable(_target);
    glBindTexture(_target, _object);
}

void Texture::stopUsing()
{
    glActiveTexture(_textureUnit);
    glBindTexture(_target, 0);
}


GLuint Texture::object() const
{
    return _object;
}

GLfloat Texture::width() const
{
    return _width;
}

GLfloat Texture::height() const
{
    return _height;
}

GLfloat Texture::depth() const
{
    return _depth;
}
