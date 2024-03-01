#include "CellLoader.h"

#include "Neuron.h"

#define NOMINMAX

#include "curl/curl.h"

#include <sstream>
#include <iostream>

#include <vector>
#include <limits>

size_t CurlWrite_CallbackFunc_StdString(void* contents, size_t size, size_t nmemb, std::string* s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

void loadCell(std::string dataInput, std::string& result)
{
    //curl_global_init(CURL_GLOBAL_SSL);

    //curl_global_cleanup();

    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    std::string url = "https://download.brainimagelibrary.org/biccn/zeng/pseq/morph/200526/" + dataInput + "_transformed.swc";

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    //std::cout << "Beep" << result.c_str() << std::endl;
}

void readCell(const std::string& fileResult, Neuron& neuron)
{
    std::istringstream fileStream(fileResult);
    std::string line;

    // #n type x y z radius parent
    while (getline(fileStream, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string value;

        if (line.at(0) == '#')
            continue;

        int i = 0;
        while (getline(ss, value, ' ')) {
            //row.push_back(value);
            switch (i)
            {
            case 0: neuron.ids.push_back(stoi(value)); break;
            case 1: neuron.types.push_back(stoi(value)); break;
            case 2:
            {
                float x = stof(value);
                getline(ss, value, ' ');
                float y = stof(value);
                getline(ss, value, ' ');
                float z = stof(value);
                neuron.positions.emplace_back(x, y, z);
                break;
            }
            case 3: neuron.radii.push_back(stof(value)); break;
            case 4: neuron.parents.push_back(stoi(value)); break;
            }
            i++;
        }
    }

    // Compute id to index map
    for (int i = 0; i < neuron.ids.size(); i++)
    {
        int id = neuron.ids[i];
        neuron.idMap[id] = i;
    }

    // Print headers
    //for (const auto& header : headers) {
    //    std::cout << header << "\t";
    //}

    // Print data
    //for (const auto& position : _positions) {
    //    std::cout << position.str() << std::endl;
    //}

    // Find centroid and extents
    mv::Vector3f avgPos;
    for (const auto& pos : neuron.positions)
        avgPos += pos;
    avgPos /= neuron.positions.size();

    // Center cell positions
    for (auto& pos : neuron.positions)
        pos -= avgPos;

    // Find cell position ranges
    mv::Vector3f minV(std::numeric_limits<float>::max());
    mv::Vector3f maxV(-std::numeric_limits<float>::max());
    for (const auto& pos : neuron.positions)
    {
        if (pos.x < minV.x) minV.x = pos.x;
        if (pos.y < minV.y) minV.y = pos.y;
        if (pos.z < minV.z) minV.z = pos.z;
        if (pos.x > maxV.x) maxV.x = pos.x;
        if (pos.y > maxV.y) maxV.y = pos.y;
        if (pos.z > maxV.z) maxV.z = pos.z;
    }
    mv::Vector3f range = (maxV - minV);
    float maxRange = std::max(std::max(range.x, range.y), range.z);
    // Rescale positions
    for (auto& pos : neuron.positions)
    {
        pos /= maxRange;
    }
}
