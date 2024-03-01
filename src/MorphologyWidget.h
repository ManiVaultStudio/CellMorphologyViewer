#pragma once

#include "NeuronDescriptor.h"

#include "graphics/Vector3f.h"
#include "graphics/Shader.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>

#include <QNetworkReply>
#include <QPixmap>

#include <vector>
#include <unordered_map>

class CellMorphologyView;
class Neuron;

class MorphologyWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MorphologyWidget(CellMorphologyView* plugin);
    ~MorphologyWidget();

    void updateNeuron(NeuronDescriptor nd);
    void setNeuron(Neuron& neuron);

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

    void downloadFinished(QNetworkReply* reply);

signals:
    void changeNeuron();

private:
    bool isInitialized = false;

    mv::ShaderProgram _lineShader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint rbo = 0;
    GLuint tbo = 0;

    std::vector<mv::Vector3f> _segments;
    std::vector<float> _segmentRadii;
    std::vector<int> _segmentTypes;
    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;

    NeuronDescriptor _nd;

    QPixmap _morphologyImage;
    QPixmap _evImage;
    QPixmap _wheelImage;
};
