#pragma once

#include "MorphologyWidget.h"

#include "NeuronDescriptor.h"

#include <ViewPlugin.h>

#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <actions/StringAction.h>

#include <PointData/PointData.h>

#include <QWidget>

/** All plugin related classes are in the HDPS plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the HDPS gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the HDPS util namespace */
using namespace mv::util;

class QLabel;

/**
 * Cell morphology plugin class
 *
 * @authors J. Thijssen
 */
class CellMorphologyView : public ViewPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    CellMorphologyView(const PluginFactory* factory);

    /** Destructor */
    ~CellMorphologyView() override = default;
    
    /** This function is called by the core after the view plugin has been created */
    void init() override;

    /**
     * Invoked when a data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(mv::DatasetEvent* dataEvent);
    void onNeuronChanged(QString neuronId);

    void dataInputChanged(const QString& dataInput);

private:
    /** Invoked when the position points dataset changes */
    void onPointsDatasetChanged();

    void onCellSelectionChanged();

private:
    DropWidget*                     _dropWidget;                /** Widget for drag and drop behavior */
    mv::Dataset<Points>             _points;                    /** Points smart pointer */
    QString                         _currentDatasetName;        /** Name of the current dataset */
    QLabel*                         _currentDatasetNameLabel;   /** Label that show the current dataset name */

    MorphologyWidget*         _morphologyWidget;
    StringAction              _inputAction;
    //StringAction            _tTypeClassAction;
    //StringAction            _tTypeAction;

    NeuronDescriptor        _currentNeuron;

    std::vector<NeuronDescriptor> _neuronList;
};

/**
 * Example view plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
class CellMorphologyPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.CellMorphologyView"
                      FILE  "CellMorphologyView.json")

public:

    /** Default constructor */
    CellMorphologyPluginFactory() {}

    /** Destructor */
    ~CellMorphologyPluginFactory() override {}
    
    /** Creates an instance of the example view plugin */
    ViewPlugin* produce() override;

    /** Returns the data types that are supported by the example view plugin */
    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
