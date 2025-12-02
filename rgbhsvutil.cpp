#include "rgbhsvutil.h"




void RgbHsvUtil::rgb2hsv(double r, double g, double b, double &h, double &s, double &v) {

    double maxv = std::max({r, g, b});
    double minv = std::min({r, g, b});
    double d = maxv - minv;

    v = maxv;

    if (maxv == 0) {
        s = 0;
    } else {
        s = d / maxv;
    }

    if (d == 0) {
        h = 0;
    } else if (maxv == r) {
        h = 60 * fmod(((g - b) / d), 6.0);
    } else if (maxv == g) {
        h = 60 * (((b - r) / d) + 2.0);
    } else {
        h = 60 * (((r - g) / d) + 4.0);
    }

    if (h < 0) h += 360;

}

QRgb RgbHsvUtil::hsv2rgb(double h, double s, double v) {

    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2.0) - 1));
    double m = v - c;

    double r, g, b;

    if      (h < 60)  { r=c; g=x; b=0; }
    else if (h < 120) { r=x; g=c; b=0; }
    else if (h < 180) { r=0; g=c; b=x; }
    else if (h < 240) { r=0; g=x; b=c; }
    else if (h < 300) { r=x; g=0; b=c; }
    else              { r=c; g=0; b=x; }

    return qRgb(
        int(std::clamp((r+m) * 255, 0.0, 255.0)),
        int(std::clamp((g+m) * 255, 0.0, 255.0)),
        int(std::clamp((b+m) * 255, 0.0, 255.0))
        );

}
