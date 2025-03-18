#include "CellMorphologyView.h"

#include "MorphologyDescription.h"

#include <event/Event.h>

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
    _scene(),
    _morphologyWidget(new MorphologyWidget(this, &_scene)),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
{
    // Notice when the cell morphologies dataset changes, so that we can connect to its selection changes
    connect(&_scene.getMorphologyDataset(), &Dataset<CellMorphologies>::changed, this, [this]() {
        if (_scene.getMorphologyDataset().isValid())
            connect(&_scene.getMorphologyDataset(), &Dataset<CellMorphologies>::dataSelectionChanged, this, &CellMorphologyView::onCellSelectionChanged);
    });
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
    layout->addWidget(_morphologyWidget);

    // Apply the layout
    getWidget().setLayout(layout);

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_scene.getMorphologyDataset(), &Dataset<Text>::guiNameChanged, this, [this]()
    {
        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(_scene.getMorphologyDataset()->getGuiName().isEmpty());
    });

    // Alternatively, classes which derive from hdsp::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.registerDataEventByType(PointType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(TextType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(CellMorphologyType, std::bind(&CellMorphologyView::onDataEvent, this, std::placeholders::_1));

    // Check if any usable datasets are already available, if so, use them
    for (mv::Dataset dataset : mv::data().getAllDatasets())
        _scene.offerCandidateDataset(dataset);
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

            _scene.offerCandidateDataset(changedDataSet);

            // Get the GUI name of the added points dataset and print to the console
            qDebug() << datasetGuiName << "was added";

            break;
        }

        default:
            break;
    }
}

void CellMorphologyView::onCellSelectionChanged()
{
    qDebug() << "onCellSelectionChanged()";

    if (!_scene.hasAllRequiredDatasets())
        return;

    const auto& selectionIndices = _scene.getMorphologyDataset()->getSelectionIndices();
    qDebug() << "Selection indices morph: " << selectionIndices.size();
    qDebug() << "Selection indices morph2: " << _scene.getMorphologyDataset()->getSelection()->getSelectionIndices().size();

    float totalWidth = 0;
    for (CellMorphology& cellMorphology : _scene.getMorphologyDataset()->getData())
    {
        Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;

        float maxWidth = sqrtf(range.x * range.x + range.z * range.z);

        totalWidth += maxWidth;
    }
    float averageWidth = totalWidth / _scene.getMorphologyDataset()->getData().size();

    _morphologyWidget->setRowWidth(averageWidth * 8);

    CellMorphology cellMorphology;
    _morphologyWidget->setCellMorphology(cellMorphology);
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
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<CellMorphologyPluginFactory*>(this), this, "Cell Morphology", "View cell morphologies", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
