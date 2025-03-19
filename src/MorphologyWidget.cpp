#include "MorphologyWidget.h"

#include "CellMorphologyView.h"

#include "CellMorphologyData/CellMorphology.h"

#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace mv;

MorphologyWidget::MorphologyWidget(CellMorphologyView* plugin, Scene* scene) :
    _renderMode(RenderMode::LINE),
    _scene(scene),
    _lineRenderer(scene),
    _tubeRenderer(scene)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    installEventFilter(this);

    QSurfaceFormat surfaceFormat;

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);

    // Ask for an OpenGL 4.3 Core Context as the default
    surfaceFormat.setVersion(4, 3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    surfaceFormat.setSamples(64);

    setFormat(surfaceFormat);
}

MorphologyWidget::~MorphologyWidget()
{

}

void MorphologyWidget::setRowWidth(float rowWidth)
{
    _lineRenderer.setRowWidth(rowWidth);
}

void MorphologyWidget::uploadMorphologies()
{
    if (!isInitialized)
        return;

    makeCurrent();
    _lineRenderer.buildRenderObjects();
}

void MorphologyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    _lineRenderer.init();
    _tubeRenderer.init();

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 60);

    isInitialized = true;
}

void MorphologyWidget::resizeGL(int w, int h)
{
    _lineRenderer.resize(w, h);
    _tubeRenderer.resize(w, h);
}

void MorphologyWidget::paintGL()
{
    t += 0.3f;

    int numObjects = _lineRenderer.getNumRenderObjects();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (numObjects < 1)
        return;

    std::vector<float> offset;
    _lineRenderer.getCellMetadataLocations(offset);

    QPainter painter(this);

    int MARGIN = 64;
    QPen midPen(QColor(80, 80, 80, 255), 3, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(midPen);
    painter.drawLine(MARGIN, height() / 2, width() - MARGIN, height() / 2);

    QPen leftAxis(QColor(80, 80, 80, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(leftAxis);
    painter.drawLine(MARGIN, MARGIN, MARGIN, height() - MARGIN);

    mv::Dataset<CellMorphologies> morphologyDataset = _scene->getMorphologyDataset();
    mv::Dataset<Text> cellMetadata = _scene->getCellMetadataDataset();
    const auto& selectionIndices = morphologyDataset->getSelectionIndices();
    QStringList morphCellIds = morphologyDataset->getCellIdentifiers();
    std::vector<QString> cellIds = cellMetadata->getColumn("Cell ID");

    if (cellMetadata->hasColumn("Subclass"))
    {
        std::vector<QString> subclasses = cellMetadata->getColumn("Subclass");

        QFont font = painter.font();
        //font.setPointSizeF(font.pointSizeF() * 2);
        painter.setFont(font);

        painter.setPen(QPen(Qt::black, 1));
        for (int i = 0; i < selectionIndices.size(); i++)
        {
            int si = selectionIndices[i];
            CellMorphology& morphology = morphologyDataset->getData()[si];

            QString morphCellId = morphCellIds[si];
            for (int ci = 0; ci < cellIds.size(); ci++)
            {
                if (cellIds[ci] == morphCellId)
                {
                    painter.drawText(offset[i] * width() - 16, 16, clusters[ci]);
                }
            }
        }
    }
    
    painter.beginNativePainting();

    switch (_renderMode)
    {
        case RenderMode::LINE: _lineRenderer.render(0, t); break;
        case RenderMode::REAL: _tubeRenderer.render(0, t); break;
        default: _lineRenderer.render(0, t);
    }

    painter.endNativePainting();

    painter.end();
}

void MorphologyWidget::cleanup()
{

}

bool MorphologyWidget::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::Resize:
    {
        const auto resizeEvent = static_cast<QResizeEvent*>(event);

        break;
    }

    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "Mouse click";

        makeCurrent();
        _tubeRenderer.reloadShaders();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
