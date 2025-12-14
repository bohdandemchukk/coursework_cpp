#ifndef FILTERSPANEL_H
#define FILTERSPANEL_H

#include <QWidget>
#include <memory>
#include <QPushButton>

class QVBoxLayout;
class FilterSlider;
class QCheckBox;
class CollapsibleSection;

class Layer;
class FilterPipeline;

class FiltersPanel : public QWidget
{
    Q_OBJECT
public:
    explicit FiltersPanel(QWidget* parent = nullptr);

    void setActiveLayer(std::shared_ptr<Layer> layer, int index);
    void syncFromActiveLayer();

signals:
    void pipelineChanged(
        int layerIndex,
        FilterPipeline before,
        FilterPipeline after
        );


    void previewRequested();


private:
    void buildUI();
    void connectSlider(
        FilterSlider* slider,
        std::function<void(FilterPipeline&, int)> apply
        );

    std::shared_ptr<Layer> m_activeLayer;
    int m_activeLayerIndex = -1;

    bool m_updating = false;

    CollapsibleSection* addSection(QVBoxLayout* root, const QString& title, const std::function<void(QVBoxLayout*)>& contentBuilder);

    FilterSlider* m_shadow = nullptr;
    FilterSlider* m_temperature = nullptr;
    FilterSlider* m_tint = nullptr;
    FilterSlider* m_saturation = nullptr;
    FilterSlider* m_vibrance = nullptr;
    FilterSlider* m_fade = nullptr;
    FilterSlider* m_clarity = nullptr;
    FilterSlider* m_grain = nullptr;
    FilterSlider* m_splithighlight = nullptr;
    FilterSlider* m_splitshadow = nullptr;
    FilterSlider* m_vignette = nullptr;
    QPushButton* m_rotateRight = nullptr;
    QPushButton* m_rotateLeft = nullptr;
    QPushButton* m_flipH = nullptr;
    QPushButton* m_flipV = nullptr;
    FilterSlider* m_exposure = nullptr;
    FilterSlider* m_contrast = nullptr;
    FilterSlider* m_brightness = nullptr;
    FilterSlider* m_blur = nullptr;
    FilterSlider* m_fastBlur = nullptr;
    FilterSlider* m_sharpen = nullptr;
    FilterSlider* m_gamma = nullptr;

    QCheckBox* m_bw = nullptr;
};

#endif // FILTERSPANEL_H
