#include <DefaultVirtualStackSettings.h>
#include <common/DataStructures/Model/Storage.h>
#include <common/Helper/RoutingTableHelper.h>
#include <gtest/gtest.h>
#include <linux/udp.h>
#include <netinet/ip6.h>

TEST(GoogleTest, FrameWork_Works) {
    ASSERT_FALSE(false);
}

TEST(IPHeaderTests, SizeOfIPHeaders)
{
    Storage packet{0x60, 0x00, 0x00, 0x00, 0x00, 0x40, 0x3a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    ASSERT_EQ(58, reinterpret_cast<ip6_hdr *>(packet.data())->ip6_ctlun.ip6_un1.ip6_un1_nxt);
    ASSERT_EQ(8, sizeof(udphdr));
    ASSERT_EQ(40, sizeof(ip6_hdr));
}

TEST(FlowId, MapTest)
{
    flowid_t entry{};
    entry.destination = 123123;
    entry.transportProtocol = TransportProtocolEnum::UDP;

    flowid_t copy = entry;

    std::map<flowid_t, size_t> flowMap{};
    flowMap.emplace(entry, 100);

    auto res = flowMap.find(copy);
    ASSERT_NE(res, flowMap.end());
    ASSERT_EQ(res->first.transportProtocol, TransportProtocolEnum::UDP);
    ASSERT_EQ(res->first.destination, 123123);
    ASSERT_EQ(res->second, 100);
}

TEST(ReRouteTests, RouteTest)
{
    auto baselineIp = NetworkExtensions::convertIpv4NetworkOrder("127.0.0.1");

    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.0.1", baselineIp));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.0.0", baselineIp));
    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.1.0", baselineIp));

    baselineIp = NetworkExtensions::convertIpv4NetworkOrder("127.0.1.1");

    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.0.1", baselineIp));
    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.0.0", baselineIp));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.1.0", baselineIp));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.1.1", baselineIp));
    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.1.2", baselineIp));
}

TEST(ReRouteTests, RouteTestSubnet)
{
    auto subnet = NetworkExtensions::convertIpv4NetworkOrder("255.255.0.0");
    auto ip = NetworkExtensions::convertIpv4NetworkOrder("127.0.0.1");

    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.0.1", ip, subnet));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.0.0", ip, subnet));
    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.1.1", ip, subnet));

    ip = NetworkExtensions::convertIpv4NetworkOrder("127.0.1.1");

    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.0.1", ip, subnet));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.0.0", ip, subnet));
    ASSERT_TRUE(RoutingTableHelper::isIpV4InNetwork("127.0.1.1", ip, subnet));
    ASSERT_FALSE(RoutingTableHelper::isIpV4InNetwork("127.0.1.2", ip, subnet));
}