#include "filterspanel.h"

#include <QVBoxLayout>
#include <QCheckBox>

#include "filterslider.h"
#include "collapsiblesection.h"

#include "layer.h"
#include "filterpipeline.h"

#include "rotatefilter.h"
#include "flipfilter.h"
#include "exposurefilter.h"
#include "contrastfilter.h"
#include "brightnessfilter.h"
#include "blurfilter.h"
#include "sharpenfilter.h"
#include "gammafilter.h"
#include "highlightfilter.h"
#include "shadowfilter.h"
#include "saturationfilter.h"
#include "vibrancefilter.h"
#include "fadefilter.h"
#include "temperaturefilter.h"
#include "tintfilter.h"
#include "clarityFilter.h"
#include "grainfilter.h"
#include "splittoningfilter.h"
#include "vignettefilter.h"
#include "bwfilter.h"

FiltersPanel::FiltersPanel(QWidget* parent)
    : QWidget(parent)
{
    buildUI();
}

void FiltersPanel::buildUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);


    {
        auto* section = new CollapsibleSection("Basic", this);
        auto* l = new QVBoxLayout();

        m_exposure   = new FilterSlider("Exposure",   -100, 100, 0);
        m_contrast   = new FilterSlider("Contrast",   -100, 100, 0);
        m_brightness = new FilterSlider("Brightness", -100, 100, 0);

        l->addWidget(m_exposure);
        l->addWidget(m_contrast);
        l->addWidget(m_brightness);


        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_exposure,  [](auto& p,int v){ p.template setOrReplace<ExposureFilter>(v); });
        connectSlider(m_contrast,  [](auto& p,int v){ p.template setOrReplace<ContrastFilter>(v); });
        connectSlider(m_brightness,[](auto& p,int v){ p.template setOrReplace<BrightnessFilter>(v); });
    }

    {
        auto* section = new CollapsibleSection("Color", this);
        auto* l = new QVBoxLayout();

        m_temperature = new FilterSlider("Temperature", -100, 100, 0);
        m_tint        = new FilterSlider("Tint",        -100, 100, 0);
        m_saturation  = new FilterSlider("Saturation", -100, 100, 0);
        m_vibrance    = new FilterSlider("Vibrance",   -100, 100, 0);
        m_fade        = new FilterSlider("Fade",        0,   100, 0);
        m_gamma       = new FilterSlider("Gamma",       0, 300, 0);

        l->addWidget(m_temperature);
        l->addWidget(m_tint);
        l->addWidget(m_saturation);
        l->addWidget(m_vibrance);
        l->addWidget(m_fade);
        l->addWidget(m_gamma);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_temperature,[](auto& p,int v){ p.template setOrReplace<TemperatureFilter>(v); });
        connectSlider(m_tint,       [](auto& p,int v){ p.template setOrReplace<TintFilter>(v); });
        connectSlider(m_saturation, [](auto& p,int v){ p.template setOrReplace<SaturationFilter>(v); });
        connectSlider(m_vibrance,   [](auto& p,int v){ p.template setOrReplace<VibranceFilter>(v); });
        connectSlider(m_fade,       [](auto& p,int v){ p.template setOrReplace<FadeFilter>(v); });
        connectSlider(m_gamma,       [](auto& p,int v){ p.template setOrReplace<GammaFilter>(v); });
    }

    {
        auto* section = new CollapsibleSection("Details", this);
        auto* l = new QVBoxLayout();

        m_clarity = new FilterSlider("Clarity",  -100, 100, 0);
        m_sharpen = new FilterSlider("Sharpen",   0,   100, 0);
        m_blur    = new FilterSlider("Blur",      0,   100, 0);
        m_grain   = new FilterSlider("Grain",     0,   100, 0);

        l->addWidget(m_clarity);
        l->addWidget(m_sharpen);
        l->addWidget(m_blur);
        l->addWidget(m_grain);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_clarity,[](auto& p,int v){ p.template setOrReplace<ClarityFilter>(v); });
        connectSlider(m_sharpen,[](auto& p,int v){ p.template setOrReplace<SharpenFilter>(v); });
        connectSlider(m_blur,   [](auto& p,int v){ p.template setOrReplace<BlurFilter>(v); });
        connectSlider(m_grain,  [](auto& p,int v){ p.template setOrReplace<GrainFilter>(v); });
    }


    {
        auto* section = new CollapsibleSection("Split Toning", this);
        auto* l = new QVBoxLayout();

        m_splithighlight = new FilterSlider("Highlights", -100, 100, 0);
        m_splitshadow    = new FilterSlider("Shadows",    -100, 100, 0);

        l->addWidget(m_splithighlight);
        l->addWidget(m_splitshadow);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_splithighlight,[](auto& p,int v){ p.template setOrReplace<HighlightFilter>(v); });
        connectSlider(m_splitshadow,   [](auto& p,int v){ p.template setOrReplace<ShadowFilter>(v); });
    }


    {
        auto* section = new CollapsibleSection("Effects", this);
        auto* l = new QVBoxLayout();

        m_vignette = new FilterSlider("Vignette", -100, 100, 0);
        m_bw = new QCheckBox("Black & White");

        l->addWidget(m_vignette);
        l->addWidget(m_bw);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_vignette,[](auto& p,int v){ p.template setOrReplace<VignetteFilter>(v); });

        connect(m_bw, &QCheckBox::toggled, this, [this](bool on){
            if (m_updating || !m_activeLayer) return;
            auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
            if (!adj) return;

            auto before = adj->pipeline();
            auto after  = before;

            if (on) after.setOrReplace<BWFilter>(true);
            else    after.remove<BWFilter>();

            emit pipelineChanged(m_activeLayerIndex,
                                 std::move(before),
                                 std::move(after));
        });
    }



    root->addStretch();
}


void FiltersPanel::connectSlider(
    FilterSlider* slider,
    std::function<void(FilterPipeline&, int)> apply)
    {
    connect(slider, &FilterSlider::valueChanged,
            this, [this, slider, apply](int value)
            {
                if (m_updating || !m_activeLayer) return;
                if (m_activeLayer->type() != LayerType::Adjustment) return;

                auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
                if (!adj) return;

                auto temp = adj->pipeline();
                apply(temp, value);

                adj->pipeline() = std::move(temp);
                emit previewRequested();
            });

    connect(slider, &FilterSlider::sliderReleased,
            this, [this, slider, apply](int value)
            {
                if (m_updating || !m_activeLayer) return;
                if (m_activeLayer->type() != LayerType::Adjustment) return;

                auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
                if (!adj) return;

                auto before = adj->pipeline();
                auto after  = before;
                apply(after, value);

                emit pipelineChanged(
                    m_activeLayerIndex,
                    std::move(before),
                    std::move(after)
                    );
            });
}

void FiltersPanel::setActiveLayer(std::shared_ptr<Layer> layer, int index)
{
    m_activeLayer = std::move(layer);
    m_activeLayerIndex = index;

    syncFromActiveLayer();

    bool isAdj = m_activeLayer && m_activeLayer->type() == LayerType::Adjustment;
    setEnabled(isAdj);
}
void FiltersPanel::syncFromActiveLayer()
{
    m_updating = true;

    auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
    if (!adj)
    {
        m_exposure->setValue(0);
        m_contrast->setValue(0);
        m_brightness->setValue(0);
        m_blur->setValue(0);
        m_sharpen->setValue(0);
        m_gamma->setValue(0);
        m_bw->setChecked(false);
        m_updating = false;
        return;
    }

    auto& p = adj->pipeline();

    m_exposure->setValue(p.find<ExposureFilter>() ? p.find<ExposureFilter>()->getExposure() : 0);
    m_contrast->setValue(p.find<ContrastFilter>() ? p.find<ContrastFilter>()->getContrast() : 0);
    m_brightness->setValue(p.find<BrightnessFilter>() ? p.find<BrightnessFilter>()->getBrightness() : 0);
    m_blur->setValue(p.find<BlurFilter>() ? p.find<BlurFilter>()->getBlur() : 0);
    m_sharpen->setValue(p.find<SharpenFilter>() ? p.find<SharpenFilter>()->getSharpness() : 0);
    m_gamma->setValue(p.find<GammaFilter>() ? p.find<GammaFilter>()->getGamma() : 0);

    m_bw->setChecked(p.find<BWFilter>() != nullptr);

    m_updating = false;
}

CollapsibleSection* FiltersPanel::addSection(
    QVBoxLayout* root,
    const QString& title,
    const std::function<void(QVBoxLayout*)>& contentBuilder)
{
    auto* section = new CollapsibleSection(title, this);
    auto* layout  = new QVBoxLayout();

    contentBuilder(layout);

    section->setContentLayout(layout);
    root->addWidget(section);
    return section;
}

