#include "MorphologyRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

void MorphologyRenderer::resize(int w, int h)
{
    glViewport(0, 0, w, h);

    _aspectRatio = (float) w / h;

    _projMatrix.setToIdentity();
    _projMatrix.ortho(-_aspectRatio, _aspectRatio, -1, 1, -1, 1);
}

void MorphologyRenderer::update(float t)
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    
}

void MorphologyRenderer::buildRenderObjects()
{
    // Delete previous render objects
    for (CellRenderObject& cellRenderObject : _cellRenderObjects)
    {
        glDeleteBuffers(1, &cellRenderObject.vbo);
        glDeleteBuffers(1, &cellRenderObject.rbo);
        glDeleteBuffers(1, &cellRenderObject.tbo);
        glDeleteVertexArrays(1, &cellRenderObject.vao);
    }

    _cellRenderObjects.clear();

    mv::Dataset<CellMorphologies> morphologyDataset = _scene->getMorphologyDataset();

    const auto& selectionIndices = morphologyDataset->getSelectionIndices();

    _cellRenderObjects.resize(selectionIndices.size());
    qDebug() << "Build render objects: " << selectionIndices.size();
    for (int i = 0; i < selectionIndices.size(); i++)
    {
        int si = selectionIndices[i];
        CellMorphology& morphology = morphologyDataset->getData()[si];

        CellRenderObject& cellRenderObject = _cellRenderObjects[i];
        buildRenderObject(morphology, cellRenderObject);
    }
}
