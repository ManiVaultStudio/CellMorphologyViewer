#pragma once

#include "graphics/Vector3f.h"
#include "graphics/Shader.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>

#include <vector>
#include <unordered_map>

class CellMorphologyView;

class MorphologyWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MorphologyWidget(CellMorphologyView* plugin);
    ~MorphologyWidget();

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    bool eventFilter(QObject* target, QEvent* event);

private:
    hdps::ShaderProgram _lineShader;
    GLuint vao;
    GLuint vbo;
    GLuint rbo;
    GLuint tbo;

    std::unordered_map<int, int> _idMap;
    std::vector<hdps::Vector3f> _positions;
    std::vector<hdps::Vector3f> _segments;
    std::vector<float> _segmentRadii;
    std::vector<int> _segmentTypes;
    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;
};
