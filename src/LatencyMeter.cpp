#include "LatencyMeter.h"
#include "common/Helper/make_unique.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>


std::map<size_t, std::pair<std::string, std::vector<std::chrono::steady_clock::time_point>>> LatencyMeter::_inTimeList{};
std::mutex LatencyMeter::_getNewInTimeMutex{};

LatencyMeter::LatencyMeter(size_t packetSize, size_t reserveSize) : _outTime(),
                                                                    _packetSize(packetSize),
                                                                    _dumpCounter(0)
{
    _outTime.reserve(reserveSize);
}

size_t LatencyMeter::getNewInTime(const std::string& inName, size_t reserveSize)
{
    std::lock_guard<std::mutex> lock(_getNewInTimeMutex);

    static size_t inTimeKey{0};
    auto inKey = inTimeKey++;

    _inTimeList.emplace(inKey, std::make_pair(inName, std::vector<std::chrono::steady_clock::time_point>()));
    _inTimeList[inKey].second.reserve(reserveSize);

    return inKey;
}

void LatencyMeter::addInTime(size_t key)
{
    _inTimeList[key].second.push_back(std::chrono::steady_clock::now());
}

void LatencyMeter::addInTime(size_t key, size_t nanoseconds)
{
    _inTimeList[key].second.push_back(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(nanoseconds)));
}

void LatencyMeter::addOutTime()
{
    _outTime.push_back(std::chrono::steady_clock::now());
}

void LatencyMeter::addOutTime(size_t nanoseconds)
{
    _outTime.push_back(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(nanoseconds)));
}

std::vector<std::unique_ptr<LatencyResult>> LatencyMeter::analyse(const size_t runtime)
{
    if(_outTime.empty())
        return {};

    std::vector<std::unique_ptr<LatencyResult>> latencyResultList{};
    const size_t connectionEstablishment = static_cast<const size_t>(connectionEstablishmentWatch.getDifferenceInNanoSeconds());
    for(const auto& in: _inTimeList)
    {
        if(in.second.second.empty())
            continue;
        if(_outTime.size() > in.second.second.size())
            continue;

        std::vector<int64_t> delayList;
        delayList.reserve(_outTime.size());

        for (size_t i = 0; i < _outTime.size(); ++i)
        {
            auto delay = std::chrono::duration_cast<std::chrono::nanoseconds>(_outTime[i] - in.second.second[i]);
            delayList.push_back(delay.count());
        }

        latencyResultList.push_back(LatencyResult::analyse(_dumpCounter,
                                                           in.second.first,
                                                           connectionEstablishment,
                                                           runtime,
                                                           _packetSize,
                                                           std::move(delayList)));

        ++_dumpCounter;
    }

    return latencyResultList;
}

void LatencyMeter::dump(const std::string& outName, const LatencyResult& latencyResult, bool fullDump)
{
    if (latencyResult.delayList.empty())
        return;

    std::ofstream file{"latencyMeter-" + latencyResult.inName + "-" + outName + "-" + std::to_string(latencyResult.id) + ".txt"};

    latencyResult.print(file);

    if(fullDump)
    {
        for (size_t i = 0; i < latencyResult.delayList.size(); ++i)
        {
            file << latencyResult.delayList[i] << std::endl;
        }
    }

    file.close();
}

void LatencyMeter::clearInTime()
{
    for (auto& val : _inTimeList)
    {
        val.second.second.clear();
    }
}

//####### LatencyResult

void LatencyResult::print(std::ostream &stream) const
{
    double throughput;
    std::string throughputUnit;
    std::tie(throughput, throughputUnit) = normalizeThroughput(pps * packetSize);

    stream << "Id: " << id << std::endl;
    stream << "InName: " << inName << std::endl;
    stream << "ConEstab: " << connectionEstablishment << " ns" << std::endl;
    stream << "PacketSize: " << packetSize << std::endl;
    stream << "PacketCount: " << delayList.size() << std::endl;
    stream << "Pps: " << pps << std::endl;
    stream << "Throughput: " << std::fixed << std::setprecision(2) << throughput << " " << throughputUnit << std::endl;
    stream << "AVG: " << avg << " ns" << std::endl;
    stream << "Jitter: " << jitter << " ns" << std::endl;
    stream << "Median: " << median << " ns" << std::endl;
    stream << "0th-Percentile: " << percentile0 << " ns" << std::endl;
    stream << "10th-Percentile: " << percentile10 << " ns" << std::endl;
    stream << "20th-Percentile: " << percentile20 << " ns" << std::endl;
    stream << "40th-Percentile: " << percentile40 << " ns" << std::endl;
    stream << "60th-Percentile: " << percentile60 << " ns" << std::endl;
    stream << "80th-Percentile: " << percentile80 << " ns" << std::endl;
    stream << "90th-Percentile: " << percentile90 << " ns" << std::endl;
    stream << "99th-Percentile: " << percentile99 << " ns" << std::endl;
    stream << "100th-Percentile: " << percentile100 << " ns" << std::endl;
}

void LatencyResult::printAsMicroSeconds(std::ostream &stream) const
{
    double throughput;
    std::string throughputUnit;
    std::tie(throughput, throughputUnit) = normalizeThroughput(pps * packetSize);

    stream << "Id: " << id << std::endl;
    stream << "InName: " << inName << std::endl;
    stream << "ConEstab: " << connectionEstablishment / 1000 << " us" << std::endl;
    stream << "PacketSize: " << packetSize << std::endl;
    stream << "PacketCount: " << delayList.size() << std::endl;
    stream << "Pps: " << pps << std::endl;
    stream << "Throughput: " << std::fixed << std::setprecision(2) << throughput << " " << throughputUnit << std::endl;
    stream << "AVG: " << avg / 1000 << " us" << std::endl;
    stream << "Jitter: " << jitter / 1000 << " us" << std::endl;
    stream << "Median: " << median / 1000 << " us" << std::endl;
    stream << "0th-Percentile: " << percentile0 / 1000 << " us" << std::endl;
    stream << "10th-Percentile: " << percentile10 / 1000 << " us" << std::endl;
    stream << "20th-Percentile: " << percentile20 / 1000 << " us" << std::endl;
    stream << "40th-Percentile: " << percentile40 / 1000 << " us" << std::endl;
    stream << "60th-Percentile: " << percentile60 / 1000 << " us" << std::endl;
    stream << "80th-Percentile: " << percentile80 / 1000 << " us" << std::endl;
    stream << "90th-Percentile: " << percentile90 / 1000 << " us" << std::endl;
    stream << "99th-Percentile: " << percentile99 / 1000 << " us" << std::endl;
    stream << "100th-Percentile: " << percentile100 / 1000 << " us" << std::endl;
}

void LatencyResult::printValues(std::ostream &stream, const std::string& separator) const
{
    stream << id << separator;
    stream << inName << separator;
    stream << connectionEstablishment << separator;
    stream << packetSize << separator;
    stream << delayList.size() << separator;
    stream << pps << separator;
    stream << pps * packetSize * 8 << separator;
    stream << avg <<separator;
    stream << jitter << separator;
    stream << median << separator;
    stream << percentile0 << separator;
    stream << percentile10 << separator;
    stream << percentile20 << separator;
    stream << percentile40 << separator;
    stream << percentile60 << separator;
    stream << percentile80 << separator;
    stream << percentile90 << separator;
    stream << percentile99 << separator;
    stream << percentile100 << std::endl;
}

int64_t LatencyResult::calcJitter(int64_t avgDelay, const std::vector<int64_t> &ascOrderedDelayList)
{
    int64_t jitterSum = 0;
    for (size_t j = 0; j < ascOrderedDelayList.size(); ++j)
        jitterSum += static_cast<size_t>(std::abs(static_cast<ssize_t>(ascOrderedDelayList[j]) - static_cast<ssize_t>(avgDelay)));

    return jitterSum / static_cast<ssize_t>(ascOrderedDelayList.size());
}

size_t LatencyResult::getIndexForNthPertcentile(double nthPercentile, size_t size)
{
    if(size == 0)
        return 0;

    auto index = std::ceil(size * nthPercentile);

    if(index > 0)
        --index;

    return static_cast<size_t>(index);
}

int64_t LatencyResult::getNthPercentile(double nthPercentile,
                                                     const std::vector<int64_t> &ascOrderedDelayList)
{
    if(ascOrderedDelayList.empty())
        return 0;

    return ascOrderedDelayList[getIndexForNthPertcentile(nthPercentile, ascOrderedDelayList.size())];
}

int64_t LatencyResult::getMedian(const std::vector<int64_t> &ascOrderedDelayList)
{
    return getNthPercentile(0.5, ascOrderedDelayList);
}

std::unique_ptr<LatencyResult> LatencyResult::analyse(size_t id,
                                     const std::string &name,
                                     const size_t connectionEstablishment,
                                     const size_t runtime,
                                     const size_t packetSize,
                                     std::vector<int64_t> &&delayList)
{
    auto latencyResult = std::make_unique<LatencyResult>();
    latencyResult->id = id;
    latencyResult->inName = name;
    latencyResult->delayList = std::move(delayList);
    latencyResult->packetSize = packetSize;
    latencyResult->connectionEstablishment = connectionEstablishment;

    if(latencyResult->delayList.size() == 0)
        return latencyResult;

    if(runtime > 0)
        latencyResult->pps = static_cast<size_t>(std::ceil(latencyResult->delayList.size() / static_cast<double>(runtime)));

    int64_t delaySum = std::accumulate(latencyResult->delayList.begin(), latencyResult->delayList.end(), 0l);

    latencyResult->avg = delaySum / static_cast<ssize_t>(latencyResult->delayList.size());
    latencyResult->jitter = calcJitter(latencyResult->avg, latencyResult->delayList);

    //sort for metrics below
    auto sortedList = latencyResult->delayList;
    std::sort(sortedList.begin(), sortedList.end());
    latencyResult->median = getMedian(sortedList);
    latencyResult->percentile0 = getNthPercentile(0, sortedList);
    latencyResult->percentile10 = getNthPercentile(0.1, sortedList);
    latencyResult->percentile20 = getNthPercentile(0.2, sortedList);
    latencyResult->percentile40 = getNthPercentile(0.4, sortedList);
    latencyResult->percentile60 = getNthPercentile(0.6, sortedList);
    latencyResult->percentile80 = getNthPercentile(0.8, sortedList);
    latencyResult->percentile90 = getNthPercentile(0.9, sortedList);
    latencyResult->percentile99 = getNthPercentile(0.99, sortedList);
    latencyResult->percentile100 = sortedList[sortedList.size() - 1];

    latencyResult->ascSortedDelayList = std::move(sortedList);

    return latencyResult;
}

void LatencyResult::dumpLatencies(std::ostream& stream) const
{
    if (delayList.empty())
        return;

    for (size_t i = 0; i < delayList.size(); ++i)
    {
        stream << delayList[i] << std::endl;
    }
}

void LatencyResult::dumpBargraphMetadata(std::ostream &stream, const size_t percentilCount) const
{
    if (delayList.size() == 0 || ascSortedDelayList.size() == 0)
        return;

    std::vector<int64_t> percentileList;
    percentileList.resize(percentilCount + 1);//+1 so we get 0 to 1000

    for (size_t i = 0; i <= percentilCount; ++i)
    {
        percentileList[i] = getNthPercentile(static_cast<double>(i) / percentilCount, ascSortedDelayList);
    }

    auto q25Index = getIndexForNthPertcentile(0.25, percentileList.size());
    auto q75Index = getIndexForNthPertcentile(0.75, percentileList.size());
    auto q25 = percentileList[q25Index];
    auto q75 = percentileList[q75Index];
    auto iqr = q75 - q25;
    auto lower = q25 - 1.5 * iqr;
    auto upper = q75 + 1.5 * iqr;
    auto zahl1It = std::lower_bound(ascSortedDelayList.begin(), ascSortedDelayList.end(), lower);
    auto zahl2It = std::upper_bound(ascSortedDelayList.begin(), ascSortedDelayList.end(), upper);

    if (zahl2It != ascSortedDelayList.begin())
        zahl2It = std::prev(zahl2It);

    if (std::find(percentileList.begin(), percentileList.end(), *zahl1It) == percentileList.end())
        stream << *zahl1It << std::endl;
    if (std::find(percentileList.begin(), percentileList.end(), *zahl2It) == percentileList.end())
        stream << *zahl2It << std::endl;

    for (size_t j = 0; j <= percentilCount; ++j)
    {
        stream << percentileList[j] << std::endl;
    }
}

std::tuple<double, std::string> LatencyResult::normalizeThroughput(size_t bytesPerSecond)
{
    double bitsPerSecond = bytesPerSecond * 8.0;
    bool shallBeKBytePs = bitsPerSecond >= 1<<10;
    bool shallBeMBytePs = bitsPerSecond >= 1<<20;

    std::string throughputUnit = "bps";

    if(shallBeMBytePs)
    {
        throughputUnit = "Mbps";
        bitsPerSecond /= 1024.0 * 1024.0;
    }
    else if(shallBeKBytePs)
    {
        throughputUnit = "kbps";
        bitsPerSecond /= 1024.0;
    }

    return std::tuple<double, std::string>(bitsPerSecond, std::move(throughputUnit));
}
