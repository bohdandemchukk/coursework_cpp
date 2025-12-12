#ifndef LAYERSPANEL_H
#define LAYERSPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>
#include <memory>

#include "layer.h"

class LayersPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LayersPanel(QWidget* parent = nullptr);

    void setLayers(const std::vector<std::shared_ptr<Layer>>& layers, int activeIndex);

signals:
    void activeLayerChanged(int managerIndex);
    void addLayerRequested();
    void deleteLayerRequested(int managerIndex);
    void moveLayerRequested(int fromIndex, int toIndex);
    void visibilityToggled(int managerIndex, bool visible);
    void opacityChanged(int managerIndex, float opacity);

private slots:
    void onSelectionChanged();
    void onItemChanged(QListWidgetItem* item);
    void onAddLayer();
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

    QListWidget* m_list {nullptr};
    QSlider* m_opacitySlider {nullptr};
    QDoubleSpinBox* m_opacitySpin {nullptr};
    QPushButton* m_addButton {nullptr};
    QPushButton* m_deleteButton {nullptr};
    QPushButton* m_moveUpButton {nullptr};
    QPushButton* m_moveDownButton {nullptr};

    std::vector<std::shared_ptr<Layer>> m_layers;
    bool m_blockSignals {false};
};

#endif
