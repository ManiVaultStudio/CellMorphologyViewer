#pragma once

#include "MorphologyLineRenderer.h"
#include "MorphologyTubeRenderer.h"

#include "NeuronDescriptor.h"

#include "graphics/Vector3f.h"

#include <QOpenGLWidget>

#include <QNetworkReply>
#include <QPixmap>

#include <vector>
#include <unordered_map>

class CellMorphologyView;
class CellMorphology;

class MorphologyWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MorphologyWidget(CellMorphologyView* plugin);
    ~MorphologyWidget();

    void updateNeuron(NeuronDescriptor nd);
    void setCellMorphology(const CellMorphology& cellMorphology);

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

    void downloadFinished(QNetworkReply* reply);

signals:
    void changeNeuron(QString neuronId);

private:
    bool isInitialized = false;

    NeuronDescriptor _nd;

    QPixmap _morphologyImage;
    QPixmap _evImage;
    QPixmap _wheelImage;

    MorphologyTubeRenderer _renderer;
};
