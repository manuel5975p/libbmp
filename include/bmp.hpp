#include <cstdio>
#include <iostream>
#include <vector>
using std::fprintf;
template <typename T>
class has_cols
{
    typedef char one;
    struct two { char x[2]; };

    template <typename C> static one test( decltype(&C::cols) ) ;
    template <typename C> static two test(...);    

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};


struct dimension_obtainer{
    size_t width;
    size_t height;
    
    template<typename T>
    dimension_obtainer(const T& t){
        if constexpr(has_cols<T>::value){
            width = t.cols();
            height = t.rows();
        }
        else{
            width = t.width();
            height = t.height();
        }
    }
    
};
template<typename image>
void write_bmp(std::ostream& ostr, const image& img){
    const dimension_obtainer dims(img);

    unsigned int headers[13];
    int extrabytes;
    int paddedsize;
    int x; int y; int n;
    int red, green, blue;
    extrabytes = 4 - ((dims.width * 3) % 4);
    if (extrabytes == 4)
    extrabytes = 0;
    paddedsize = ((dims.width * 3) + extrabytes) * dims.height;

    headers[0]  = paddedsize + 54;      // bfSize (whole file size)
    headers[1]  = 0;                    // bfReserved (both)
    headers[2]  = 54;                   // bfOffbits
    headers[3]  = 40;                   // biSize
    headers[4]  = dims.width;  // biWidth
    headers[5]  = dims.height; // biHeight

    headers[7]  = 0;                    // biCompression
    headers[8]  = paddedsize;           // biSizeImage
    headers[9]  = 0;                    // biXPelsPerMeter
    headers[10] = 0;                    // biYPelsPerMeter
    headers[11] = 0;                    // biClrUsed
    headers[12] = 0; 
    ostr << "BM";
    for (n = 0; n <= 5; n++){
        ostr << char( headers[n] & 0x000000FF);
        ostr << char((headers[n] & 0x0000FF00) >> 8);
        ostr << char((headers[n] & 0x00FF0000) >> 16);
        ostr << char((headers[n] & (unsigned int) 0xFF000000) >> 24);
    }
    ostr << char(1);
    ostr << char(0);
    ostr << char(24);
    ostr << char(0);
    for (n = 7; n <= 12; n++){
        ostr << char(headers[n] & 0x000000FF);
        ostr << char((headers[n] & 0x0000FF00) >> 8);
        ostr << char((headers[n] & 0x00FF0000) >> 16);
        ostr << char((headers[n] & (unsigned int) 0xFF000000) >> 24);
    }
    for (y = 0; y < dims.height; y++){
        for (x = 0; x < dims.width; x++){
            if constexpr(std::is_floating_point_v<decltype(img(x, y)[0])>){
                red =   (int)(img(x, y)[0] * 256);
                green = (int)(img(x, y)[1] * 256);
                blue =  (int)(img(x, y)[2] * 256);
            }
            else{
                red =   img(x, y)[0];
                green = img(x, y)[1];
                blue =  img(x, y)[2];
            }
            if (red > 255) red = 255; if (red < 0) red = 0;
            if (green > 255) green = 255; if (green < 0) green = 0;
            if (blue > 255) blue = 255; if (blue < 0) blue = 0;
            ostr << char(blue);
            ostr << char(green);
            ostr << char(red);
        }
        if (extrabytes){
            for (n = 1; n <= extrabytes; n++){
                ostr << char(0);
            }
        }
    }
}