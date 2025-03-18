#pragma once

#include <PointData/PointData.h>
#include <TextData/TextData.h>
#include <CellMorphologyData/CellMorphologyData.h>

class Scene
{
public:
    Scene();

    bool hasAllRequiredDatasets();

    mv::Dataset<CellMorphologies>&   getMorphologyDataset()           { return _morphologyDataset; }
    mv::Dataset<Points>&             getMorphologyFeatureDataset()    { return _morphologyFeatureDataset; }
    mv::Dataset<Text>&               getCellMetadataDataset()         { return _cellMetadataDataset; }

    void offerCandidateDataset(mv::Dataset<mv::DatasetImpl> candidateDataset);

private:
    mv::Dataset<CellMorphologies>   _morphologyDataset;             /** Morphology data */
    mv::Dataset<Points>             _morphologyFeatureDataset;      /** Morphology feature data */
    mv::Dataset<Text>               _cellMetadataDataset;           /** Cell metadata */
};
