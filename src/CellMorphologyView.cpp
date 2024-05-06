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
    _cellMetadata(),
    _currentDatasetName(),
    _morphologyWidget(new MorphologyWidget(this)),
    _inputAction(this, "Dataset ID", ""),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
    //_tTypeClassAction(this, "T-Type Class", "", ""),
    //_tTypeAction(this, "T-Type", "", "")
{
    //connect(&_inputAction, &StringAction::stringChanged, this, &CellMorphologyView::dataInputChanged);
    connect(_morphologyWidget, &MorphologyWidget::changeNeuron, this, &CellMorphologyView::onNeuronChanged);

    connect(&_cellMetadata, &Dataset<Text>::changed, this, &CellMorphologyView::onCellIdDatasetChanged);
}

void CellMorphologyView::init()
{
    // Create layout
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);

    //_primaryToolbarAction.addAction(&_settingsAction.getRenderModeAction(), 4, GroupAction::Horizontal);
    //_primaryToolbarAction.addAction(&_settingsAction.getPlotAction(), 7, GroupAction::Horizontal);
    //_primaryToolbarAction.addAction(&_settingsAction.getPositionAction(), 10, GroupAction::Horizontal);
    //_primaryToolbarAction.addAction(&_settingsAction.getFilterAction(), 0, GroupAction::Horizontal);
    //_primaryToolbarAction.addAction(&_settingsAction.getOverlayAction(), 0, GroupAction::Horizontal);
    //_primaryToolbarAction.addAction(&_settingsAction.getExportAction(), 0, GroupAction::Horizontal);
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
    connect(&_cellMetadata, &Dataset<Text>::guiNameChanged, this, [this]()
    {
        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(_cellMetadata->getGuiName().isEmpty());
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
            _cellMorphologyData = dataset;
        if (isMorphologies(dataset))
            _cellMorphologies = dataset;
        if (isMetadata(dataset))
            _cellMetadata = dataset;
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
                _cellMorphologyData = changedDataSet;
            if (isMorphologies(changedDataSet))
                _cellMorphologies = changedDataSet;
            if (isMetadata(changedDataSet))
                _cellMetadata = changedDataSet;

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
            qDebug() << datasetGuiName << "data changed";

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
            qDebug() << datasetGuiName << "selection has changed";

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
    if (!_cellMetadata.isValid())
    {
        qWarning() << "No cell metadata dataset set.";
        return;
    }

    if (!_cellMorphologies.isValid())
    {
        qWarning() << "No cell morphology dataset set.";
        return;
    }

    auto selectionDataset = _cellMetadata->getSelection();

    const std::vector<uint32_t>& metaIndices = selectionDataset->getSelectionIndices();

    if (metaIndices.empty())
        return;

    std::vector<QString> cellIds = _cellMetadata->getColumn("Cell ID");
    std::vector<QString> cellSubclasses = _cellMetadata->getColumn("Subclass");

    // Find first cell with morphology
    QString foundCellId;
    uint32_t foundCellIndex = -1;
    for (uint32_t metaIndex : metaIndices)
    {
        QString cellId = cellIds[metaIndex];

        // Get index of cell identifier
        const QStringList& cellIdsWithMorphologies = _cellMorphologies->getCellIdentifiers();

        int ci = cellIdsWithMorphologies.indexOf(cellId);

        if (ci != -1)
        {
            // Found a cell with morphology
            foundCellId = cellId;
            foundCellIndex = ci;
            break;
        }
    }

    if (foundCellId.isEmpty())
    {
        qWarning() << "No cells were selected that have an associated morphology loaded.";
        return;
    }

    uint32_t cellIndex = metaIndices[0];

    qDebug() << "Found cell with ID: " << foundCellId;

    // Get cell morphology at index
    const std::vector<CellMorphology>& cellMorphologies = _cellMorphologies->getData();

    const CellMorphology& cellMorphology = cellMorphologies[foundCellIndex];

    _morphologyWidget->setCellMorphology(cellMorphology);
    _morphologyWidget->setCellMetadata(foundCellId, cellSubclasses[cellIndex]);

    // Check if any cell morphology data is loaded
    if (!_cellMorphologyData.isValid())
    {
        qWarning() << "No cell morphology data dataset set.";
        return;
    }

    // Provide additional cell morphological feature
    const std::vector<uint32_t>& cmdIndices = _cellMorphologyData->getSelectionIndices();

    if (cmdIndices.empty())
        return;

    uint32_t cmdIndex = cmdIndices[0];

    size_t index = cmdIndex * _cellMorphologyData->getNumDimensions();

    _cellMorphologyData->getDimensionNames();

    std::vector<float> values(_cellMorphologyData->getNumDimensions());
    for (int i = 0; i < values.size(); i++)
    {
        values[i] = _cellMorphologyData->getValueAt(index + i);
    }

    qDebug() << "apical_dendrite_bias_x" << _cellMorphologyData->getValueAt(index);

    MorphologyDescription morphDescription;
    morphDescription.setData(_cellMorphologyData->getDimensionNames(), values);

    _morphologyWidget->setCellMorphologyData(morphDescription);
    qDebug() << morphDescription.getApicalDendriteDescription().bias.x;
}

void CellMorphologyView::onCellIdDatasetChanged()
{
    connect(&_cellMetadata, &Dataset<Text>::dataSelectionChanged, this, &CellMorphologyView::onCellSelectionChanged);
}

void CellMorphologyView::onCellSelectionChanged()
{
    if (!_cellMetadata.isValid())
        return;

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
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<CellMorphologyPluginFactory*>(this), this, "Example", "View example data", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
