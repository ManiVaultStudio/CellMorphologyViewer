#pragma once

#include <PointData/PointData.h>
#include <TextData/TextData.h>
#include <CellMorphologyData/CellMorphologyData.h>

class Scene
{
public:
    Scene() :
        _cellMorphologyData(),
        _cellMorphologies(),
        _cellMetadata()
    {

    }

    mv::Dataset<CellMorphologies> getCellMorphologies() { return _cellMorphologies; }

private:
    mv::Dataset<Points>             _cellMorphologyData;        /** Morphology feature data */
    mv::Dataset<CellMorphologies>   _cellMorphologies;          /** Morphology data */
    mv::Dataset<Text>               _cellMetadata;              /** Cell metadata */

    friend class CellMorphologyView;
};
