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

    setFormat(surfaceFormat);
}

MorphologyWidget::~MorphologyWidget()
{

}

void MorphologyWidget::setCellMorphology(const CellMorphology& cellMorphology)
{
    if (!isInitialized)
        return;

    _segments.clear();
    _segmentRadii.clear();
    _segmentTypes.clear();

    // Generate line segments
    try
    {
        for (int i = 1; i < cellMorphology.parents.size(); i++)
        {
            if (cellMorphology.parents[i] == -1) // New root found, there is no line segment here so skip it
                continue;

            int id = cellMorphology.idMap.at(cellMorphology.ids[i]);
            int parent = cellMorphology.idMap.at(cellMorphology.parents[i]);

            _segments.push_back(cellMorphology.positions[parent]);
            _segments.push_back(cellMorphology.positions[id]);
            _segmentRadii.push_back(cellMorphology.radii[id]);
            _segmentRadii.push_back(cellMorphology.radii[id]);
            _segmentTypes.push_back(cellMorphology.types[id]);
            _segmentTypes.push_back(cellMorphology.types[id]);
        }
    }
    catch (std::out_of_range& oor)
    {
        qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
        return;
    }

    // Store data on GPU
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, _segments.size() * sizeof(Vector3f), _segments.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, rbo);
    glBufferData(GL_ARRAY_BUFFER, _segmentRadii.size() * sizeof(float), _segmentRadii.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, _segmentTypes.size() * sizeof(int), _segmentTypes.data(), GL_STATIC_DRAW);
}

void MorphologyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":shaders/PassThrough.vert", ":shaders/Lines.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    // Set projection matrix
    //_projMatrix.ortho(-1, 1, -1, 1, -1, 1);
    _viewMatrix.translate(0, 0, 0);

    for (int i = 0; i < 16; i++)
    {
        qDebug() << _projMatrix.constData()[i];
    }

    // Initialize VAO and VBOs
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &rbo);
    glBindBuffer(GL_ARRAY_BUFFER, rbo);;
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(50);

    isInitialized = true;
}

void MorphologyWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    float aspect = w / h;
    _projMatrix.setToIdentity();
    _projMatrix.ortho(-aspect, aspect, -1, 1, -1, 1);
}
float t = 0;
void MorphologyWidget::paintGL()
{
    QPainter painter(this);

    painter.beginNativePainting();
    
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    //qDebug() << t;
    t += 1.6f;
    _viewMatrix.setToIdentity();
    _viewMatrix.rotate(t, 0, 1, 0);

    _lineShader.bind();
    _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
    _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, _segments.size());

    _lineShader.release();
    glBindVertexArray(0);

    painter.endNativePainting();

    QFont font = painter.font();
    font.setPointSize(24);
    painter.setFont(font);
    painter.setPen(QPen(Qt::white));
    painter.drawText(25, 60, "T-Type Class: " + _nd.tTypeClass);
    
    font.setPointSize(16);
    painter.setFont(font);
    painter.drawText(25, 100, "T-Type Subclass: " + _nd.tTypeSubClass);

    font.setPointSize(14);
    painter.setFont(font);
    painter.drawText(25, 140, "T-Type: " + _nd.tType);

    font.setPointSize(14);
    painter.setFont(font);
    painter.drawText(25, 180, "Cortical Layer: " + _nd.corticalLayer);

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

        QString neuronId = "592479953";
        if (mouseEvent->x() > 500)
            neuronId = "595572609";
        
        emit changeNeuron(neuronId);
        qDebug() << "Mouse button press";

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

        t = mouseEvent->x();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
