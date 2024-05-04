#include "CellMorphologyView.h"

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

CellMorphologyView::CellMorphologyView(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _cellIdData(),
    _currentDatasetName(),
    _currentDatasetNameLabel(new QLabel("Cell IDs")),
    _morphologyWidget(new MorphologyWidget(this)),
    _inputAction(this, "Dataset ID", ""),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
    //_tTypeClassAction(this, "T-Type Class", "", ""),
    //_tTypeAction(this, "T-Type", "", "")
{
    // This line is mandatory if drag and drop behavior is required
    _currentDatasetNameLabel->setAcceptDrops(true);

    // Align text in the center
    _currentDatasetNameLabel->setAlignment(Qt::AlignCenter);

    //connect(&_inputAction, &StringAction::stringChanged, this, &CellMorphologyView::dataInputChanged);
    connect(_morphologyWidget, &MorphologyWidget::changeNeuron, this, &CellMorphologyView::onNeuronChanged);

    connect(&_cellIdData, &Dataset<Text>::changed, this, &CellMorphologyView::onCellIdDatasetChanged);
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

    // Instantiate new drop widget
    _dropWidget = new DropWidget(_currentDatasetNameLabel);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    // Initialize the drop regions
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        // A drop widget can contain zero or more drop regions
        DropWidget::DropRegions dropRegions;
        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->text();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ TextType, CellMorphologyType });

        // Visually indicate if the dataset is of the wrong data type and thus cannot be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        }
        else {
            
            // Get points dataset from the core
            auto candidateDataset = mv::data().getDataset<Text>(datasetId);

            // Accept points datasets drag and drop
            if (dataType == TextType) {
                const auto description = QString("Load %1 into example view").arg(datasetGuiName);

                if (!_cellIdData.isValid()) {
                    // Dataset can be dropped
                    dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _cellIdData = candidateDataset;
                        qDebug() << _cellIdData.isValid();
                    });
                }
                else {
                    if (_cellIdData == candidateDataset) {
                        // Dataset cannot be dropped because it is already loaded
                        dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                    }
                    else {
                        // Dataset can be dropped
                        dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                            _cellIdData = candidateDataset;
                        });
                    }
                }
            }
            else if (dataType == CellMorphologyType)
            {
                const auto description = QString("Load %1 into cell morphology view").arg(datasetGuiName);

                if (!_cellMorphologies.isValid()) {
                    // Dataset can be dropped
                    dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _cellMorphologies = candidateDataset;
                        qDebug() << _cellMorphologies.isValid();
                    });
                }
                else {
                    if (_cellMorphologies == candidateDataset) {
                        // Dataset cannot be dropped because it is already loaded
                        dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                    }
                    else {
                        // Dataset can be dropped
                        dropRegions << new DropWidget::DropRegion(this, "Points", description, "map-marker-alt", true, [this, candidateDataset]() {
                            _cellMorphologies = candidateDataset;
                        });
                    }
                }
            }
        }

        return dropRegions;
    });

    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    layout->addWidget(_currentDatasetNameLabel);
    //layout->addWidget(_tTypeClassAction.createWidget(&getWidget()), 1);
    //layout->addWidget(_tTypeAction.createWidget(&getWidget()), 1);
    layout->addWidget(_morphologyWidget, 99);
    layout->addWidget(_inputAction.createWidget(&getWidget()), 1);

    // Apply the layout
    getWidget().setLayout(layout);

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_cellIdData, &Dataset<Text>::guiNameChanged, this, [this]()
    {
        // Update the current dataset name label
        _currentDatasetNameLabel->setText(QString("Current points dataset: %1").arg(_cellIdData->getGuiName()));

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(_cellIdData->getGuiName().isEmpty());
    });

    // Alternatively, classes which derive from hdsp::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetRemoved));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(TextType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));

    //Query query;
    //_neuronList = query.send();
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
    _morphologyWidget->setCellMetadata(cellId, cellSubclasses[cellIndex]);
}

void CellMorphologyView::onCellIdDatasetChanged()
{
    connect(&_cellIdData, &Dataset<Text>::dataSelectionChanged, this, &CellMorphologyView::onCellSelectionChanged);
}

void CellMorphologyView::onCellSelectionChanged()
{
    if (!_cellIdData.isValid())
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
