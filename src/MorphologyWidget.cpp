#include "MorphologyWidget.h"

#include "CellMorphologyView.h"

#include "CellMorphologyData/CellMorphology.h"
#include "ImageQuery.h"

#include <QPainter>
#include <QNetworkAccessManager>
#include <QSslSocket>
#include <QEvent>
#include <QResizeEvent>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace mv;

MorphologyWidget::MorphologyWidget(CellMorphologyView* plugin)
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
    _renderer.setCellMorphology(cellMorphology);
}

void MorphologyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    _renderer.init();

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(50);

    isInitialized = true;
}

void MorphologyWidget::resizeGL(int w, int h)
{
    _renderer.resize(w, h);
}

void MorphologyWidget::paintGL()
{
    _renderer.update();

    QPainter painter(this);

    QFont font = painter.font();
    font.setPointSize(14);
    painter.setFont(font);
    painter.setPen(QPen(Qt::white));
    painter.drawText(25, 60, "Cell ID: " + _cellId);
    
    font.setPointSize(14);
    painter.setFont(font);
    painter.drawText(25, 80, "Class: " + _nd.tTypeSubClass);

    font.setPointSize(14);
    painter.setFont(font);
    painter.drawText(25, 100, "Subclass: " + _subclass);

    font.setPointSize(14);
    painter.setFont(font);
    painter.drawText(25, 120, "Cortical Layer: " + _nd.corticalLayer);

    painter.drawPixmap(-30, -30, 300, 300, _wheelImage);

    painter.drawPixmap(0, 250, 400, 300, _morphologyImage);

    painter.drawPixmap(0, 500, 300, 300, _evImage);

    painter.end();
}

void MorphologyWidget::cleanup()
{

}

void MorphologyWidget::updateNeuron(NeuronDescriptor nd)
{
    _nd = nd;

    qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();

    QByteArray ba = loadImage(_nd.morphologyUrl);

    _morphologyImage.loadFromData(ba);

    ba = loadImage(_nd.evUrl);
    _evImage.loadFromData(ba);

    ba = loadImage(_nd.wheelUrl);
    _wheelImage.loadFromData(ba);

    //QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    //connect(nam, &QNetworkAccessManager::finished, this, &MorphologyWidget::downloadFinished);
    //const QUrl url = QUrl(_nd.morphologyUrl);
    //QNetworkRequest request(url);
    //nam->get(request);
}

void MorphologyWidget::downloadFinished(QNetworkReply* reply)
{
    _morphologyImage.loadFromData(reply->readAll());
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
