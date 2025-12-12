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

    template<typename T, typename... Args>
    void setOrReplace(Args&&... args)
    {
        static_assert(std::is_base_of_v<ImageFilter, T>,
                      "T must derive from ImageFilter");

        for (auto& f : filters)
        {
            if (dynamic_cast<T*>(f.get()))
            {
                f = std::make_unique<T>(std::forward<Args>(args)...);
                return;
            }
        }

        filters.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template<typename T>
    void remove()
    {
        static_assert(std::is_base_of_v<ImageFilter, T>,
                      "T must derive from ImageFilter");

        auto it = std::remove_if(
            filters.begin(),
            filters.end(),
            [](const std::unique_ptr<ImageFilter>& f)
            {
                return dynamic_cast<T*>(f.get()) != nullptr;
            }
            );

        filters.erase(it, filters.end());
    }

    template<typename T>
    T* find() const
    {
        for (const auto& f : filters)
            if (auto* casted = dynamic_cast<T*>(f.get()))
                return casted;
        return nullptr;
    }





private:
    std::vector<std::unique_ptr<ImageFilter>> filters{};
};

#endif // FILTERPIPELINE_H
