#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void drawPixel(unsigned char* data, int width, int height, int channels, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    int index = (y * width + x) * channels;
    data[index]     = r;
    data[index + 1] = g;
    data[index + 2] = b;
    if (channels == 4) {
        data[index + 3] = 255;
    }
}

void lineBruteForce(unsigned char* data, int width, int height, int channels, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    if (x1 == x2) {
        int startY = min(y1, y2);
        int endY = max(y1, y2);
        for (int y = startY; y <= endY; ++y) {
            if (steep) {
                drawPixel(data, width, height, channels, y, x1, r, g, b);
            } else {
                drawPixel(data, width, height, channels, x1, y, r, g, b);
            }
        }
        return;
    }
    
    double m = static_cast<double>(y2 - y1) / (x2 - x1);

    for (int x = x1; x <= x2; ++x) {
        double y = m * (x - x1) + y1;
        int rounded_y = static_cast<int>(round(y));
        if (steep) {
            drawPixel(data, width, height, channels, rounded_y, x, r, g, b);
        } else {
            drawPixel(data, width, height, channels, x, rounded_y, r, g, b);
        }
    }
}

int main() {
    // input "INTP.jpg"
    const char* inputFilename = "INTP.jpg";
    const char* outputFilename = "output_line.png";

    int width, height, channels;

    unsigned char *img = stbi_load(inputFilename, &width, &height, &channels, 0);

    if (img == nullptr) {
        cerr << "Error: Could not load image '" << inputFilename << "'." << endl;
        return 1;
    }

    // **** INI ADALAH BARIS YANG DITAMBAHKAN ****
    cout << "Gambar berhasil dimuat! Ukuran: " << width << " x " << height << " piksel." << endl;

    int x1, y1, x2, y2;

    cout << "Program Menggambar Garis pada Gambar" << endl;
    cout << "Koordinat (0,0) ada di pojok kiri atas." << endl;

    cout << "Masukkan koordinat titik pertama (x1 y1): ";
    cin >> x1 >> y1;

    cout << "Masukkan koordinat titik kedua (x2 y2): ";
    cin >> x2 >> y2;

    unsigned char r = 255, g = 0, b = 0; // Warna merah
    cout << "Menggambar garis merah..." << endl;

    lineBruteForce(img, width, height, channels, x1, y1, x2, y2, r, g, b);

    if (stbi_write_png(outputFilename, width, height, channels, img, width * channels) == 0) {
        cerr << "Error: Could not save image to '" << outputFilename << "'." << endl;
    } else {
        cout << "Garis berhasil digambar! Gambar disimpan sebagai '" << outputFilename << "'." << endl;
    }

    stbi_image_free(img);

    return 0;
}
