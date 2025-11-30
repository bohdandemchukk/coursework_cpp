#ifndef FILTERPIPELINE_H
#define FILTERPIPELINE_H

#include <QImage>
#include <vector>
#include <memory>
#include <utility>
#include "imagefilter.h"


class FilterPipeline
{
public:
    FilterPipeline();
    void addFilter(std::unique_ptr<ImageFilter> filter);
    void removeFilter(size_t index);
    void clear();
    QImage process(const QImage& input) const;


private:
    std::vector<std::unique_ptr<ImageFilter>> filters{};
};

#endif // FILTERPIPELINE_H
