#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include "cgimage.h"
#include <GLFW/glfw3.h> 
#include "stb_image.h"
#include <iostream>
#include <algorithm>

CgImage::CgImage()
{
    m_image_width=0;
    m_image_height=0;
    m_image_data=nullptr;
    m_orig_image_data=nullptr;
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

//======================================
//======== Aufgabe 1 ===================
//======================================

void CgImage::setIntensity(int pos_x, int pos_y, int r , int g, int b)
{
    int index = pixelIndex(pos_x, pos_y);
    m_image_data[index] = r;
    m_image_data[index+1] = g;
    m_image_data[index+2] = b;
}

void CgImage::setIntensity(int pos_x, int pos_y, int intensity)
{
    m_image_data[pixelIndex(pos_x, pos_y)] = intensity;
}

int CgImage::getIntensity(int pos_x, int pos_y)
{
    return m_image_data[pixelIndex(pos_x, pos_y)];
}

void CgImage::convertImageToGreyScale()
{

    if (m_channels == 1) return;

    constexpr double rWeight = 0.299;
    constexpr double gWeight = 0.587;
    constexpr double bWeight = 0.114;

    unsigned char* oldDataPointer = m_image_data;
    size_t pixelCount = m_image_width*m_image_height;
    auto greyscale = new unsigned char[pixelCount];
    for (int col = 0; col < m_image_width; ++col)
    {
        for (int row = 0; row < m_image_height; ++row)
        {
            int rgbIndex = pixelIndex(col, row);
            int weightedIntensity = (int)((double)m_image_data[rgbIndex]*rWeight);
            weightedIntensity += (int)((double)m_image_data[rgbIndex+1]*gWeight);
            weightedIntensity += (int)((double)m_image_data[rgbIndex+2]*bWeight);
            greyscale[pixelIndex(col, row, 1)] = weightedIntensity;
        }
    }
    m_image_data = greyscale;
    m_channels = 1;
    stbi_image_free(oldDataPointer);
    createTexture();
}


void CgImage::drawCross(int r, int g, int b, int linewidth)
{
    storeOriginalImage();

    // calculate start and end w/h to center cross with correct width
    int verticalStartColumn  {(m_image_width - linewidth) / 2};
    int verticalEndColumn {verticalStartColumn + linewidth};
    int horizontalStartRow {(m_image_height - linewidth) / 2};
    int horizontalEndRow {horizontalStartRow + linewidth};

    // iterate horizontal line by line
    for (int h_row = horizontalStartRow; h_row < horizontalEndRow; ++h_row)
    {
        for (int h_col = 0; h_col < m_image_width; ++h_col)
        {
            if (m_channels == 3)
                setIntensity(h_col, h_row, r, g, b);
            else
                setIntensity(h_col, h_row, r);
        }
    }

    // iterate vertical column by column
    for (int v_row = 0; v_row < m_image_height; ++v_row)
    {
        for (int v_col = verticalStartColumn; v_col < verticalEndColumn; ++v_col)
        {
            if (m_channels == 3)
                setIntensity(v_col, v_row, r, g, b);
            else
                setIntensity(v_col, v_row, r);
        }
    }

    // create texture from the processed image data for opengl to render
    createTexture();

    resetImage();
}

void CgImage::storeOriginalImage()
{
    // eine Kopie des aktuellen Bildes merken
    int pixelCount = m_image_width*m_image_height;
    size_t imageMemSize = pixelCount * m_channels * sizeof(unsigned char);
    m_orig_image_data = new unsigned char[imageMemSize];
    memcpy(m_orig_image_data, m_image_data, imageMemSize);
    m_orig_channels = m_channels;
}

void CgImage::resetImage()
{
    // aktuelles Bild aus der Kopie wiederherstellen ohne neu zu laden
    int pixelCount = m_image_width*m_image_height;
    size_t imageMemSize = pixelCount * m_channels * sizeof(unsigned char);
    memcpy(m_image_data, m_orig_image_data, imageMemSize);
}

void CgImage::deleteImage()
{
    // Speicher aufräumen, wird z.B. aufgerufen wenn ein neues Bild geladen wird
    stbi_image_free(m_image_data);
    m_image_data = nullptr;
}
void CgImage::deleteOrigImage()
{
    // Speicher aufräumen, wird z.B. aufgerufen bevor ein aktuelles Bild gemerkt werden soll
    free(m_orig_image_data);
    m_orig_image_data = nullptr;
}

int CgImage::pixelIndex(const int x, const int y) const
{
    return y*m_channels*m_image_width + m_channels*x;
}

int CgImage::pixelIndex(const int x, const int y, const int numberOfChannels) const
{
    return y*numberOfChannels*m_image_width + numberOfChannels*x;
}

//======================================
//======== Aufgabe 2 ===================
//======================================

double CgImage::imageVariance()
{
    double meanIntensity {imageMeanIntensity()};
    double pixelCount = {(double)(m_image_width*m_image_height)};
    double accumulator {0};
    for (int i = 0; i < pixelCount; ++i)
    {
        double diffFromMean {((double)m_image_data[i]-meanIntensity)};
        accumulator += diffFromMean*diffFromMean;
    }
    return accumulator/pixelCount;
}

double CgImage::imageMeanIntensity()
{
    convertImageToGreyScale();
    double pixelCount {(double)(m_image_width*m_image_height)};
    double accumulator {0};
    for (int i = 0; i < m_image_width; ++i)
    {
        accumulator += m_image_data[i];
    }
    return accumulator/pixelCount;
}

void CgImage::histogram(float targetArray[256], size_t length)
{
    for (int k = 0; k < 256; ++k)
    {
        targetArray[k] = 0.0;
    }
    int pixelCount {m_image_width*m_image_height};
    for (int i = 0; i < pixelCount; ++i)
    {
        targetArray[m_image_data[i]]++;
    }
    /* debug print
    float total = 0.0f;
    for (int j = 0; j < length; ++j)
    {
        total += targetArray[j];
        std::cout << j << ": " << targetArray[j] << " ";
        std::cout << std::endl;
    }
    std::cout << "Total: " << total << std::endl;
    */
}

void CgImage::changeContrast(double factor, float targetArray[256])
{
    convertImageToGreyScale();
    storeOriginalImage();
    int pixelCount {m_image_width*m_image_height};
    for (int i = 0; i < pixelCount; ++i)
    {
        int newVal {(int)((double)m_image_data[i] * factor)};
        if (newVal > 255) newVal = 255;
        m_image_data[i] = newVal;
    }
    createTexture();
    histogram(targetArray, 256);
    resetImage();
}

void CgImage::changeBrightnes(int value, float targetArray[256])
{
    convertImageToGreyScale();
    storeOriginalImage();
    int pixelCount {m_image_width*m_image_height};
    for (int i = 0; i < pixelCount; ++i)
    {
        int newVal {m_image_data[i]+value};
        if (newVal > 255)
        {
            newVal = 255;
            m_image_data[i] = newVal;
            continue;
        }
        if (newVal < 0) newVal = 0;
        m_image_data[i] = newVal;
    }
    createTexture();
    histogram(targetArray, 256);
    resetImage();
}

void CgImage::changeBitDepth(int newDepth, float targetArray[256])
{
    convertImageToGreyScale();
    storeOriginalImage();

    int pixelCount {m_image_width*m_image_height};
    for (int i = 0; i < pixelCount; ++i)
    {
        unsigned int newMaxVal = (1 << newDepth)-1; // bit shift left by n == mult by 2^n
        unsigned int quantizedPixel = (m_image_data[i] * newMaxVal+127)/255; // ratio of old val to old max val applied to new max val
                                                                             // with offset applied for correct rounding
        unsigned char result = (quantizedPixel*255)/newMaxVal; // ratio of new val to new max val applied to old max val
        m_image_data[i] = result;
    }

    createTexture();
    histogram(targetArray, 256);
    resetImage();
}

void CgImage::robustAutoContrast(float s_low, float s_high, float histogramArray[256])
{
    float accumulatedHistogram[256];
    histogram(histogramArray, 256);
    accumulatedHistogram[0] = histogramArray[0];

    int pixelCount {m_image_width*m_image_height};
    int lowDiscardAmount {(int)(s_low * (float)pixelCount)};
    int highDiscardAmount {(int)((float)pixelCount * (1.0f-s_high))};
    int atick_low {0};
    int atick_high {0};
    int a_min {0};
    int a_max {255};
    for (int l = 1; l < 256; ++l)
    {
        accumulatedHistogram[l] = histogramArray[l] + accumulatedHistogram[l-1];
    }

    for (int i = 0; i < 256; ++i)
    {
        if (accumulatedHistogram[i] >= lowDiscardAmount)
        {
            atick_low = i;
            break;
        }
    }

    for (int j = 255; j >= 0; --j)
    {
        if (accumulatedHistogram[j] <= highDiscardAmount)
        {
            atick_high = j;
            break;
        }
    }
    int atick_diff {atick_high-atick_low};
    int a_range = a_max-a_min;
    storeOriginalImage();
    for (int k = 0; k < pixelCount; ++k)
    {
        int pixelIntensity = m_image_data[k];
        if (pixelIntensity <= atick_low)
        {
            m_image_data[k] = a_min;
            continue;
        }
        if (pixelIntensity > atick_low && pixelIntensity < atick_high)
        {
            m_image_data[k] = a_min + (pixelIntensity - atick_low)*(a_range/atick_diff);
            continue;
        }
        if (pixelIntensity >= atick_high)
        {
            m_image_data[k] = a_max;
        }
    }
    createTexture();
    histogram(histogramArray, 256);
    resetImage();
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
