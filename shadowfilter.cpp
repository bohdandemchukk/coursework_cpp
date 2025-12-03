#include "shadowfilter.h"


ShadowFilter::ShadowFilter(int shadow)
    : m_shadow{shadow} {}


QImage ShadowFilter::apply(const QImage& input) const {
    if (!isActive()) return input;
    QImage result {input.copy()};
    double amount { getShadow() / 100.0}; // -1.0 до +1.0
    int height {result.height()};
    int width {result.width()};

    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};
        for (int x{0}; x < width; x++) {
            double r = qRed(row[x]) / 255.0;
            double g = qGreen(row[x]) / 255.0;
            double b = qBlue(row[x]) / 255.0;

            // Яскравість пікселя
            double luma = 0.299 * r + 0.587 * g + 0.114 * b;

            // Маска для темних ділянок (плавна крива)
            // Якщо luma < 0.5, то це темна ділянка
            double mask = 0.0;
            if (luma < 0.5) {
                mask = (0.5 - luma) * 2.0; // 0 до 1
                mask = std::pow(mask, 1.5); // Плавна крива
            }

            // Застосовуємо корекцію тільки до темних ділянок
            if (mask > 0.01) {
                // Для тіней краще додавати, а не множити
                double boost = amount * mask * 0.3;

                r += boost * (1.0 - r); // Додаємо пропорційно до простору
                g += boost * (1.0 - g);
                b += boost * (1.0 - b);
            }

            row[x] = qRgb(
                std::clamp(int(r * 255), 0, 255),
                std::clamp(int(g * 255), 0, 255),
                std::clamp(int(b * 255), 0, 255)
                );
        }
    }
    return result;
}

bool ShadowFilter::isActive() const {
    return m_shadow != 0;
}

int ShadowFilter::getShadow() const {
    return m_shadow;
}

void ShadowFilter::setShadow(int shadow) {
    m_shadow = shadow;
}
