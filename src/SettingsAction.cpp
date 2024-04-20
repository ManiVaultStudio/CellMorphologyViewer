#include "SettingsAction.h"

#include "CellMorphologyView.h"

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<CellMorphologyView*>(parent)),
    _lineRendererButton(this, "Line Renderer"),
    _realRendererButton(this, "True Renderer")
{

}
