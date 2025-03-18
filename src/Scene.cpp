#include "Scene.h"

namespace
{
    bool isMorphologicalData(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "M";
    }

    bool isMorphologies(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "Morphologies";
    }

    bool isMetadata(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "Metadata";
    }
}

Scene::Scene() :
    _morphologyFeatureDataset(),
    _morphologyDataset(),
    _cellMetadataDataset()
{

}

bool Scene::hasAllRequiredDatasets()
{
    if (!_cellMetadataDataset.isValid())
    {
        qWarning() << "[CellMorphologyViewer] No cell metadata dataset set.";
        return false;
    }

    if (!_morphologyDataset.isValid())
    {
        qWarning() << "[CellMorphologyViewer] No cell morphology dataset found.";
        return false;
    }

    if (!_morphologyFeatureDataset.isValid())
    {
        qWarning() << "[CellMorphologyViewer] No cell morphology feature dataset found.";
        return false;
    }
}

void Scene::offerCandidateDataset(Dataset<DatasetImpl> candidateDataset)
{
    if (isMorphologicalData(candidateDataset))
        _morphologyFeatureDataset = candidateDataset;
    if (isMorphologies(candidateDataset))
        _morphologyDataset = candidateDataset;
    if (isMetadata(candidateDataset))
        _cellMetadataDataset = candidateDataset;

    qDebug() << _morphologyFeatureDataset.isValid();
    qDebug() << _morphologyDataset.isValid();
    qDebug() << _cellMetadataDataset.isValid();
}
