#pragma once

#include <actions/GroupAction.h>
#include <actions/TriggerAction.h>

class CellMorphologyView;

class SettingsAction : public mv::gui::GroupAction
{
public:
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

public: // Action getters
    mv::gui::TriggerAction& getLineRendererButton() { return _lineRendererButton; }
    mv::gui::TriggerAction& getRealRendererButton() { return _realRendererButton; }

private:
    CellMorphologyView*     _plugin;

    mv::gui::TriggerAction  _lineRendererButton;
    mv::gui::TriggerAction  _realRendererButton;
};
