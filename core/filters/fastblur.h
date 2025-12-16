#ifndef FASTBLUR_H
#define FASTBLUR_H

#include <QImage>

class FastBlur {
public:
    static QImage apply(const QImage& input, int radius);

private:
    static void boxBlurHorizontal(QImage& img, int radius);
    static void boxBlurVertical(QImage& img, int radius);
};

#endif // FASTBLUR_H
