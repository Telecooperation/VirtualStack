
#pragma once

#include "../Helper/ClassMacros.h"
#include <functional>

class IPool
{
public:
	IPool() {}
	virtual ~IPool();

    /**
     * Is memory of this pool beeing used right now
     * @return True if beeing used
     */
    virtual bool isInUse() const = 0;

    /// Function to be called after all objects of the pool are freed and non is in use
    /// \param callback The callback to be called
    virtual void setOnLastReleaseCallback(const std::function<void()>& callback) = 0;

    /// Check if pool as a releaseCallback
    virtual bool hasReleaseCallback() = 0;
	ALLOW_MOVE_SEMANTICS_ONLY(IPool);
};


