#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "export_image.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

static std::vector<unsigned char> colorToPixels(int width, int height,
                                                 const std::vector<Color>& imgData,
                                                 bool flipVertically)
{
    std::vector<unsigned char> pixels(width * height * 3);
    for (int y = 0; y < height; ++y) {
        int srcY = flipVertically ? (height - 1 - y) : y;
        for (int x = 0; x < width; ++x) {
            size_t dst = (y * width + x) * 3;
            size_t src = srcY * width + x;
            pixels[dst + 0] = static_cast<unsigned char>(std::clamp(imgData[src].r, 0.f, 1.f) * 255.f + 0.5f);
            pixels[dst + 1] = static_cast<unsigned char>(std::clamp(imgData[src].g, 0.f, 1.f) * 255.f + 0.5f);
            pixels[dst + 2] = static_cast<unsigned char>(std::clamp(imgData[src].b, 0.f, 1.f) * 255.f + 0.5f);
        }
    }
    return pixels;
}

bool exportImagePng(const std::string& filename,
                    int width, int height,
                    const std::vector<Color>& imgData,
                    bool flipVertically)
{
    if (imgData.size() != static_cast<size_t>(width * height))
        return false;

    auto pixels = colorToPixels(width, height, imgData, flipVertically);
    int stride = width * 3;
    return stbi_write_png(filename.c_str(), width, height, 3, pixels.data(), stride) != 0;
}

static void pngWriteToBuffer(void* context, void* data, int size)
{
    auto* buf = static_cast<std::vector<unsigned char>*>(context);
    auto* bytes = static_cast<unsigned char*>(data);
    buf->insert(buf->end(), bytes, bytes + size);
}

bool copyImageToClipboard(int width, int height,
                          const std::vector<Color>& imgData,
                          bool flipVertically)
{
    if (imgData.size() != static_cast<size_t>(width * height))
        return false;

    auto pixels = colorToPixels(width, height, imgData, flipVertically);

    // Encode PNG to memory buffer
    std::vector<unsigned char> pngBuf;
    int stride = width * 3;
    int ok = stbi_write_png_to_func(pngWriteToBuffer, &pngBuf,
                                     width, height, 3, pixels.data(), stride);
    if (!ok || pngBuf.empty())
        return false;

    // Pipe PNG data to xclip
    FILE* proc = popen("xclip -selection clipboard -t image/png", "w");
    if (!proc)
        return false;

    size_t written = fwrite(pngBuf.data(), 1, pngBuf.size(), proc);
    int ret = pclose(proc);
    return written == pngBuf.size() && ret == 0;
}
