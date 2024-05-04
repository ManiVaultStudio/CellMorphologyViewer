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

void MorphologyLineRenderer::setCellMorphology(const CellMorphology& cellMorphology)
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
    MorphologyView morphView;
    glGenVertexArrays(1, &morphView.vao);
    glBindVertexArray(morphView.vao);

    glGenBuffers(1, &morphView.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, morphView.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &morphView.rbo);
    glBindBuffer(GL_ARRAY_BUFFER, morphView.rbo);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &morphView.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, morphView.tbo);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Store data on GPU
    glBindVertexArray(morphView.vao);

    glBindBuffer(GL_ARRAY_BUFFER, morphView.vbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segments.size() * sizeof(mv::Vector3f), lineSegments.segments.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, morphView.rbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentRadii.size() * sizeof(float), lineSegments.segmentRadii.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, morphView.tbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentTypes.size() * sizeof(int), lineSegments.segmentTypes.data(), GL_STATIC_DRAW);

    morphView.numVertices = lineSegments.segments.size();

    morphView.centroid = somaPosition;
    mv::Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;
    float maxExtent = std::max(std::max(range.x, range.y), range.z);
    morphView.maxExtent = maxExtent;

    _morphologyView = morphView;
}

void MorphologyLineRenderer::update(float t)
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mv::Vector3f centroid = _morphologyView.centroid;
    float maxExtent = _morphologyView.maxExtent / 2;

    _viewMatrix.setToIdentity();
    _viewMatrix.scale(1.0f / maxExtent);
    _viewMatrix.rotate(t, 0, 1, 0);
    _viewMatrix.translate(-centroid.x, -centroid.y, -centroid.z);

    _lineShader.bind();
    _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
    _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

    glBindVertexArray(_morphologyView.vao);
    glDrawArrays(GL_LINES, 0, _morphologyView.numVertices);
    glBindVertexArray(0);

    _lineShader.release();
}
