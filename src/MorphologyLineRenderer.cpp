#include "MorphologyLineRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

void MorphologyLineRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":shaders/PassThrough.vert", ":shaders/Lines.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    glEnable(GL_LINE_SMOOTH);
}

void MorphologyLineRenderer::render(int index, float t)
{
    float totalFramesWidth = 0;
    float maxCellHeight = 0;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];
        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2));

        maxCellHeight = cellRenderObject.ranges.y > maxCellHeight ? cellRenderObject.ranges.y : maxCellHeight;

        totalFramesWidth += maxWidth;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    _lineShader.bind();

    float framesAspectRatio = totalFramesWidth / maxCellHeight;

    float scalingFactor = 1;
    if (framesAspectRatio > _aspectRatio)
    {
        scalingFactor = totalFramesWidth / _aspectRatio;
    }
    else
        scalingFactor = maxCellHeight;

    _viewMatrix.setToIdentity();
    _viewMatrix.scale(1.8 / scalingFactor);

    //float maxYExtent = 0;
    float xOffset = 0;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        mv::Vector3f centroid = cellRenderObject.centroid;

        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2));

        //qDebug() << "YOffset" << yOffset;
        _modelMatrix.setToIdentity();
        _modelMatrix.translate(-totalFramesWidth /2 + maxWidth /2 + xOffset, 0, 0);
        _modelMatrix.rotate(t, 0, 1, 0);
        _modelMatrix.translate(-centroid.x, -centroid.y, -centroid.z);

        _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
        _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        glBindVertexArray(cellRenderObject.vao);
        glDrawArrays(GL_LINES, 0, cellRenderObject.numVertices);
        glBindVertexArray(0);

        xOffset += maxWidth;
    }

    _lineShader.release();
}

void MorphologyLineRenderer::buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject)
{
    MorphologyLineSegments lineSegments;

    mv::Vector3f                somaPosition;
    float                       somaRadius;

    // Generate line segments
    try
    {
        for (int i = 0; i < cellMorphology.ids.size(); i++)
        {
            mv::Vector3f position = cellMorphology.positions.at(i);
            float radius = cellMorphology.radii.at(i);

            if (cellMorphology.types.at(i) == 1) // Soma
            {
                somaPosition = position;
                somaRadius = radius;
                break;
            }
        }

        for (int i = 1; i < cellMorphology.parents.size(); i++)
        {
            if (cellMorphology.parents[i] == -1) // New root found, there is no line segment here so skip it
                continue;

            int id = cellMorphology.idMap.at(cellMorphology.ids[i]);
            int parent = cellMorphology.idMap.at(cellMorphology.parents[i]);

            lineSegments.segments.push_back(cellMorphology.positions[parent]);
            lineSegments.segments.push_back(cellMorphology.positions[id]);
            lineSegments.segmentRadii.push_back(cellMorphology.radii[id]);
            lineSegments.segmentRadii.push_back(cellMorphology.radii[id]);
            lineSegments.segmentTypes.push_back(cellMorphology.types[id]);
            lineSegments.segmentTypes.push_back(cellMorphology.types[id]);
        }
    }
    catch (std::out_of_range& oor)
    {
        qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
        return;
    }

    // Initialize VAO and VBOs
    glGenVertexArrays(1, &cellRenderObject.vao);
    glBindVertexArray(cellRenderObject.vao);

    glGenBuffers(1, &cellRenderObject.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cellRenderObject.rbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &cellRenderObject.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Store data on GPU
    glBindVertexArray(cellRenderObject.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segments.size() * sizeof(mv::Vector3f), lineSegments.segments.data(), GL_STATIC_DRAW);
    qDebug() << "VBO size: " << (lineSegments.segments.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentRadii.size() * sizeof(float), lineSegments.segmentRadii.data(), GL_STATIC_DRAW);
    qDebug() << "RBO size: " << (lineSegments.segmentRadii.size() * sizeof(float)) / 1000 << "kb";
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentTypes.size() * sizeof(int), lineSegments.segmentTypes.data(), GL_STATIC_DRAW);
    qDebug() << "TBO size: " << (lineSegments.segmentTypes.size() * sizeof(int)) / 1000 << "kb";

    cellRenderObject.numVertices = lineSegments.segments.size();

    cellRenderObject.centroid = somaPosition;
    mv::Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;
    float maxExtent = std::max(std::max(range.x, range.y), range.z);
    cellRenderObject.ranges = range;
    cellRenderObject.maxExtent = maxExtent;

    _morphologyView = cellRenderObject;
}
