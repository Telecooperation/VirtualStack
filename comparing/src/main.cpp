
#include "RawSocketTestCase.h"
#include "StackTestCase.h"
#include "StackWithRecvControllerTestCase.h"
#include "VSRouterOnly.h"
#include "VSRouterOnlyStack.h"
#include "VSSnippetMultiSocket.h"
#include "VSSnippetSocket.h"
#include "VSSnippetSocketRouted.h"
#include "VSSnippetSocketRouted2.h"
#include "VSSnippetMultiSocketRouted.h"

#include <utility>
#include <iomanip>

struct CompareCase
{
#define CompareDelegate(compareFun) [](size_t runtime, double dataRateInMbps, size_t packetSize, size_t stackSendBufferSize) { return compareFun; }

    const std::string name;
    const std::function<std::unique_ptr<LatencyMeter>(size_t, double, size_t, size_t)> fun;
    const bool placeholder;

    CompareCase(std::string pName, std::function<std::unique_ptr<LatencyMeter>(size_t, double, size_t, size_t)> pFun, bool withPlaceHolder = false) :
            name(std::move(pName)),
            fun(std::move(pFun)),
            placeholder(withPlaceHolder)
    {}
};

template <typename T>
void readVariable(const std::string& text, T& val)
{
    do
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << text << ": ";
    }
    while (std::cin.peek() != '\n' && !(std::cin >> val));
}

int main(int argc, char** argv)
{
    std::vector<CompareCase> compareCases{
            CompareCase("AppToApp TCP", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp SCTP", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDP", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDPLite", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp DCCP", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDP-Plus", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDP-Plus-NoFlowFec", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusNoFlowFecIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDP-Plus-NoFec", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusNoFecIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp UDP-Plus-OnlyFlow", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusOnlyFlowIPv4,
                                         stackSendBufferSize))),
            CompareCase("AppToApp SoftwareLoop", CompareDelegate(
                    VSSnippetSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::SoftwareLoop,
                                         stackSendBufferSize)), true),

            CompareCase("Router TCP to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::TCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router TCP to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::SCTPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router TCP to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::UDPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router TCP to DCCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::DCCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router TCP to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("Router SCTP to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4, StackEnum::TCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router SCTP to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4, StackEnum::SCTPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router SCTP to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4, StackEnum::UDPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router SCTP to DCCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4, StackEnum::DCCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router SCTP to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("Router UDP to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4, StackEnum::TCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDP to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4, StackEnum::SCTPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDP to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4, StackEnum::UDPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDP to DCCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4, StackEnum::DCCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDP to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("Router UDPLite to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4, StackEnum::TCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDPLite to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4, StackEnum::SCTPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDPLite to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4, StackEnum::UDPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDPLite to DCCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4, StackEnum::DCCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router UDPLite to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("Router DCCP to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4, StackEnum::TCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router DCCP to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4, StackEnum::SCTPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router DCCP to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4, StackEnum::UDPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router DCCP to DCCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4, StackEnum::DCCPIPv4,
                                      stackSendBufferSize))),
            CompareCase("Router DCCP to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("Router UDP-Plus to TCP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                           StackEnum::TCPIPv4, stackSendBufferSize))),
            CompareCase("Router UDP-Plus to SCTP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                           StackEnum::SCTPIPv4, stackSendBufferSize))),
            CompareCase("Router UDP-Plus to UDP", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                           StackEnum::UDPIPv4, stackSendBufferSize))),
            CompareCase("Router UDP-Plus to UDP-Plus", CompareDelegate(
                    VSRouterOnlyStack::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                           StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),

            CompareCase("Stack-only with TCP", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, stackSendBufferSize))),
            CompareCase("Stack-only with SCTP", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::SCTPIPv4, stackSendBufferSize))),
            CompareCase("Stack-only with UDP", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPIPv4, stackSendBufferSize))),
            CompareCase("Stack-only with UDPLite", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPLITEIPv4,
                                       stackSendBufferSize))),
            CompareCase("Stack-only with DCCP", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::DCCPIPv4, stackSendBufferSize))),
            CompareCase("Stack-only with UDP-Plus", CompareDelegate(
                    StackTestCase::run(runtime, dataRateInMbps, packetSize, StackEnum::UDPPlusIPv4,
                                       stackSendBufferSize)), true),
            CompareCase("Raw TCP", CompareDelegate(
                    RawSocketTestCase::run(runtime, dataRateInMbps, packetSize, TransportProtocolEnum::TCP))),
            CompareCase("Raw SCTP", CompareDelegate(
                    RawSocketTestCase::run(runtime, dataRateInMbps, packetSize, TransportProtocolEnum::SCTP))),
            CompareCase("Raw UDP", CompareDelegate(
                    RawSocketTestCase::run(runtime, dataRateInMbps, packetSize, TransportProtocolEnum::UDP))),
            CompareCase("Raw UDPLite", CompareDelegate(
                    RawSocketTestCase::run(runtime, dataRateInMbps, packetSize, TransportProtocolEnum::UDPLITE))),
            CompareCase("Raw DCCP", CompareDelegate(
                    RawSocketTestCase::run(runtime, dataRateInMbps, packetSize, TransportProtocolEnum::DCCP)), true),

            CompareCase("AppToApp TCP and TCP", CompareDelegate(
                    VSSnippetMultiSocket::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::TCPIPv4,
                                              stackSendBufferSize)), true),
            CompareCase("AppToApp TCP and TCP with Router", CompareDelegate(
                    VSSnippetMultiSocketRouted::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4, StackEnum::TCPIPv4)), true),

            CompareCase("StackWithRecvController with TCP", CompareDelegate(
                    StackWithRecvControllerTestCase::run(runtime, StackEnum::TCPIPv4, stackSendBufferSize))),
            CompareCase("StackWithRecvController with UDP-Plus", CompareDelegate(
                    StackWithRecvControllerTestCase::run(runtime, StackEnum::UDPPlusIPv4, stackSendBufferSize)), true),
            CompareCase("VirtualStack with Router TCP", CompareDelegate(
                    VSSnippetSocketRouted::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4))),
            CompareCase("VirtualStack with 2 Routers TCP", CompareDelegate(
                    VSSnippetSocketRouted2::run(runtime, dataRateInMbps, packetSize, StackEnum::TCPIPv4)), true),
    };

    const size_t maxPayloadSize = 1400ul; //MAX Value for
    int selection = 0;
    size_t runtime = 10;
    double dataRateInMbps = 0;
    size_t payloadSize = maxPayloadSize;
    size_t stackSendBufferSize = 8;
    bool enableDelayDump = false;
    std::vector<std::string> nameAppendixList;

    if (argc >= 6)
    {
        selection = atoi(argv[1]);
        runtime = static_cast<size_t>(atoll(argv[2]));
        dataRateInMbps = atof(argv[3]);
        payloadSize = static_cast<size_t>(atoll(argv[4]));
        stackSendBufferSize = static_cast<size_t>(atoll(argv[5]));

        if (argc >= 7)
            enableDelayDump = static_cast<bool>(atoi(argv[6]));
        if (argc >= 8)
        {
            for (int i = 7; i < argc; ++i)
                nameAppendixList.push_back(argv[i]);
        }
    } else
    {
        std::stringstream printString;

        for (size_t i = 0; i < compareCases.size(); ++i)
        {
            printString << "\t" << (i + 1) << ". " << compareCases[i].name << std::endl;

            if (compareCases[i].placeholder)
                printString << "\t-------------" << std::endl;
        }
        std::cout << printString.str() << std::flush;

        while (selection < 1 || selection > static_cast<int>(compareCases.size()))
        {
            std::cout << "Enter selection (1-" << compareCases.size() << "): ";
            if (!(std::cin >> selection))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                selection = 0;
            }
        };

        readVariable("Enter Runtime in s [" + std::to_string(runtime) + "]", runtime);
        readVariable("Enter DataRate in Mbit/s ["+ std::to_string(dataRateInMbps) + "]", dataRateInMbps);
        readVariable("Enter PayloadSize between 0-" + std::to_string(maxPayloadSize) + " [" + std::to_string(payloadSize) + "]", payloadSize);
        readVariable("Enter StackSendBufferSize (multiple of 2 and greater 1) (2,4,8,16,...) [" + std::to_string(stackSendBufferSize) + "]", stackSendBufferSize);
        readVariable("Enable Delay-Dump [" + std::to_string(enableDelayDump) + "]", enableDelayDump);
    }

    payloadSize = std::min(payloadSize, maxPayloadSize);

    const auto compareCaseIndex = static_cast<size_t>(selection - 1);
    auto &compageCase = compareCases[compareCaseIndex];
    auto result = compageCase.fun(runtime, dataRateInMbps, payloadSize, stackSendBufferSize);

    if (!result)
        return 0;

    auto res = result->analyse(runtime);
    if (res.empty())
    {
        Logger::Log(Logger::ERROR, "No data was available for LatencyResult");
        return 0;
    }

    auto &analysis = res[0];

    const auto separator = "_";
    std::stringstream fileName;

    fileName << std::fixed << std::setprecision(2) <<
             compageCase.name << separator <<
             runtime << "s" << separator <<
             dataRateInMbps << "Mbps" << separator <<
             payloadSize << "B" << separator <<
             stackSendBufferSize;

    if (!nameAppendixList.empty())
    {
        for (size_t i = 0; i < nameAppendixList.size(); ++i)
            fileName << separator << nameAppendixList[i];
    }

    const std::string metadataFilename = fileName.str() + ".vsmeta";
    const std::string bargraphFilename = fileName.str() + ".vsbar";
    const std::string latencyFilename = fileName.str() + ".vsdelay";

    std::cout << "##### " << compageCase.name << " #####" << std::endl;
    std::cout << metadataFilename << ":" << bargraphFilename << ":" << latencyFilename << std::endl;
    analysis->printValues(std::cout, "\t");
    analysis->printAsMicroSeconds(std::cout);

    std::ofstream metadataFile{metadataFilename};
    analysis->printValues(metadataFile, "\t");
    analysis->printAsMicroSeconds(metadataFile);
    metadataFile.close();

    std::ofstream bargraphFile{bargraphFilename};
    analysis->dumpBargraphMetadata(bargraphFile, 1000);
    bargraphFile.close();

    if (enableDelayDump)
    {
        std::ofstream latencyFile{latencyFilename};
        analysis->dumpLatencies(latencyFile);
        latencyFile.close();
    }


    return 0;
}