
#pragma once

#include "../../VirtualStackSettings.h"
#include "NetworkExtensions.h"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class RoutingTableHelper
{
public:
    static bool reRoute(sockaddr_storage& destination, const VirtualStackSettings& settings)
    {
        auto destIp = NetworkExtensions::getAddressIpv4(destination);
        auto& routeList = settings.RoutingTable.value;
        for (size_t i = 0; i < routeList.size(); i+=2)
        {
            if (!isIpV4InNetwork(routeList[i], destIp))
                continue;

            NetworkExtensions::SetIPv4Address(destination, routeList[i + 1]);
            //Logger::Log(Logger::DEBUG, "Rewrite Route from: ", destIp, ", to: ", routeList[i + 1]);
            return true;
        }
        return false;
    }

    static bool isIpV4InNetwork(const std::string& network, uint32_t ip, uint32_t subnet)
    {
        /*
         * subnetz 255.255.0.0
         * ip 127.5.1.1
         * netz 127.5.0.0
         *
         * subnetz 11111111_11111111_00000000_00000000
         * ip      01111111_00000101_00000001_00000001
         * netz    01111111_00000101_00000000_00000000
         *
         * (subnetz & ip) = (netz & subnetz)
         */





        auto networkIp = NetworkExtensions::convertIpv4NetworkOrder(network);
        if (!isIpV4NetworkIp(networkIp, subnet))
            return ip == networkIp;

        return (ip & subnet) == (networkIp & subnet); //is part of ip with subnet 255.255.255.0s
    }

    static bool isIpV4InNetwork(const std::string& network, uint32_t ip)
    {
        static auto subnet = NetworkExtensions::convertIpv4NetworkOrder("255.255.255.0");
        return isIpV4InNetwork(network, ip, subnet);
    }

    static bool isIpV4NetworkIp(uint32_t networkIp, uint32_t subnet)
    {
        return (networkIp & ~subnet) == 0;
    }
};