#pragma once

#include <cstdint>
#include <string>

struct ComplexNetworkConfig
{
    std::string inDeviceName = "left";
    std::string outDeviceName = "right";
    std::string tapPathDataRate = "1Gbps";
    size_t tapPathLatency = 0;
    double lteModifier = 0; //disable LTE slotting per default
    std::string lteDataRate = "25Mbps";
    size_t lteLatency = 50;
    std::string dslDataRate = "6Mbps";
    size_t dslLatency = 30;

    ComplexNetworkConfig(int size, char** array)
    {
        int i = 0;
        inDeviceName = array[i++];
        outDeviceName = array[i++];
        tapPathDataRate = array[i++];
        tapPathLatency = static_cast<size_t>(std::atoll(array[i++]));

        if(size > i)
            lteModifier = std::atof(array[i++]);
        if(size > i)
        {
            lteDataRate = array[i++];
            lteLatency = static_cast<size_t>(std::atoll(array[i++]));
            dslDataRate = array[i++];
            dslLatency = static_cast<size_t>(std::atoll(array[i++]));
        }
    }
};