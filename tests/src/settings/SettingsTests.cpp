
#include <common/DataStructures/VS/SettingsParser.h>
#include <common/DataStructures/VS/SettingsProvider.h>
#include <common/Helper/SettingsStreamFactory.h>
#include <common/DataStructures/VS/SettingsEntry.h>
#include <stacks/StackEnum.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/DataStructures/VS/SettingsEntryFactory.h>
#include <common/DataStructures/VS/SettingsMacros.h>
#include <cstdio>
#include <cstdlib>
#include <stacks/StackEnumHelper.h>

TEST(SettingsTests, SettingsTests_StringVector_Test)
{
	std::string testString = "     hallo,welt,,, , wie          ,geht       ,es           ,dir   ";
	std::vector<std::string> expected{"hallo","welt","wie","geht","es","dir"};
	
	auto tmpResult = SettingsParser::splitString(testString, ',');
	
	EXPECT_THAT(tmpResult, ::testing::ContainerEq(expected));
}

TEST(SettingsTests, SettingsTests_IntVector_Test_Invalid)
{
    Logger::setMinLogLevel(Logger::NONE);
	std::string testString = "     hallo,welt,,, , wie          ,geht       ,es           ,dir   ";
	
	auto tmpStream = SettingsStreamFactory::New().AddString("key", testString).Build();
	
	SettingsProvider provider;
	bool tmpSettingsRead = provider.ReadSettings(*tmpStream);
	
	bool tmpKeyValueInValid = false;
	auto tmpResult = provider.get<std::vector<int>>("key", tmpKeyValueInValid);
	
	
	ASSERT_TRUE(tmpSettingsRead);
	ASSERT_TRUE(tmpKeyValueInValid);
	
	ASSERT_TRUE(tmpResult.empty());
}

TEST(SettingsTests, SettingsTests_SizeTVector_Test)
{
	std::string testString = "     2ul,1ul,,, , 22ul          ,3434ul       ,214234ul           ,121ul   ";
	std::vector<size_t> expected{2,1,22,3434,214234,121};
	
	auto tmpStream = SettingsStreamFactory::New().AddString("key", testString).Build();
	SettingsProvider provider;
	bool tmpSettingsRead = provider.ReadSettings(*tmpStream);
	
	bool tmpKeyValueInValid = false;
	auto tmpResult = provider.get<std::vector<size_t>>("key", tmpKeyValueInValid);
	
	ASSERT_TRUE(tmpSettingsRead);
	ASSERT_FALSE(tmpKeyValueInValid);
	
	EXPECT_THAT(tmpResult, ::testing::ContainerEq(expected));
}

TEST(SettingsTests, SettingsTests_IntVector_Test)
{
	std::string testString = "     2,1,,, , 22          ,3434       ,214234           ,121   ";
	std::vector<int> expected{2,1,22,3434,214234,121};
	
	auto tmpStream = SettingsStreamFactory::New().AddString("key", testString).Build();
	SettingsProvider provider;
	bool tmpSettingsRead = provider.ReadSettings(*tmpStream);
	
	bool tmpKeyValueInValid = false;
	auto tmpResult = provider.get<std::vector<int>>("key", tmpKeyValueInValid);
	
	ASSERT_TRUE(tmpSettingsRead);
	ASSERT_FALSE(tmpKeyValueInValid);
	
	EXPECT_THAT(tmpResult, ::testing::ContainerEq(expected));
}

TEST(SettingsTests, SettingsTests_Generic_Key)
{
	auto tmpStream = SettingsStreamFactory::New().AddString("defaultstack", "tcpipv4").Build();
	SettingsProvider provider;
	bool tmpSettingsRead = provider.ReadSettings(*tmpStream);
	
	bool tmpKeyValueInValid = false;
	auto tmpResult = SettingsEntryFactory<StackEnumHelper>::byConvert<std::string>("defaultstack", provider, tmpKeyValueInValid);
	
	ASSERT_TRUE(tmpSettingsRead);
	ASSERT_FALSE(tmpKeyValueInValid);
	ASSERT_EQ(tmpResult.value, StackEnum::TCPIPv4);
}

TEST(SettingsTests, SettingsTests_Generic_Vector_Key)
{
	auto tmpStream = SettingsStreamFactory::New().AddStringVector("Stacks", {"tcpipv4", "udpipv4", "sctpipv4"}).Build();
	SettingsProvider _provider;
	bool tmpSettingsRead = _provider.ReadSettings(*tmpStream);

	bool _settingsReadFailed = false;
	GENERIC_VECTOR_KEY_EXPLICIT(Stacks, StackEnum, std::string, &StackEnumHelper::convert);

	ASSERT_TRUE(tmpSettingsRead);
	ASSERT_FALSE(_settingsReadFailed);

	std::vector<StackEnum> expected{StackEnum::TCPIPv4, StackEnum::UDPIPv4, StackEnum::SCTPIPv4};
	EXPECT_THAT(Stacks.value, ::testing::ContainerEq(expected));
}
