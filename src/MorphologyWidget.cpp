#include "MorphologyWidget.h"

#include "CellMorphologyView.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace hdps;

MorphologyWidget::MorphologyWidget(CellMorphologyView* plugin)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    installEventFilter(this);
}

MorphologyWidget::~MorphologyWidget()
{

}

void MorphologyWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":shaders/PassThrough.vert", ":shaders/Lines.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    // Load swc data
    //Bstd::string filename = "Tlx3-Cre_PL56_Ai14-318845.02.02.01_652303152_m.swc";
    std::string filename = "C:/Users/julia/Downloads/Sst-IRES-Cre_Ai14-167636.04.01.01_491119515_m.swc";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Error opening file." << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> data;

    // Read in headers
    if (getline(file, line)) {
        std::stringstream ss(line);
        std::string header;

        while (getline(ss, header, ' ')) {
            headers.push_back(header);
        }
    }

    // Read in data
    // #n type x y z radius parent
    // 0 1 303.3888 442.0416 26.3200 6.0149 - 1
    std::vector<int> ids;
    std::vector<int> types;
    std::vector<float> radii;
    std::vector<int> parents;

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string value;

        int i = 0;
        while (getline(ss, value, ' ')) {
            //row.push_back(value);
            switch (i)
            {
            case 0: ids.push_back(stoi(value)); break;
            case 1: types.push_back(stoi(value)); break;
            case 2: 
            {
                float x = stof(value);
                getline(ss, value, ' ');
                float y = stof(value);
                getline(ss, value, ' ');
                float z = stof(value);
                _positions.emplace_back(x, y, z);
                break;
            }
            case 3: radii.push_back(stof(value)); break;
            case 4: parents.push_back(stoi(value)); break;
            }
            i++;
        }

        data.push_back(row);
    }

    // Compute id to index map
    for (int i = 0; i < ids.size(); i++)
    {
        int id = ids[i];
        _idMap[id] = i;
    }

    // Print headers
    for (const auto& header : headers) {
        std::cout << header << "\t";
    }

    std::cout << std::endl;

    // Print data
    //for (const auto& position : _positions) {
    //    std::cout << position.str() << std::endl;
    //}


    // Find centroid and extents
    Vector3f avgPos;
    for (const auto& pos : _positions)
        avgPos += pos;
    avgPos /= _positions.size();

    // Center cell positions
    for (auto& pos : _positions)
        pos -= avgPos;

    // Find cell position ranges
    Vector3f minV(std::numeric_limits<float>::max());
    Vector3f maxV(-std::numeric_limits<float>::max());
    for (const auto& pos : _positions)
    {
        if (pos.x < minV.x) minV.x = pos.x;
        if (pos.y < minV.y) minV.y = pos.y;
        if (pos.z < minV.z) minV.z = pos.z;
        if (pos.x > maxV.x) maxV.x = pos.x;
        if (pos.y > maxV.y) maxV.y = pos.y;
        if (pos.z > maxV.z) maxV.z = pos.z;
    }
    Vector3f range = (maxV - minV);
    float maxRange = std::max(std::max(range.x, range.y), range.z);
    // Rescale positions
    for (auto& pos : _positions)
    {
        pos /= maxRange;
    }

    // Set projection matrix
    //_projMatrix.ortho(-1, 1, -1, 1, -1, 1);
    _viewMatrix.translate(0, 0, 0);

    std::cout << minV.str() << " " << maxV.str() << std::endl;
    for (int i = 0; i < 16; i++)
    {
        qDebug() << _projMatrix.constData()[i];
    }
    //for (auto& pos : _positions)
    //{
    //    std::cout << pos.str() << std::endl;
    //}

    // Generate line segments
    for (int i = 1; i < parents.size(); i++)
    {
        int id = _idMap[ids[i]];
        int parent = _idMap[parents[i]];
        if (parents[i] == -1)
            continue;
        _segments.push_back(_positions[parent]);
        _segments.push_back(_positions[id]);
        _segmentRadii.push_back(radii[id]);
        _segmentRadii.push_back(radii[id]);
        _segmentTypes.push_back(types[id]);
        _segmentTypes.push_back(types[id]);
    }

    //for (int& type : _segmentTypes)
    //    qDebug() << type;

    // Store data on GPU
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, _segments.size() * sizeof(Vector3f), _segments.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &rbo);
    glBindBuffer(GL_ARRAY_BUFFER, rbo);
    glBufferData(GL_ARRAY_BUFFER, _segmentRadii.size() * sizeof(float), _segmentRadii.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, _segmentTypes.size() * sizeof(int), _segmentTypes.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(50);
}

void MorphologyWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
float t = 0;
void MorphologyWidget::paintGL()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    qDebug() << t;
    t += 1.6f;
    _viewMatrix.setToIdentity();
    _viewMatrix.rotate(t, 0, 1, 0);

    _lineShader.bind();
    _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
    _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, _segments.size());
}

void MorphologyWidget::cleanup()
{

}

bool MorphologyWidget::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::Resize:
    {
        const auto resizeEvent = static_cast<QResizeEvent*>(event);

        break;
    }

    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        qDebug() << "Mouse button press";

        break;
    }

    case QEvent::MouseButtonRelease:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        break;
    }

    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        t = mouseEvent->x();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}
