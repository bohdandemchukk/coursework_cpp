#include "fastblur.h"
#include <algorithm>

static inline int clamp(int v, int lo, int hi) {
    return std::min(std::max(v, lo), hi);
}

QImage FastBlur::apply(const QImage& input, int radius)
{
    Q_ASSERT(input.format() == QImage::Format_ARGB32_Premultiplied);
    if (radius <= 0)
        return input;

    QImage img = input.copy();

    for (int i = 0; i < 3; ++i) {
        boxBlurHorizontal(img, radius);
        boxBlurVertical(img, radius);
    }

    return img;
}


void FastBlur::boxBlurHorizontal(QImage& img, int r)
{
    int w = img.width();
    int h = img.height();
    int size = r * 2 + 1;

    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));

        int rsum = 0, gsum = 0, bsum = 0, asum = 0;

        for (int i = -r; i <= r; ++i) {
            int x = clamp(i, 0, w - 1);
            QRgb p = line[x];
            rsum += qRed(p);
            gsum += qGreen(p);
            bsum += qBlue(p);
            asum += qAlpha(p);
        }

        for (int x = 0; x < w; ++x) {
            line[x] = qRgba(
                rsum / size,
                gsum / size,
                bsum / size,
                asum / size
                );

            int xAdd = clamp(x + r + 1, 0, w - 1);
            int xSub = clamp(x - r,     0, w - 1);

            QRgb pAdd = line[xAdd];
            QRgb pSub = line[xSub];

            rsum += qRed(pAdd)   - qRed(pSub);
            gsum += qGreen(pAdd) - qGreen(pSub);
            bsum += qBlue(pAdd)  - qBlue(pSub);
            asum += qAlpha(pAdd) - qAlpha(pSub);
        }
    }
}

void FastBlur::boxBlurVertical(QImage& img, int r)
{
    int w = img.width();
    int h = img.height();
    int size = r * 2 + 1;

    for (int x = 0; x < w; ++x) {
        int rsum = 0, gsum = 0, bsum = 0, asum = 0;

        for (int i = -r; i <= r; ++i) {
            int y = clamp(i, 0, h - 1);
            QRgb p = reinterpret_cast<QRgb*>(img.scanLine(y))[x];
            rsum += qRed(p);
            gsum += qGreen(p);
            bsum += qBlue(p);
            asum += qAlpha(p);
        }

        for (int y = 0; y < h; ++y) {
            reinterpret_cast<QRgb*>(img.scanLine(y))[x] = qRgba(
                rsum / size,
                gsum / size,
                bsum / size,
                asum / size
                );

            int yAdd = clamp(y + r + 1, 0, h - 1);
            int ySub = clamp(y - r,     0, h - 1);

            QRgb pAdd = reinterpret_cast<QRgb*>(img.scanLine(yAdd))[x];
            QRgb pSub = reinterpret_cast<QRgb*>(img.scanLine(ySub))[x];

            rsum += qRed(pAdd)   - qRed(pSub);
            gsum += qGreen(pAdd) - qGreen(pSub);
            bsum += qBlue(pAdd)  - qBlue(pSub);
            asum += qAlpha(pAdd) - qAlpha(pSub);
        }
    }
}
