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

enum class RenderMode
{
    LINE, REAL
};

class MorphologyWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MorphologyWidget(CellMorphologyView* plugin);
    ~MorphologyWidget();

    void setRenderMode(RenderMode renderMode)
    {
        _renderMode = renderMode;
    }
    void updateNeuron(NeuronDescriptor nd);
    void setCellMorphology(const CellMorphology& cellMorphology);
    void setCellMetadata(QString cellId, QString subclass) { _cellId = cellId; _subclass = subclass; }

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

    //void downloadFinished(QNetworkReply* reply);

signals:
    void changeNeuron(QString neuronId);

private:
    bool isInitialized = false;

    NeuronDescriptor _nd;

    QPixmap _morphologyImage;
    QPixmap _evImage;
    QPixmap _wheelImage;

    QString _cellId;
    QString _subclass;

    MorphologyLineRenderer _lineRenderer;
    MorphologyTubeRenderer _tubeRenderer;
    RenderMode _renderMode;
};
