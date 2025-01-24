#include "MorphologyWidget.h"

#include "CellMorphologyView.h"

#include "CellMorphologyData/CellMorphology.h"
//#include "ImageQuery.h"

#include <QPainter>
//#include <QNetworkAccessManager>
//#include <QSslSocket>
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

void MorphologyWidget::setCellMorphology(const CellMorphology& cellMorphology)
{
    if (!isInitialized)
        return;

    makeCurrent();
    _lineRenderer.buildRenderObjects();
    //makeCurrent();
    //_lineRenderer.setCellMorphology(cellMorphology);
    //_tubeRenderer.setCellMorphology(cellMorphology);
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

void MorphologyWidget::renderCell(int index)
{
    switch (_renderMode)
    {
    case RenderMode::LINE: _lineRenderer.render(index, t); break;
    case RenderMode::REAL: _tubeRenderer.render(index, t); break;
    default: _lineRenderer.render(index, t);
    }
}

void MorphologyWidget::paintGL()
{
    t += 0.3f;

    int numObjects = _lineRenderer.getNumRenderObjects();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (numObjects < 1)
        return;

    //float renderWidth = width() / numObjects;
    //for (int i = 0; i < _lineRenderer.getNumRenderObjects(); i++)
    //{
    //    glViewport(renderWidth * i, 0, renderWidth * (i+1), height() / 2);
    //    renderCell(i);
    //}

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

    mv::Dataset<CellMorphologies> morphologyDataset = _scene->getCellMorphologies();
    mv::Dataset<Text> cellMetadata = _scene->getCellMetadata();
    const auto& selectionIndices = morphologyDataset->getSelectionIndices();
    QStringList morphCellIds = morphologyDataset->getCellIdentifiers();
    std::vector<QString> cellIds = cellMetadata->getColumn("Cell ID");
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
                qDebug() << "CellID: " << cellIds[ci];
                qDebug() << "Subclass: " << subclasses[ci];
                painter.drawText(offset[i] * width()-16, 16, subclasses[ci]);
            }
        }
    }

    painter.beginNativePainting();

    renderCell(0);
    painter.endNativePainting();

    //QFont font = painter.font();
    //font.setPointSize(14);
    //painter.setFont(font);
    //painter.setPen(QPen(Qt::white));
    //painter.drawText(25, 60, "Cell ID: " + _cellId);
    //
    //font.setPointSize(14);
    //painter.setFont(font);
    //painter.drawText(25, 80, "Class: " + _nd.tTypeSubClass);

    //font.setPointSize(14);
    //painter.setFont(font);
    //painter.drawText(25, 100, "Subclass: " + _subclass);

    //font.setPointSize(14);
    //painter.setFont(font);
    //painter.drawText(25, 120, "Cortical Layer: " + _nd.corticalLayer);

    //painter.fillRect(20, 130, 200, 200, QColor(64, 64, 64));
    //font.setPointSize(10);
    //painter.setFont(font);
    //painter.drawText(25, 140, "Apical Dendrite Bias X: " + QString::number(_desc.getApicalDendriteDescription().bias.x));
    //painter.drawText(25, 155, "Num branches - " + QString::number(_desc.getApicalDendriteDescription().numBranches));
    //painter.drawText(25, 170, QString("Extent - x:%1, y:%2").arg(_desc.getApicalDendriteDescription().extent.x).arg(_desc.getApicalDendriteDescription().extent.y));

    //painter.drawPixmap(-30, -30, 300, 300, _wheelImage);

    //painter.drawPixmap(0, 250, 400, 300, _morphologyImage);

    //painter.drawPixmap(0, 500, 300, 300, _evImage);

    painter.end();
}

void MorphologyWidget::cleanup()
{

}

void MorphologyWidget::updateNeuron(NeuronDescriptor nd)
{
    _nd = nd;

    //qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();

    //QByteArray ba = loadImage(_nd.morphologyUrl);

    //_morphologyImage.loadFromData(ba);

    //ba = loadImage(_nd.evUrl);
    //_evImage.loadFromData(ba);

    //ba = loadImage(_nd.wheelUrl);
    //_wheelImage.loadFromData(ba);

    //QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    //connect(nam, &QNetworkAccessManager::finished, this, &MorphologyWidget::downloadFinished);
    //const QUrl url = QUrl(_nd.morphologyUrl);
    //QNetworkRequest request(url);
    //nam->get(request);
}

//void MorphologyWidget::downloadFinished(QNetworkReply* reply)
//{
//    _morphologyImage.loadFromData(reply->readAll());
//}

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

    case QEvent::MouseButtonRelease:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        break;
    }

    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        //t = mouseEvent->x();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
