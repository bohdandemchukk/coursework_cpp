#ifndef LAYERSPANEL_H
#define LAYERSPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <vector>
#include <memory>

#include "layer.h"

class LayersPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LayersPanel(QWidget* parent = nullptr);

    bool isAdjustmentLayer(int index) const;
    void setLayers(const std::vector<std::shared_ptr<Layer>>& layers, int activeIndex);

signals:
    void activeLayerChanged(int managerIndex);
    void clippedChanged(int index, bool clipped);
    void addLayerRequested();
    void addAdjustmentLayerRequested();
    void addImageLayerRequested();
    void deleteLayerRequested(int managerIndex);
    void moveLayerRequested(int fromIndex, int toIndex);
    void visibilityToggled(int managerIndex, bool visible);
    void opacityChanged(int managerIndex, float opacity);
    void blendModeChanged(int managerIndex, int mode);

private slots:
    void onSelectionChanged();
    void onItemChanged(QListWidgetItem* item);
    void onAddLayer();
    void onAddImageLayer();
    void onDeleteLayer();
    void onMoveUp();
    void onMoveDown();
    void onOpacitySliderPressed();
    void onOpacitySliderReleased();
    void onOpacityValueChanged(int value);
    void onOpacitySpinChanged(double value);

private:
    int selectedManagerIndex() const;
    float currentOpacityFromLayer(int managerIndex) const;
    void updateOpacityControls(int managerIndex);
    QComboBox* m_blendModeCombo {nullptr};
    QListWidget* m_list {nullptr};
    QSlider* m_opacitySlider {nullptr};
    QDoubleSpinBox* m_opacitySpin {nullptr};
    QCheckBox* m_clipCheck = nullptr;
    QPushButton* m_addButton {nullptr};
    QPushButton* m_addAdjButton {nullptr};
    QPushButton* m_addImageButton {nullptr};
    QPushButton* m_deleteButton {nullptr};
    QPushButton* m_moveUpButton {nullptr};
    QPushButton* m_moveDownButton {nullptr};

    std::vector<std::shared_ptr<Layer>> m_layers;
    bool m_blockSignals {false};
};

#endif
