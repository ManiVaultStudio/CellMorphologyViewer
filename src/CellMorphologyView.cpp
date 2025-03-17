#include "CellMorphologyView.h"

#include "MorphologyDescription.h"

#include <event/Event.h>

//#include "Query.h"

#include "CellMorphologyData/CellMorphology.h"

#include <DatasetsMimeData.h>

#include <QDebug>
#include <QMimeData>
#include <fstream>
#include <sstream>

Q_PLUGIN_METADATA(IID "studio.manivault.CellMorphologyView")

using namespace mv;

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

CellMorphologyView::CellMorphologyView(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _scene(),
    _morphologyWidget(new MorphologyWidget(this, &_scene)),
    _inputAction(this, "Dataset ID", ""),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
    //_tTypeClassAction(this, "T-Type Class", "", ""),
    //_tTypeAction(this, "T-Type", "", "")
{
    //connect(&_inputAction, &StringAction::stringChanged, this, &CellMorphologyView::dataInputChanged);
    connect(_morphologyWidget, &MorphologyWidget::changeNeuron, this, &CellMorphologyView::onNeuronChanged);

    connect(&_scene._cellMetadata, &Dataset<Text>::changed, this, &CellMorphologyView::onCellIdDatasetChanged);
}

void CellMorphologyView::init()
{
    // Create layout
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);

    _primaryToolbarAction.addAction(&_settingsAction.getLineRendererButton());
    _primaryToolbarAction.addAction(&_settingsAction.getRealRendererButton());

    connect(&_settingsAction.getLineRendererButton(), &TriggerAction::triggered, this, [this]() { _morphologyWidget->setRenderMode(RenderMode::LINE); });
    connect(&_settingsAction.getRealRendererButton(), &TriggerAction::triggered, this, [this]() { _morphologyWidget->setRenderMode(RenderMode::REAL); });

    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    //layout->addWidget(_tTypeClassAction.createWidget(&getWidget()), 1);
    //layout->addWidget(_tTypeAction.createWidget(&getWidget()), 1);
    layout->addWidget(_morphologyWidget, 99);
    layout->addWidget(_inputAction.createWidget(&getWidget()), 1);

    // Apply the layout
    getWidget().setLayout(layout);

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_scene._cellMetadata, &Dataset<Text>::guiNameChanged, this, [this]()
    {
        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(_scene._cellMetadata->getGuiName().isEmpty());
    });

    // Alternatively, classes which derive from hdsp::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetRemoved));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(TextType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(CellMorphologyType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));

    //Query query;
    //_neuronList = query.send();

    // Check if any usable datasets are already available, if so, use them
    for (mv::Dataset dataset : mv::data().getAllDatasets())
    {
        if (isMorphologicalData(dataset))
            _scene._cellMorphologyData = dataset;
        if (isMorphologies(dataset))
            _scene._cellMorphologies = dataset;
        if (isMetadata(dataset))
            _scene._cellMetadata = dataset;
    }
}

void CellMorphologyView::onDataEvent(mv::DatasetEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto changedDataSet = dataEvent->getDataset();

    // Get GUI name of the dataset that changed
    const auto datasetGuiName = changedDataSet->getGuiName();

    // The data event has a type so that we know what type of data event occurred (e.g. data added, changed, removed, renamed, selection changes)
    switch (dataEvent->getType()) {

        // A points dataset was added
        case EventType::DatasetAdded:
        {
            // Cast the data event to a data added event
            const auto dataAddedEvent = static_cast<DatasetAddedEvent*>(dataEvent);

            if (isMorphologicalData(changedDataSet))
                _scene._cellMorphologyData = changedDataSet;
            if (isMorphologies(changedDataSet))
                _scene._cellMorphologies = changedDataSet;
            if (isMetadata(changedDataSet))
                _scene._cellMetadata = changedDataSet;
            qDebug() << _scene._cellMorphologyData.isValid();
            qDebug() << _scene._cellMorphologies.isValid();
            qDebug() << _scene._cellMetadata.isValid();
            // Get the GUI name of the added points dataset and print to the console
            qDebug() << datasetGuiName << "was added";

            break;
        }

        // Points dataset data has changed
        case EventType::DatasetDataChanged:
        {
            // Cast the data event to a data changed event
            const auto dataChangedEvent = static_cast<DatasetDataChangedEvent*>(dataEvent);

            // Get the name of the points dataset of which the data changed and print to the console
            //qDebug() << datasetGuiName << "data changed";

            break;
        }

        // Points dataset data was removed
        case EventType::DatasetRemoved:
        {
            // Cast the data event to a data removed event
            const auto dataRemovedEvent = static_cast<DatasetRemovedEvent*>(dataEvent);

            // Get the name of the removed points dataset and print to the console
            qDebug() << datasetGuiName << "was removed";

            break;
        }

        // Points dataset selection has changed
        case EventType::DatasetDataSelectionChanged:
        {
            // Cast the data event to a data selection changed event
            const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);

            // Get the selection set that changed
            const auto& selectionSet = changedDataSet->getSelection<Text>();

            // Print to the console
            //qDebug() << datasetGuiName << "selection has changed";

            break;
        }

        default:
            break;
    }
}

void CellMorphologyView::dataInputChanged(const QString& dataInput)
{

}

void CellMorphologyView::onNeuronChanged()
{
    if (!_scene._cellMetadata.isValid())
    {
        qWarning() << "No cell metadata dataset set.";
        return;
    }

    if (!_scene._cellMorphologies.isValid())
    {
        qWarning() << "No cell morphology dataset found by CellMorphologyViewer.";
        return;
    }

    const auto& selectionIndices = _scene._cellMorphologies->getSelectionIndices();
    qDebug() << "Selection indices morph: " << selectionIndices.size();
    qDebug() << "Selection indices morph2: " << _scene._cellMorphologies->getSelection()->getSelectionIndices().size();

    float totalWidth = 0;
    for (CellMorphology& cellMorphology : _scene._cellMorphologies->getData())
    {
        Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;

        float maxWidth = sqrtf(range.x * range.x + range.z * range.z);

        totalWidth += maxWidth;
    }
    float averageWidth = totalWidth / _scene._cellMorphologies->getData().size();

    _morphologyWidget->setRowWidth(averageWidth * 8);

    CellMorphology cellMorphology;
    _morphologyWidget->setCellMorphology(cellMorphology);

    return;

    //const QStringList& cellIdsWithMorphologies = _scene._cellMorphologies->getCellIdentifiers();

    //const auto& selectionIndices = _scene._cellMorphologies->getSelectionIndices();

    //for (int i = 0; i < selectionIndices.size(); i++)
    //{
    //    _morphologyWidget->set
    //}



    //////////////
    //auto selectionDataset = _scene._cellMetadata->getSelection();

    //const std::vector<uint32_t>& metaIndices = selectionDataset->getSelectionIndices();

    //if (metaIndices.empty())
    //    return;

    //std::vector<QString> cellIds = _scene._cellMetadata->getColumn("Cell ID");
    //std::vector<QString> cellSubclasses = _scene._cellMetadata->getColumn("Subclass");

    //// Find first cell with morphology
    //QString foundCellId;
    //uint32_t foundCellIndex = -1;
    //for (uint32_t metaIndex : metaIndices)
    //{
    //    QString cellId = cellIds[metaIndex];

    //    // Get index of cell identifier
    //    const QStringList& cellIdsWithMorphologies = _scene._cellMorphologies->getCellIdentifiers();

    //    int ci = cellIdsWithMorphologies.indexOf(cellId);

    //    if (ci != -1)
    //    {
    //        // Found a cell with morphology
    //        foundCellId = cellId;
    //        foundCellIndex = ci;
    //        break;
    //    }
    //}

    //if (foundCellId.isEmpty())
    //{
    //    qWarning() << "No cells were selected that have an associated morphology loaded.";
    //    return;
    //}

    //uint32_t cellIndex = metaIndices[0];

    //qDebug() << "Found cell with ID: " << foundCellId;

    //// Get cell morphology at index
    //const std::vector<CellMorphology>& cellMorphologies = _scene._cellMorphologies->getData();

    //const CellMorphology& cellMorphology = cellMorphologies[foundCellIndex];

    //_morphologyWidget->setCellMorphology(cellMorphology);
    //_morphologyWidget->setCellMetadata(foundCellId, cellSubclasses[cellIndex]);

    //// Check if any cell morphology data is loaded
    //if (!_scene._cellMorphologyData.isValid())
    //{
    //    qWarning() << "No cell morphology data dataset set.";
    //    return;
    //}

    //// Provide additional cell morphological feature
    //const std::vector<uint32_t>& cmdIndices = _scene._cellMorphologyData->getSelectionIndices();

    //if (cmdIndices.empty())
    //    return;

    //uint32_t cmdIndex = cmdIndices[0];

    //size_t index = cmdIndex * _scene._cellMorphologyData->getNumDimensions();

    //_scene._cellMorphologyData->getDimensionNames();

    //std::vector<float> values(_scene._cellMorphologyData->getNumDimensions());
    //for (int i = 0; i < values.size(); i++)
    //{
    //    values[i] = _scene._cellMorphologyData->getValueAt(index + i);
    //}

    //qDebug() << "apical_dendrite_bias_x" << _scene._cellMorphologyData->getValueAt(index);

    //MorphologyDescription morphDescription;
    //morphDescription.setData(_scene._cellMorphologyData->getDimensionNames(), values);

    //_morphologyWidget->setCellMorphologyData(morphDescription);
    //qDebug() << morphDescription.getApicalDendriteDescription().bias.x;
}

void CellMorphologyView::onCellIdDatasetChanged()
{
    connect(&_scene._cellMetadata, &Dataset<Text>::dataSelectionChanged, this, &CellMorphologyView::onCellSelectionChanged);
}

void CellMorphologyView::onCellSelectionChanged()
{
    qDebug() << "onCellSelectionChanged()";
    if (!_scene._cellMetadata.isValid())
        return;
    qDebug() << "Metadata selection size: " << _scene._cellMetadata->getSelection()->getSelectionIndices().size();
    onNeuronChanged();
}

ViewPlugin* CellMorphologyPluginFactory::produce()
{
    return new CellMorphologyView(this);
}

mv::DataTypes CellMorphologyPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This example analysis plugin is compatible with points datasets
    supportedTypes.append(TextType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions CellMorphologyPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this]() -> CellMorphologyView* {
        return dynamic_cast<CellMorphologyView*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets >= 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, TextType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<CellMorphologyPluginFactory*>(this), this, "Example", "View example data", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
