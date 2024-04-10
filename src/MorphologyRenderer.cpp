#include "MorphologyRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

void MorphologyRenderer::resize(int w, int h)
{
    glViewport(0, 0, w, h);

    _aspectRatio = (float) w / h;

    _projMatrix.setToIdentity();
    _projMatrix.ortho(-_aspectRatio, _aspectRatio, -1, 1, -1, 1);
}
