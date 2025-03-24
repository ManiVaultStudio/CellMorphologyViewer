#include "LayerDrawing.h"

#include "Scene.h"

int MARGIN = 64;

LayerDrawing::LayerDrawing(QWidget* parent) :
    _parent(parent),
    _minDepth(0),
    _maxDepth(1),
    _depthRange(1)
{

}

void LayerDrawing::setDepthRange(float minDepth, float maxDepth)
{
    _minDepth = minDepth;
    _maxDepth = maxDepth;
    _depthRange = maxDepth - minDepth;
}

void LayerDrawing::drawAxes(QPainter& painter, Scene* scene)
{
    int chartWidth = _parent->width() - MARGIN * 2;
    int chartHeight = _parent->height() - MARGIN * 2;

    const CortexStructure& cortexStructure = scene->getCortexStructure();

    //int lightness = 240;
    for (int i = 0; i < cortexStructure._layerDepths.size() - 1; i++)
    {
        float layerDepthTop = cortexStructure.getLayerDepth(i);
        float layerDepthBottom = cortexStructure.getLayerDepth(i+1);

        int topY = (layerDepthTop / _depthRange) * chartHeight + MARGIN;
        int bottomY = (layerDepthBottom / _depthRange) * chartHeight + MARGIN;

        QPen midPen(QColor(80, 80, 80, 255), 1, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin);
        painter.setPen(midPen);
        drawHorizontalLine(painter, topY);
        drawHorizontalLine(painter, bottomY);

        //painter.fillRect(MARGIN, topY, chartWidth, abs(topY - bottomY), QColor::fromHsl(0, 0, lightness));
        //lightness -= 2;
    }

    QPen leftAxis(QColor(80, 80, 80, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(leftAxis);
    painter.drawLine(MARGIN, MARGIN, MARGIN, _parent->height() - MARGIN);
}

void LayerDrawing::drawHorizontalLine(QPainter& painter, float y)
{
    painter.drawLine(MARGIN, y, _parent->width() - MARGIN, y);
}
