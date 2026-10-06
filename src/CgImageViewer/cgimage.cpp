#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include "cgimage.h"
#include <GLFW/glfw3.h> 
#include "stb_image.h"
#include <iostream>


CgImage::CgImage()
{
    m_image_width=0;
    m_image_height=0;
    m_image_data=nullptr;
    m_orig_channels=0;
}

CgImage::~CgImage()
{
    delete [] m_image_data;
}

int CgImage::getImageWidth()
{
   return m_image_width;
}

int CgImage::getImageHeight()
{
   return m_image_height;
}

void CgImage::setIntensity(int pos_x, int pos_y, int r , int g, int b)
{
    int pixelIndex = gridIndex(pos_x, pos_y);
    m_image_data[pixelIndex] = r;
    m_image_data[pixelIndex+1] = g;
    m_image_data[pixelIndex+2] = b;
}

void CgImage::setIntensity(int pos_x, int pos_y, int intensity)
{
    m_image_data[gridIndex(pos_x, pos_y)] = intensity;
}

int CgImage::getIntensity(int pos_x, int pos_y)
{
    return m_image_data[gridIndex(pos_x, pos_y)];
}

void CgImage::convertImageToGreyScale()
{
    // to be implemented
}


void CgImage::drawCross(int r, int g, int b, int linewidth)
{
    // calculate start and end w/h to center cross with correct width

    int verticalStartColumn  {(m_image_width - linewidth) / 2};
    int verticalEndColumn {verticalStartColumn + linewidth};

    int horizontalStartRow {(m_image_height - linewidth) / 2};
    int horizontalEndRow {horizontalStartRow + linewidth};

    // iterate horizontal line by line -> small loop outside, 0 to m_image_width loop inside

    for (int h_row = horizontalStartRow; h_row <= horizontalEndRow; ++h_row)
    {
        for (int h_col = 0; h_col < m_image_width; ++h_col)
        {
            m_image_data[gridIndex(h_col, h_row)] = r;
            m_image_data[gridIndex(h_col, h_row) + 1] = g;
            m_image_data[gridIndex(h_col, h_row) + 2] = b;
        }
    }

    // iterate vertical column by column -> 0 to m_image_height loop outside, small loop inside

    for (int v_col = 0; v_col < m_image_height; ++v_col)
    {
        for (int v_row = verticalStartColumn; v_row <= verticalEndColumn; ++v_row)
        {
            m_image_data[gridIndex(v_col, v_row)] = r;
            m_image_data[gridIndex(v_col, v_row) + 1] = g;
            m_image_data[gridIndex(v_col, v_row) + 2] = b;
        }
    }

    // create texture from the processed image data for opengl to render

    createTexture();

}

void CgImage::storeOriginalImage()
{
    // eine Kopie des aktuellen Bildes merken
    // to be implemented
}

void CgImage::resetImage()
{
    // aktuelles Bild aus der Kopie wiederherstellen ohne neu zu laden
    // to be implemented
}

void CgImage::deleteImage()
{
    // Speicher aufräumen, wird z.B. aufgerufen wenn ein neues Bild geladen wird
    // to be implemented
}
void CgImage::deleteOrigImage()
{
    // Speicher aufräumen, wird z.B. aufgerufen bevor ein aktuelles Bild gemerkt werden soll
    // to be implemented
}

int CgImage::gridIndex(const int x, const int y) const
{
    return y*m_channels*m_image_width + m_channels*x;
}


// Simple helper function to load an image into unsigned char* with common settings
bool CgImage::LoadFromFile(const char* filename)
{
    deleteImage();
    deleteOrigImage();
    
    int orig_num_channels;
    
    // set desired number of channels
    m_channels=3;
    
    // Load from file
    unsigned char* image_data = stbi_load(filename, &m_image_width, &m_image_height, &orig_num_channels, 4);
    if (image_data == NULL)
        return false;
    
    m_image_data = new unsigned char[m_image_width*m_image_height*m_channels];
    
    
    for(unsigned int i=0;i<m_image_width*m_image_height;i++)
    {
        m_image_data[m_channels*i]=image_data[4*i];
        m_image_data[m_channels*i+1]=image_data[4*i+1];
        m_image_data[m_channels*i+2]=image_data[4*i+2];
    }
    
    stbi_image_free(image_data);
    
    storeOriginalImage();
    createTexture();
    
    return true;
}


/********************************************************************
*
*
*   OpenGl stuff, alles was hier unten steht muss nicht angefasst werden
*
*
****************************************************************************************************/

GLuint& CgImage::getImageTexture()
{
   return m_image_texture;
}



// OpenGL helper function to convert unsigned char* into OpenGL Texture
// not necessary to unterstand
void CgImage::createTexture()
{
    
    
    
    // Create a OpenGL texture identifier

    glGenTextures(1, &m_image_texture);
    glBindTexture(GL_TEXTURE_2D, m_image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same


    
    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    switch (m_channels) {
        case 1:
        {
            unsigned char* rgb_image_data = new unsigned char[m_image_width*m_image_height*3];
            for(unsigned int i=0;i<m_image_width*m_image_height;i++)
            {
                rgb_image_data[3*i]=  m_image_data[i];
                rgb_image_data[3*i+1]=m_image_data[i];
                rgb_image_data[3*i+2]=m_image_data[i];
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_image_width, m_image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_image_data);
            delete [] rgb_image_data;
            break;
        }
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_image_width, m_image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_image_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image_width, m_image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image_data);
            break;
        default:
            break;
    }
    
}
