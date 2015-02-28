#include "loadertdogl.h"

#include <QImage>
#include <QGLWidget>

#include <iostream>
#include <fstream>
#include <sstream>


std::shared_ptr<tdogl::Program> LoaderTDOGL::LoadShaders(const std::string& vertFilename, const std::string& fragFilename)
{
    return std::make_shared<tdogl::Program>(std::vector<tdogl::Shader>{
        tdogl::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER),
        tdogl::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER)});
}


std::shared_ptr<tdogl::Texture> LoaderTDOGL::LoadTexture(const std::string& filename)
{
    std::shared_ptr<tdogl::Bitmap> bmp(LoadBitmap(ResourcePath(filename)));
    if (!bmp) {
        return std::shared_ptr<tdogl::Texture>();
    }
    bmp->flipVertically();
    return std::make_shared<tdogl::Texture>(*bmp);
}

std::shared_ptr<tdogl::Texture> LoaderTDOGL::LoadTexture(const std::vector<std::string> &filenames)
{
    std::vector<std::shared_ptr<tdogl::Bitmap>> bitmaps;
    std::vector<const tdogl::Bitmap*> bmps;
    for (const auto& filename : filenames) {
        auto bmp = LoadBitmap(ResourcePath(filename));
        if (!bmp) {
            return std::shared_ptr<tdogl::Texture>();
        }
        bmp->flipVertically();
        bitmaps.push_back(bmp);
        bmps.push_back(bmp.get());
    }
    return std::make_shared<tdogl::Texture>(bmps);
}

std::shared_ptr<tdogl::Bitmap> LoaderTDOGL::LoadBitmap(const std::string& filename)
{
    if (filename.substr(filename.size() - 4) == ".txt") {
        return LoadBitmapTXT(filename);
    }
    QImage img;
    if (!img.load(filename.c_str())) {
        std::cerr << "Error loading " << filename << std::endl ;
        return std::shared_ptr<tdogl::Bitmap>();
    }

    QImage image;
    image = QGLWidget::convertToGLFormat(img);
    if (image.isNull()) {
        std::cerr << "Error converting " << filename << std::endl ;
        return std::shared_ptr<tdogl::Bitmap>();
    }

    return std::make_shared<tdogl::Bitmap>(image.width(), image.height(), tdogl::Bitmap::Format_RGBA, image.bits());
}

std::shared_ptr<tdogl::Bitmap> LoaderTDOGL::LoadBitmapTXT(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error loading " << filename << std::endl ;
        return std::shared_ptr<tdogl::Bitmap>();
    }
    std::string line;
    std::vector<unsigned char> data;
    int x = 0;
    int y = 0;
    while (std::getline(in, line)) {
        double v = 0;
        std::istringstream linein(line);
        int xx = 0;
        while (linein >> v) {
            v = (10 - v) * 20;
            v = v < 0 ? 0 : v > 255 ? 255 : v;
            data.push_back((unsigned char)v);
            data.push_back((unsigned char)v);
            data.push_back((unsigned char)v);
            data.push_back((unsigned char)v);
            xx++;
//            if (xx == 16) break;
        }
        if (xx == 0) {
            std::cerr << "Empty line in " << filename << std::endl ;
            return std::shared_ptr<tdogl::Bitmap>();
        }
        if (y == 0) {
            x = xx;
        }
        else if (xx != x) {
            std::cerr << "Invalid line in " << filename << std::endl ;
            return std::shared_ptr<tdogl::Bitmap>();
        }
        y++;
//        if (y == 16) break;
    }
    if (y == 0) {
        std::cerr << "Empty file in " << filename << std::endl ;
        return std::shared_ptr<tdogl::Bitmap>();
    }
    return std::make_shared<tdogl::Bitmap>(x, y, tdogl::Bitmap::Format_RGBA, &data[0]);
}

std::string LoaderTDOGL::GetProcessPath()
{
    return "/home/igor/Development/qt-workspace/App/";
}


std::string LoaderTDOGL::ResourcePath(const std::string& fileName)
{
    return GetProcessPath() + fileName;
}
