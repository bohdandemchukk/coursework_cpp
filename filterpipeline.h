#ifndef FILTERPIPELINE_H
#define FILTERPIPELINE_H

#include <QImage>
#include <vector>
#include <memory>
#include <utility>
#include "imagefilter.h"
#include <algorithm>
#include <type_traits>


class FilterPipeline
{
public:
    FilterPipeline();
    FilterPipeline(const FilterPipeline& other);
    FilterPipeline& operator=(const FilterPipeline& other);

    FilterPipeline(FilterPipeline&&) = default;
    FilterPipeline& operator=(FilterPipeline&&) = default;

    void addFilter(std::unique_ptr<ImageFilter> filter);
    void removeFilter(size_t index);
    void clear();
    QImage process(const QImage& input) const;

    template<class T>
    T* find()
    {
        for (auto& f : filters)
            if (auto p = dynamic_cast<T*>(f.get()))
                return p;
        return nullptr;
    }

    template<class T>
    void remove()
    {
        filters.erase(
            std::remove_if(filters.begin(), filters.end(),
                           [](const std::unique_ptr<ImageFilter>& f)
                           {
                               return dynamic_cast<T*>(f.get()) != nullptr;
                           }),
            filters.end()
            );
    }

    template<class T, class... Args>
    void setOrReplace(Args&&... args)
    {
        if (auto* f = find<T>())
        {
            *f = T(std::forward<Args>(args)...);
        }
        else
        {
            filters.push_back(
                std::make_unique<T>(std::forward<Args>(args)...)
                );
        }
    }




private:
    std::vector<std::unique_ptr<ImageFilter>> filters{};
};

#endif // FILTERPIPELINE_H
