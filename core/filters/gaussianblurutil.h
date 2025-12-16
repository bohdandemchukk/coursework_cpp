#ifndef GAUSSIANBLURUTIL_H
#define GAUSSIANBLURUTIL_H


#include <QImage>
#include <vector>


class GaussianBlurUtil
{
public:

    static QImage apply (const QImage& input, double sigma);

private:

    static std::vector<double> createKernel(double sigma);

    static QImage applyHorizontal(const QImage& input, const std::vector<double>& kernel, int radius);

    static QImage applyVertical(const QImage& input, const std::vector<double>& kernel, int radius);

};

#endif // GAUSSIANBLURUTIL_H
