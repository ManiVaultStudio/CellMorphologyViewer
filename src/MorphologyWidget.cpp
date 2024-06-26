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

MorphologyWidget::MorphologyWidget(CellMorphologyView* plugin) :
    _renderMode(RenderMode::LINE)
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
    surfaceFormat.setSamples(16);

    setFormat(surfaceFormat);
}

MorphologyWidget::~MorphologyWidget()
{

}

void MorphologyWidget::setCellMorphology(const CellMorphology& cellMorphology)
{
    if (!isInitialized)
        return;

    makeCurrent();
    _lineRenderer.setCellMorphology(cellMorphology);
    _tubeRenderer.setCellMorphology(cellMorphology);
}

void MorphologyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    _lineRenderer.init();
    _tubeRenderer.init();

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(50);

    isInitialized = true;
}

void MorphologyWidget::resizeGL(int w, int h)
{
    _lineRenderer.resize(w, h);
    _tubeRenderer.resize(w, h);
}

void MorphologyWidget::paintGL()
{
    t += 1.6f;

    switch (_renderMode)
    {
        case RenderMode::LINE: _lineRenderer.update(t); break;
        case RenderMode::REAL: _tubeRenderer.update(t); break;
        default: _lineRenderer.update(t);
    }

    QPainter painter(this);

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
