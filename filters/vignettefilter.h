#ifndef VIGNETTEFILTER_H
#define VIGNETTEFILTER_H

#include "imagefilter.h"
#include <QImage>

class VignetteFilter : public ImageFilter
{
public:
    explicit VignetteFilter(int vignette);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getVignette() const;

    void setVignette(int vignette);

private:
    int m_vignette{};
};

#endif // VIGNETTEFILTER_H
