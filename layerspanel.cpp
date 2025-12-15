#include "layerspanel.h"

#include <QHBoxLayout>
#include <QLabel>

#include <QPainter>




LayersPanel::LayersPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_list, 1);

    connect(m_list, &QListWidget::currentItemChanged, this, [this](QListWidgetItem*, QListWidgetItem*) {
        onSelectionChanged();
    });
    connect(m_list, &QListWidget::itemChanged, this, &LayersPanel::onItemChanged);

    auto* buttonsLayout = new QHBoxLayout();
    m_addButton = new QPushButton(QIcon(":/icons/layerspanel/add.svg"), tr("Brush Layer"), this);
    m_addAdjButton = new QPushButton(QIcon(":/icons/layerspanel/addfilter.svg"), tr("Adjustment Layer"), this);
    m_addImageButton = new QPushButton(QIcon(":/icons/layerspanel/addimg.svg"), tr("Image Layer"), this);
    m_deleteButton = new QPushButton(QIcon(":/icons/layerspanel/delete.svg"), tr("Delete"), this);
    m_moveUpButton = new QPushButton(QIcon(":/icons/layerspanel/up.svg"), tr("Up"), this);
    m_moveDownButton = new QPushButton(QIcon(":/icons/layerspanel/down.svg"), tr("Down"), this);

    buttonsLayout->addWidget(m_addButton);
    buttonsLayout->addWidget(m_addAdjButton);
    buttonsLayout->addWidget(m_addImageButton);
    buttonsLayout->addWidget(m_deleteButton);
    buttonsLayout->addWidget(m_moveUpButton);
    buttonsLayout->addWidget(m_moveDownButton);
    layout->addLayout(buttonsLayout);

    connect(m_addButton, &QPushButton::clicked, this, &LayersPanel::onAddLayer);
    connect(m_addAdjButton, &QPushButton::clicked, this, &LayersPanel::addAdjustmentLayerRequested);
    connect(m_addImageButton, &QPushButton::clicked, this, &LayersPanel::onAddImageLayer);
    connect(m_deleteButton, &QPushButton::clicked, this, &LayersPanel::onDeleteLayer);
    connect(m_moveUpButton, &QPushButton::clicked, this, &LayersPanel::onMoveUp);
    connect(m_moveDownButton, &QPushButton::clicked, this, &LayersPanel::onMoveDown);



    auto* opacityLayout = new QVBoxLayout();
    auto* opacityRow = new QHBoxLayout();
    auto* opacityLabel = new QLabel(tr("Opacity"), this);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(100);
    m_opacitySpin = new QDoubleSpinBox(this);
    m_opacitySpin->setRange(0.0, 1.0);
    m_opacitySpin->setSingleStep(0.01);
    m_opacitySpin->setDecimals(2);
    m_opacitySpin->setValue(1.0);
    m_opacitySpin->setFixedWidth(80);

    opacityRow->addWidget(opacityLabel);
    opacityRow->addWidget(m_opacitySlider, 1);
    opacityRow->addWidget(m_opacitySpin);
    opacityLayout->addLayout(opacityRow);
    layout->addLayout(opacityLayout);

    auto* blendLayout = new QHBoxLayout();
    auto* blendLabel = new QLabel(tr("Blend"), this);
    m_blendModeCombo = new QComboBox(this);
    m_blendModeCombo->setObjectName("blendComboBox");
    m_blendModeCombo->addItem("Normal");
    m_blendModeCombo->addItem("Multiply");
    m_blendModeCombo->addItem("Screen");
    m_blendModeCombo->addItem("Overlay");
    blendLayout->addWidget(blendLabel);
    blendLayout->addWidget(m_blendModeCombo);
    layout->addLayout(blendLayout);

    auto* clipLayout = new QHBoxLayout();
    m_clipCheck = new QCheckBox(tr("Clip to layer below"), this);
    clipLayout->addWidget(m_clipCheck);
    clipLayout->addStretch();
    layout->addLayout(clipLayout);

    connect(m_clipCheck, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                if (m_blockSignals) return;

                int index = selectedManagerIndex();
                if (index < 0) return;

                emit clippedChanged(index, checked);
            });

    connect(m_opacitySlider, &QSlider::sliderPressed, this, &LayersPanel::onOpacitySliderPressed);
    connect(m_opacitySlider, &QSlider::sliderReleased, this, &LayersPanel::onOpacitySliderReleased);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &LayersPanel::onOpacityValueChanged);
    connect(m_opacitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LayersPanel::onOpacitySpinChanged);
    connect(m_blendModeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int index)
            {
                if (m_blockSignals) return;
                int managerIndex = selectedManagerIndex();
                if (managerIndex < 0) return;

                emit blendModeChanged(managerIndex, index);
            });

}

bool LayersPanel::isAdjustmentLayer(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_layers.size()))
        return false;

    return m_layers[index]->type() == LayerType::Adjustment;
}


void LayersPanel::setLayers(const std::vector<std::shared_ptr<Layer>>& layers, int activeIndex)
{
    m_blockSignals = true;
    m_layers = layers;
    m_list->clear();

    int uiRowToSelect = -1;
    for (int i = static_cast<int>(layers.size()) - 1; i >= 0; --i)
    {
        auto& layer = layers[static_cast<size_t>(i)];
        auto* item = new QListWidgetItem(layer ? layer->name() : tr("Layer"), m_list);
        item->setData(Qt::UserRole, i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setCheckState(layer && layer->isVisible() ? Qt::Checked : Qt::Unchecked);

        if (dynamic_cast<PixelLayer*>(layer.get()))
        {
            item->setIcon(QIcon(":/icons/layerspanel/layer.svg"));
        }
        else if (dynamic_cast<AdjustmentLayer*>(layer.get()))
        {
            item->setIcon(QIcon(":/icons/layerspanel/adjlayer.svg"));
        }

        m_list->addItem(item);

        int uiRow = static_cast<int>(layers.size()) - 1 - i;
        if (i == activeIndex)
            uiRowToSelect = uiRow;
    }

    if (uiRowToSelect >= 0 && uiRowToSelect < m_list->count())
    {
        m_list->setCurrentRow(uiRowToSelect);
    }
    else if (m_list->count() > 0)
    {
        m_list->setCurrentRow(0);
    }

    int managerIndex = selectedManagerIndex();
    updateOpacityControls(managerIndex);

    m_blockSignals = false;
}

int LayersPanel::selectedManagerIndex() const
{
    auto* current = m_list->currentItem();
    if (!current)
        return -1;

    return current->data(Qt::UserRole).toInt();
}

float LayersPanel::currentOpacityFromLayer(int managerIndex) const
{
    if (managerIndex < 0 || managerIndex >= static_cast<int>(m_layers.size()))
        return 1.0f;

    const auto& layer = m_layers[static_cast<size_t>(managerIndex)];
    if (!layer)
        return 1.0f;

    return layer->opacity();
}

void LayersPanel::updateOpacityControls(int managerIndex)
{
    m_blockSignals = true;

    bool hasSelection = managerIndex >= 0 &&
                        managerIndex < static_cast<int>(m_layers.size());

    m_opacitySlider->setEnabled(hasSelection);
    m_opacitySpin->setEnabled(hasSelection);
    m_blendModeCombo->setEnabled(hasSelection);

    if (hasSelection)
    {
        auto& layer = m_layers[managerIndex];

        float opacity = layer ? layer->opacity() : 1.0f;
        m_opacitySlider->setValue(static_cast<int>(opacity * 100.0f));
        m_opacitySpin->setValue(opacity);

        m_blendModeCombo->setCurrentIndex(
            static_cast<int>(layer->blendMode())
            );
    }
    else
    {
        m_opacitySlider->setValue(100);
        m_opacitySpin->setValue(1.0);
        m_blendModeCombo->setCurrentIndex(0);
    }

    if (hasSelection && isAdjustmentLayer(managerIndex))
    {
        m_clipCheck->setVisible(true);
        m_clipCheck->setChecked(m_layers[managerIndex]->isClipped());
    }
    else
    {
        m_clipCheck->setVisible(false);
    }

    m_blockSignals = false;
}



void LayersPanel::onSelectionChanged()
{
    if (m_blockSignals)
        return;

    int managerIndex = selectedManagerIndex();
    updateOpacityControls(managerIndex);
    emit activeLayerChanged(managerIndex);
}

void LayersPanel::onItemChanged(QListWidgetItem* item)
{
    if (m_blockSignals || !item)
        return;

    int managerIndex = item->data(Qt::UserRole).toInt();
    bool visible = item->checkState() == Qt::Checked;
    emit visibilityToggled(managerIndex, visible);
}

void LayersPanel::onAddLayer()
{
    emit addLayerRequested();
}

void LayersPanel::onDeleteLayer()
{
    int managerIndex = selectedManagerIndex();
    if (managerIndex < 0)
        return;

    emit deleteLayerRequested(managerIndex);
}

void LayersPanel::onMoveUp()
{
    int managerIndex = selectedManagerIndex();
    if (managerIndex < 0)
        return;

    int target = managerIndex + 1;
    emit moveLayerRequested(managerIndex, target);
}

void LayersPanel::onMoveDown()
{
    int managerIndex = selectedManagerIndex();
    if (managerIndex <= 0)
        return;

    int target = managerIndex - 1;
    emit moveLayerRequested(managerIndex, target);
}

void LayersPanel::onOpacitySliderPressed()
{
}

void LayersPanel::onOpacitySliderReleased()
{
    if (m_blockSignals)
        return;

    int managerIndex = selectedManagerIndex();
    if (managerIndex < 0)
        return;

    float newOpacity = static_cast<float>(m_opacitySlider->value()) / 100.0f;
    emit opacityChanged(managerIndex, newOpacity);
}

void LayersPanel::onOpacityValueChanged(int value)
{
    if (m_blockSignals)
        return;

    float normalized = static_cast<float>(value) / 100.0f;
    m_blockSignals = true;
    m_opacitySpin->setValue(normalized);
    m_blockSignals = false;
}

void LayersPanel::onOpacitySpinChanged(double value)
{
    if (m_blockSignals)
        return;

    m_blockSignals = true;
    m_opacitySlider->setValue(static_cast<int>(value * 100.0));
    m_blockSignals = false;

    int managerIndex = selectedManagerIndex();
    if (managerIndex >= 0)
    {
        emit opacityChanged(managerIndex, static_cast<float>(value));
    }
}

void LayersPanel::onAddImageLayer()
{
    emit addImageLayerRequested();
}

