#ifndef CROPCOMMAND_H
#define CROPCOMMAND_H

#include "command.h"
#include "layers/layermanager.h"
#include <QImage>
#include <QRect>
#include <functional>

class CropCommand : public Command
{
public:
    CropCommand(LayerManager& mgr, const QRect& cropRect);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_mgr;
    QRect m_cropRect;

    QSize m_oldCanvasSize;
    QSize m_newCanvasSize;

    struct LayerState {
        std::shared_ptr<PixelLayer> layer;
        QImage before;
        QPointF oldOffset;
    };

    std::vector<LayerState> m_layers;
};


#endif // CROPCOMMAND_H
