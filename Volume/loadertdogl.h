#ifndef LOADERTDOGL_H
#define LOADERTDOGL_H

#include "tdogl/Program.h"
#include "tdogl/Texture.h"

#include <memory>

class LoaderTDOGL
{
public:
    static std::shared_ptr<tdogl::Program> LoadShaders(const std::string& vertFilename, const std::string& fragFilename);
    static std::shared_ptr<tdogl::Texture> LoadTexture(const std::string& filename);
    static std::shared_ptr<tdogl::Texture> LoadTexture(const std::vector<std::string>& filename);

private:
    static std::string GetProcessPath();
    static std::string ResourcePath(const std::string& fileName);

    static std::shared_ptr<tdogl::Bitmap> LoadBitmap(const std::string& filename);
    static std::shared_ptr<tdogl::Bitmap> LoadBitmapTXT(const std::string& filename);
};


#endif // LOADERTDOGL_H
