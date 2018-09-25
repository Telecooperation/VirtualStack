#pragma once


#include "common/Helper/ClassMacros.h"
#include "common/Helper/StopWatch.h"
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <vector>

struct LatencyResult
{
    size_t id;
    size_t packetSize;
    size_t connectionEstablishment;
    size_t pps = 0;
    int64_t avg = 0;
    int64_t jitter = 0;
    int64_t median = 0;
    int64_t percentile0 = 0;
    int64_t percentile10 = 0;
    int64_t percentile20 = 0;
    int64_t percentile40 = 0;
    int64_t percentile60 = 0;
    int64_t percentile80 = 0;
    int64_t percentile90 = 0;
    int64_t percentile99 = 0;
    int64_t percentile100 = 0;
    std::vector<int64_t> delayList{};
    std::vector<int64_t> ascSortedDelayList{};
    std::string inName;

    LatencyResult() = default;

    void print(std::ostream& stream) const;

    void printAsMicroSeconds(std::ostream& stream) const;

    void printValues(std::ostream& stream, const std::string& separator) const;

    void dumpLatencies(std::ostream& stream) const;

    void dumpBargraphMetadata(std::ostream& stream, const size_t percentilCount) const;

    static int64_t calcJitter(int64_t avgDelay, const std::vector<int64_t>& ascOrderedDelayList);

    static int64_t getNthPercentile(double nthPercentile, const std::vector<int64_t>& ascOrderedDelayList);

    static size_t getIndexForNthPertcentile(double nthPercentile, size_t size);

    static int64_t getMedian(const std::vector<int64_t> &ascOrderedDelayList);

    static std::unique_ptr<LatencyResult> analyse(size_t id, const std::string& name,
                                 const size_t connectionEstablishment,
                                 const size_t runtime,
                                 const size_t packetSize,
                                 std::vector<int64_t>&& delayList);

    static std::tuple<double, std::string> normalizeThroughput(size_t bytesPerSecond);

    ALLOW_MOVE_SEMANTICS_ONLY(LatencyResult);
};

class LatencyMeter
{
public:
    explicit LatencyMeter(size_t packetSize, size_t reserveSize = 200000000);

    static size_t getNewInTime(const std::string& inName, size_t reserveSize = 200000000);
    static void addInTime(size_t key);
    static void addInTime(size_t key, size_t nanoseconds);
    void addOutTime();
    void addOutTime(size_t nanoseconds);

    std::vector<std::unique_ptr<LatencyResult>> analyse(const size_t runtime);
    static void dump(const std::string& outName, const LatencyResult& latencyResult, bool fullDump = false);

    std::vector<std::chrono::steady_clock::time_point> _outTime;
    StopWatch connectionEstablishmentWatch;

    void clearInTime();

    ALLOW_MOVE_SEMANTICS_ONLY(LatencyMeter);
private:
    size_t _packetSize;

    static std::mutex _getNewInTimeMutex;

    size_t _dumpCounter;

    static std::map<size_t, std::pair<std::string, std::vector<std::chrono::steady_clock::time_point>>> _inTimeList;
};
