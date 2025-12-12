#include "filterpipeline.h"

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

QImage FilterPipeline::process(const QImage &input) const {

    QImage result {input};

    for (const auto& filter: filters) {
        if (filter->isActive()) {
            result = filter->apply(result);
        }
    }

    return result;
}



