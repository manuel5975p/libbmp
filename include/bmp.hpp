#include <type_traits>
#include <concepts>
#include <algorithm>
#include <ostream>
#include <iterator>

template <typename T>
class has_cols
{
    template <typename C> static std::true_type test( decltype(&C::cols) ) ;
    template <typename C> static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
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

template<std::output_iterator<char> iterator, typename image>
void write_bmp(iterator it, const image& img){
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
    *it++ = 'B';
    *it++ = 'M';
    for (n = 0; n <= 5; n++){
        *it++ = char( headers[n] & 0x000000FF);
        *it++ = char((headers[n] & 0x0000FF00) >> 8);
        *it++ = char((headers[n] & 0x00FF0000) >> 16);
        *it++ = char((headers[n] & (unsigned int) 0xFF000000) >> 24);
    }
    *it++ = char(1);
    *it++ = char(0);
    *it++ = char(24);
    *it++ = char(0);
    for (n = 7; n <= 12; n++){
        *it++ = char(headers[n] & 0x000000FF);
        *it++ = char((headers[n] & 0x0000FF00) >> 8);
        *it++ = char((headers[n] & 0x00FF0000) >> 16);
        *it++ = char((headers[n] & (unsigned int) 0xFF000000) >> 24);
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
            red = std::clamp(red, 0, 255);
            green = std::clamp(green, 0, 255);
            blue = std::clamp(blue, 0, 255);
            *it++ = char(blue);
            *it++ = char(green);
            *it++ = char(red);
        }
        if (extrabytes){
            for (n = 1; n <= extrabytes; n++){
                *it++ = char(0);
            }
        }
    }
}
template<typename image>
void write_bmp(std::ostream& ostr, const image& img){
    write_bmp(std::ostreambuf_iterator(ostr), img);
}
