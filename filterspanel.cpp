#include "filterspanel.h"

#include <QVBoxLayout>
#include <QCheckBox>

#include "filterslider.h"
#include "collapsiblesection.h"
#include <QStyle>
#include "layer.h"
#include "filterpipeline.h"

#include <QToolButton>
#include "fastblurfilter.h"
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
#include <QPainter>


FiltersPanel::FiltersPanel(QWidget* parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    buildUI();
}

void FiltersPanel::buildUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);



    {
        auto* section = new CollapsibleSection("Basic", this);
        section->setContentsMargins(0, 0, 0, 0);

        auto* l = new QVBoxLayout();

        m_exposure   = new FilterSlider("Exposure", QIcon(":/icons/filters/exposure.svg"),   -100, 100, 0);
        m_contrast   = new FilterSlider("Contrast", QIcon(":/icons/filters/contrast.svg"),  -100, 100, 0);
        m_brightness = new FilterSlider("Brightness", QIcon(":/icons/filters/brightness.svg"), -100, 100, 0);

        l->addWidget(m_brightness);
        l->addWidget(m_exposure);
        l->addWidget(m_contrast);



        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_exposure,  [](auto& p,int v){ p.template setOrReplace<ExposureFilter>(v); });
        connectSlider(m_contrast,  [](auto& p,int v){ p.template setOrReplace<ContrastFilter>(v); });
        connectSlider(m_brightness,[](auto& p,int v){ p.template setOrReplace<BrightnessFilter>(v); });
    }

    {
        auto* section = new CollapsibleSection("Color", this);
        section->setContentsMargins(0, 0, 0, 0);

        auto* l = new QVBoxLayout();

        m_temperature = new FilterSlider("Temperature", QIcon(":/icons/filters/temperature.svg"), -100, 100, 0);
        m_tint        = new FilterSlider("Tint", QIcon(":/icons/filters/tint.svg"),       -100, 100, 0);
        m_saturation  = new FilterSlider("Saturation", QIcon(":/icons/filters/saturation.svg"), -100, 100, 0);
        m_vibrance    = new FilterSlider("Vibrance", QIcon(":/icons/filters/vibrance.svg"),   -100, 100, 0);
        m_fade        = new FilterSlider("Fade", QIcon(":/icons/filters/fade.svg"),        0,   100, 0);
        m_gamma       = new FilterSlider("Gamma", QIcon(":/icons/filters/gamma.svg"),       0, 300, 0);

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
        section->setContentsMargins(0, 0, 0, 0);

        auto* l = new QVBoxLayout();

        m_clarity = new FilterSlider("Clarity", QIcon(":/icons/filters/clarity.svg"),  -100, 100, 0);
        m_sharpen = new FilterSlider("Sharpen", QIcon(":/icons/filters/sharpness.svg"),   0,   100, 0);
        m_blur    = new FilterSlider("Blur", QIcon(":/icons/filters/blur.svg"),      0,   100, 0);
        m_fastBlur= new FilterSlider("Fast Blur", QIcon(":/icons/filters/fastblur.svg"), 0,   100, 0);
        m_grain   = new FilterSlider("Grain", QIcon(":/icons/filters/grain.svg"),     0,   100, 0);

        l->addWidget(m_clarity);
        l->addWidget(m_sharpen);
        l->addWidget(m_blur);
        l->addWidget(m_fastBlur);
        l->addWidget(m_grain);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_clarity,[](auto& p,int v){ p.template setOrReplace<ClarityFilter>(v); });
        connectSlider(m_sharpen,[](auto& p,int v){ p.template setOrReplace<SharpenFilter>(v); });
        connectSlider(m_blur,   [](auto& p,int v){ p.template setOrReplace<BlurFilter>(v); });
        connectSlider(m_fastBlur,   [](auto& p,int v){ p.template setOrReplace<FastBlurFilter>(v); });
        connectSlider(m_grain,  [](auto& p,int v){ p.template setOrReplace<GrainFilter>(v); });
    }


    {
        auto* section = new CollapsibleSection("Split Toning", this);
        section->setContentsMargins(0, 0, 0, 0);

        auto* l = new QVBoxLayout();

        m_splithighlight = new FilterSlider("Highlights", QIcon(":/icons/filters/highlight.svg"), -100, 100, 0);
        m_splitshadow    = new FilterSlider("Shadows", QIcon(":/icons/filters/shadow.svg"),    -100, 100, 0);

        l->addWidget(m_splithighlight);
        l->addWidget(m_splitshadow);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_splithighlight,[](auto& p,int v){ p.template setOrReplace<HighlightFilter>(v); });
        connectSlider(m_splitshadow,   [](auto& p,int v){ p.template setOrReplace<ShadowFilter>(v); });
    }


    {
        auto* section = new CollapsibleSection("Effects", this);
        section->setContentsMargins(0, 0, 0, 0);

        auto* l = new QVBoxLayout();

        m_vignette = new FilterSlider("Vignette", QIcon(":/icons/filters/vignette.svg"), -100, 100, 0);
        //m_bw = new QToolButton(this);
        //->setCheckable(true);
        //m_bw->setChecked(false);
        //m_bw->setIcon(QIcon(":/icons/filters/bw.svg"));
        //m_bw->setIconSize(QSize(16, 16));
        //m_bw->setObjectName("bwButton");
        //m_bw->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);




        l->addWidget(m_vignette);

        // UNDER CONSTRUCTION // l->addWidget(m_bw);

        section->setContentLayout(l);
        root->addWidget(section);

        connectSlider(m_vignette,[](auto& p,int v){ p.template setOrReplace<VignetteFilter>(v); });

        /** connect(m_bw, &QToolButton::toggled, this, [this](bool on){
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

            m_pipelineBeforeSlider = adj->pipeline();
        });
        **/
    }



    root->addStretch();
}


void FiltersPanel::connectSlider(
    FilterSlider* slider,
    std::function<void(FilterPipeline&, int)> apply)
{
    auto beforeState = std::make_shared<std::optional<FilterPipeline>>();

    connect(slider, &FilterSlider::sliderPressed,
            this, [this, beforeState]()
            {
                if (m_updating || !m_activeLayer) return;
                if (m_activeLayer->type() != LayerType::Adjustment) return;

                auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
                if (!adj) return;


                *beforeState = adj->pipeline();
            });

    connect(slider, &FilterSlider::valueChanged,
            this, [](int value)
            {

                Q_UNUSED(value);
            });

    connect(slider, &FilterSlider::sliderReleased,
            this, [this, apply, beforeState](int value)
            {
                if (m_updating || !m_activeLayer) return;
                if (m_activeLayer->type() != LayerType::Adjustment) return;

                auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
                if (!adj) return;

                if (beforeState->has_value()) {

                    auto before = **beforeState;
                    auto after = before;
                    apply(after, value);

                    emit pipelineChanged(
                        m_activeLayerIndex,
                        std::move(before),
                        std::move(after)
                        );

                    beforeState->reset();
                } else {

                    auto before = adj->pipeline();
                    auto after = before;
                    apply(after, value);

                    emit pipelineChanged(
                        m_activeLayerIndex,
                        std::move(before),
                        std::move(after)
                        );
                }
            });
}
void FiltersPanel::setActiveLayer(std::shared_ptr<Layer> layer, int index)
{
    m_activeLayer = std::move(layer);
    m_activeLayerIndex = index;

    const bool isAdj =
        m_activeLayer && m_activeLayer->type() == LayerType::Adjustment;

    for (auto* s : findChildren<FilterSlider*>()) {
        s->setEnabled(isAdj);
        s->setProperty("inactive", !isAdj);
        s->style()->unpolish(s);
        s->style()->polish(s);
    }

    for (auto* sec : findChildren<CollapsibleSection*>()) {
        sec->setProperty("inactive", !isAdj);
        sec->style()->unpolish(sec);
        sec->style()->polish(sec);
    }


    syncFromActiveLayer();
}

void FiltersPanel::syncFromActiveLayer()
{
    m_updating = true;

    qDebug() << "=== syncFromActiveLayer ===";
    qDebug() << "m_activeLayer:" << m_activeLayer.get();
    if (m_activeLayer) {
        qDebug() << "Layer type:" << (int)m_activeLayer->type();
        qDebug() << "Layer name:" << m_activeLayer->name();
    }

    auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(m_activeLayer);
    if (!adj)
    {
        qDebug() << "Not adjustment layer - resetting to 0";
        m_exposure->setValue(0);
        m_contrast->setValue(0);
        m_brightness->setValue(0);
        m_blur->setValue(0);
        m_fastBlur->setValue(0);
        m_sharpen->setValue(0);
        m_gamma->setValue(0);
        // m_bw->setChecked(false);
        m_temperature->setValue(0);
        m_tint->setValue(0);
        m_saturation->setValue(0);
        m_vibrance->setValue(0);
        m_fade->setValue(0);
        m_clarity->setValue(0);
        m_grain->setValue(0);
        m_splithighlight->setValue(0);
        m_splitshadow->setValue(0);
        m_vignette->setValue(0);

        m_updating = false;
        return;
    }

    qDebug() << "Is adjustment layer - reading pipeline";
    auto& p = adj->pipeline();

    int brightness = p.find<BrightnessFilter>() ? p.find<BrightnessFilter>()->getBrightness() : 0;
    int exposure = p.find<ExposureFilter>() ? p.find<ExposureFilter>()->getExposure() : 0;
    int contrast = p.find<ContrastFilter>() ? p.find<ContrastFilter>()->getContrast() : 0;

    qDebug() << "Pipeline values - brightness:" << brightness << "exposure:" << exposure << "contrast:" << contrast;

    m_exposure->setValue(exposure);
    m_contrast->setValue(contrast);
    m_brightness->setValue(brightness);
    m_blur->setValue(p.find<BlurFilter>() ? p.find<BlurFilter>()->getBlur() : 0);
    m_fastBlur->setValue(p.find<FastBlurFilter>() ? p.find<FastBlurFilter>()->getBlur() : 0);
    m_sharpen->setValue(p.find<SharpenFilter>() ? p.find<SharpenFilter>()->getSharpness() : 0);
    m_gamma->setValue(p.find<GammaFilter>() ? p.find<GammaFilter>()->getGamma() : 0);

    m_temperature->setValue(p.find<TemperatureFilter>() ? p.find<TemperatureFilter>()->getTemperature() : 0);
    m_tint->setValue(p.find<TintFilter>() ? p.find<TintFilter>()->getTint() : 0);
    m_saturation->setValue(p.find<SaturationFilter>() ? p.find<SaturationFilter>()->getSaturation() : 0);
    m_vibrance->setValue(p.find<VibranceFilter>() ? p.find<VibranceFilter>()->getVibrance() : 0);

    m_fade->setValue(p.find<FadeFilter>() ? p.find<FadeFilter>()->getFade() : 0);
    m_clarity->setValue(p.find<ClarityFilter>() ? p.find<ClarityFilter>()->getClarity() : 0);
    m_grain->setValue(p.find<GrainFilter>() ? p.find<GrainFilter>()->getGrain() : 0);
    m_splithighlight->setValue(p.find<HighlightFilter>() ? p.find<HighlightFilter>()->getHighlight() : 0);
    m_splitshadow->setValue(p.find<ShadowFilter>() ? p.find<ShadowFilter>()->getShadow() : 0);
    m_vignette->setValue(p.find<VignetteFilter>() ? p.find<VignetteFilter>()->getVignette() : 0);

    //m_bw->setChecked(p.find<BWFilter>() != nullptr);

    qDebug() << "After setValue - brightness:" << m_brightness->getValue();

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


