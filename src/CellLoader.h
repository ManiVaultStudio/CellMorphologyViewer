#pragma once

#include <string>

class Neuron;

void loadCell(std::string dataInput, std::string& result);

void readCell(const std::string& fileResult, Neuron& neuron);
