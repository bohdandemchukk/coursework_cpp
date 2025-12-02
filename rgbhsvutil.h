#ifndef RGBHSVUTIL_H
#define RGBHSVUTIL_H

#include <QRgb>

class RgbHsvUtil
{
public:

    static void rgb2hsv(double r, double g, double b, double &h, double &s, double &v);

    static QRgb hsv2rgb(double h, double s, double v);

};

#endif // RGBHSVUTIL_H
