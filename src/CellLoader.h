#pragma once

#include <QString>

#include <string>

class Neuron;

void loadCellContentsFromWeb(std::string cellId, std::string& result);
void loadCellContentsFromFile(QString filePath, std::string& result);

void readCell(const std::string& contents, Neuron& neuron);
