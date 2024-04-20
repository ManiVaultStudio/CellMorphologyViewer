#pragma once

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"

#include <QOpenGLFunctions_3_3_Core>

#include <QMatrix4x4>

class CellMorphology;

class MorphologyView
{
public:
    GLuint vao = 0; // Vertex array object
    GLuint vbo = 0; // Vertex buffer object
    GLuint rbo = 0; // Radius buffer object
    GLuint tbo = 0; // Type buffer object

    int numVertices = 0;

    float maxExtent;
    mv::Vector3f centroid;
};

class MorphologyRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
    virtual void init() = 0;
    void resize(int w, int h);
    virtual void update(float t) = 0;

    virtual void setCellMorphology(const CellMorphology& cellMorphology) = 0;

protected:
    MorphologyView _morphologyView;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;

    float _aspectRatio;
};
