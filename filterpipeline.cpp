#include "filterpipeline.h"

FilterPipeline::FilterPipeline() {}

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



