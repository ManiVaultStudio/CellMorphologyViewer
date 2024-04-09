#include "MorphologyRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

void MorphologyRenderer::resize(int w, int h)
{
    glViewport(0, 0, w, h);

    float aspect = (float) w / h;

    _projMatrix.setToIdentity();
    _projMatrix.ortho(-aspect, aspect, -1, 1, -1, 1);
}
