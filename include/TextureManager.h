#pragma once
#include "Bitmap.h"

class TextureManager
{
public:
    TextureManager()
    {
        // A white texture is added by default
        m_bitmaps.push_back(Bitmap());
        m_bitmaps[0].width = 1;
        m_bitmaps[0].height = 1;
        m_bitmaps[0].pixels.push_back(RGBColor(0xFF, 0xFF, 0xFF));
    }

    // Add a new texture loaded from a bitmap file
    size_t AddTexture(const std::string& filename)
    {
        m_bitmaps.push_back(Bitmap());
        size_t id = m_bitmaps.size()-1;
        m_bitmaps[id].LoadFile(filename);
        return id;
    }

    void CleanUp()
    {
        m_bitmaps.clear();
    }

    Bitmap& GetTexture(size_t textureId) { return m_bitmaps[textureId]; }

private:
    std::vector<Bitmap> m_bitmaps;
};
