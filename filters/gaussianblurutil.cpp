#include "gaussianblurutil.h"

std::vector<double> GaussianBlurUtil::createKernel(double sigma) {
    int radius {std::ceil(3 * sigma)};
    int size {2 * radius + 1};
    std::vector<double> kernel(size);
    double sum {0.0};

    for (int i {-radius}; i <= radius; i++) {
        double value {std::exp( -(i*i) / (2 * sigma * sigma) )};
        kernel[i + radius] = value;
        sum += value;
    }

    for (double& x : kernel) {
        x /= sum;
    }

    return kernel;
}



QImage GaussianBlurUtil::applyHorizontal(const QImage& input, const std::vector<double>& kernel, int radius) {
    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};


    for (int y {0}; y < height; y++) {
        const QRgb* src {reinterpret_cast<const QRgb*>(input.scanLine(y))};
        QRgb* dst {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x {0}; x < width; x++) {
            double r {0.0};
            double g {0.0};
            double b {0.0};


            for (int i {-radius}; i <= radius; i++) {
                int px = std::clamp(x + i, 0, width - 1);

                r += qRed(src[px]) * kernel[i + radius];
                g += qGreen(src[px]) * kernel[i + radius];
                b += qBlue(src[px]) * kernel[i + radius];
            }

            dst[x] = qRgb(
                std::clamp(static_cast<int>(r), 0, 255),
                std::clamp(static_cast<int>(g), 0, 255),
                std::clamp(static_cast<int>(b), 0, 255)
                );
        }
    }
    return result;
}


QImage GaussianBlurUtil::applyVertical(const QImage& input, const std::vector<double>& kernel, int radius) {

    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};


    for (int x {0}; x < width; x++) {

        for (int y {0}; y < height; y++) {
            double r{0.0};
            double g{0.0};
            double b{0.0};

            for (int j {-radius}; j <= radius; j++) {
                int py {std::clamp(y + j, 0, height - 1)};
                QRgb px {reinterpret_cast<const QRgb*>(input.scanLine(py))[x]};

                r += qRed(px) * kernel[j + radius];
                g += qGreen(px) * kernel[j + radius];
                b += qBlue(px) * kernel[j + radius];
            }

            reinterpret_cast<QRgb*>(result.scanLine(y))[x] = qRgb(
                std::clamp(static_cast<int>(r), 0, 255),
                std::clamp(static_cast<int>(g), 0, 255),
                std::clamp(static_cast<int>(b), 0, 255)
                );
        }
    }

    return result;
}

QImage GaussianBlurUtil::apply(const QImage& input, double sigma) {
    if (sigma < 0.001) return input;

    auto kernel {createKernel(sigma)};
    int radius {(kernel.size() - 1) / 2};

    QImage temp {applyHorizontal(input, kernel, radius)};

    return applyVertical(temp, kernel, radius);
}
