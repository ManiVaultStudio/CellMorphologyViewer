#pragma once

#include "Scene.h"

#include "MorphologyLineRenderer.h"
#include "MorphologyTubeRenderer.h"

#include "graphics/Vector3f.h"

#include <QOpenGLWidget>

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

    void setRowWidth(float rowWidth);
    void uploadMorphologies();

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

private:
    bool isInitialized = false;

    Scene* _scene;

    float t = 0;

    MorphologyLineRenderer _lineRenderer;
    MorphologyTubeRenderer _tubeRenderer;
    RenderMode _renderMode;
};
