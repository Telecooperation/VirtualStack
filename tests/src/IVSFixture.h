#include <gtest/gtest.h>
#include <common/Helper/Logger.h>
#include <common/Allocator/VsObjectFactory.h>
#include <common/Helper/ClassMacros.h>
#include <VirtualStackSettings.h>


class IVSFixture : public ::testing::Test
{
public:
protected:
    IVSFixture();

public:
    virtual ~IVSFixture() override;

    ALLOW_MOVE_SEMANTICS_ONLY(IVSFixture);
protected:
	virtual void SetUp() override;
	
	virtual void TearDown() override
	{
	}

    //custom member data
    const VirtualStackSettings _virtualStackSettings;
	std::unique_ptr<VsObjectFactory> _vsObjectFactory;
};
