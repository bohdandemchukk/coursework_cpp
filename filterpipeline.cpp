#include "filterpipeline.h"
#include <QDebug>
FilterPipeline::FilterPipeline() {}

FilterPipeline::FilterPipeline(const FilterPipeline& other)
{
    for (const auto& f : other.filters)
        filters.push_back(f->clone());
}

FilterPipeline& FilterPipeline::operator=(const FilterPipeline& other)
{
    if (this == &other)
        return *this;

    filters.clear();
    for (const auto& f : other.filters)
        filters.push_back(f->clone());

    qDebug() << "Pipeline filters count:" << filters.size();

    return *this;
}


void FilterPipeline::addFilter(std::unique_ptr<ImageFilter> filter) {
    if (filter) {
        filters.push_back(std::move(filter));
    }
}

void FilterPipeline::removeFilter(size_t index) {
    if (index < filters.size()) {
        filters.erase(filters.begin() + index);
    }
}

void FilterPipeline::clear() {
    filters.clear();
}

QImage FilterPipeline::process(const QImage& src) const
{
    Q_ASSERT(src.format() == QImage::Format_ARGB32_Premultiplied);

    QImage img = src.copy();

    for (auto& filter : filters) {
        img = filter->apply(img);
        Q_ASSERT(img.format() == QImage::Format_ARGB32_Premultiplied);
    }


    return img;
}



