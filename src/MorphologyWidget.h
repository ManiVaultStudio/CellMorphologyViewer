#pragma once

#include "Scene.h"

#include "MorphologyLineRenderer.h"
#include "MorphologyTubeRenderer.h"

#include "MorphologyDescription.h"

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
    MorphologyWidget(CellMorphologyView* plugin, Scene* scene);
    ~MorphologyWidget();

    void setRenderMode(RenderMode renderMode)
    {
        _renderMode = renderMode;
    }

    void updateNeuron(NeuronDescriptor nd);

    void setRowWidth(float rowWidth);
    void setCellMorphology(const CellMorphology& cellMorphology);
    void setCellMetadata(QString cellId, QString subclass) { _cellId = cellId; _subclass = subclass; }
    void setCellMorphologyData(const MorphologyDescription& desc) { _desc = desc; }

    void renderCell(int index);

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

    Scene* _scene;

    NeuronDescriptor _nd;

    //QPixmap _morphologyImage;
    //QPixmap _evImage;
    //QPixmap _wheelImage;

    float t = 0;

    QString _cellId;
    QString _subclass;

    MorphologyDescription _desc;

    MorphologyLineRenderer _lineRenderer;
    MorphologyTubeRenderer _tubeRenderer;
    RenderMode _renderMode;
};
