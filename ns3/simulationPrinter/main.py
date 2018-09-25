# coding=utf-8
import math
import os
import subprocess
import sys
import time
import zipfile

import pandas as pd
import seaborn as sns
import statistics
from scipy.stats import stats

from plotHelper import *
from plotter.heatMapPlotter import HeatMapPlotter
from plotter.stackedBarPlotter import StackedBarPlotter
from utils.listUtils import ListUtils

from metadata import Metadata


def plotAppToAppLoopback_1_PC(zipHandle, plotFolder):
    plotName = "1_AppToAppLoopback_PC"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, False)

    sortedData = sorted(metadata, key=Metadata.bandwidthKeyFn)
    plotVSBoxPlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.bandwidthNameFn)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500.0),
                   Metadata.bandwidthNameFn, yValName="throughput [PPS]", yValFn=lambda meta: meta.PPS)

    sortedData = filter(lambda x: x.bandwidth != "0.00Mbps" and x.bandwidth != "0Mbps", sortedData)
    plotVSBoxPlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]", Metadata.bandwidthNameFn)
    plotVSLinePlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]",
                   Metadata.getBandwidthKeyFn(1500.0), Metadata.bandwidthNameFn, yValName="throughput [PPS]",
                   yValFn=lambda meta: meta.PPS)


def plotAppToAppLoopback_1(zipHandle, plotFolder):
    plotName = "1_AppToAppLoopback"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, False)

    sortedData = sorted(metadata, key=Metadata.bandwidthKeyFn)
    plotVSBoxPlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.bandwidthNameFn)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500.0),
                   Metadata.bandwidthNameFn, yValName="throughput [PPS]", yValFn=lambda meta: meta.PPS)

    sortedData = filter(lambda x: x.bandwidth != "0.00Mbps" and x.bandwidth != "0Mbps", sortedData)
    plotVSBoxPlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]", Metadata.bandwidthNameFn)
    plotVSLinePlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName + "_limited", "bandwidth [Mbps]",
                   Metadata.getBandwidthKeyFn(1500.0), Metadata.bandwidthNameFn, yValName="throughput [PPS]",
                   yValFn=lambda meta: meta.PPS)


def plotRawVariableBW_2(zipHandle, plotFolder):
    plotName = "2_RawVariableBW"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    plotGroupedVSBoxPlot(ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn]),
                         vsPlotPath, plotName, )

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                      xAxisCaption="bandwidth [Mbps]", boxNameFn=Metadata.bandwidthNameFn)


def plotAppToApp_MouseFlow_3a(zipHandle, plotFolder):  # TODO: Verbindungsaufbauplot für alle 3a,b Methoden
    plotName = "3a_MouseFlow_AppToApp"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    plotVSBoxPlot(sorted(metadata, key=lambda x: x.protocol), vsPlotPath, plotName)


def plotRouter_MouseFlow_3a(zipHandle, plotFolder):  # TODO: extra Diagramm mit besten 5 nach Median und 80er-Percentil
    plotName = "3a_MouseFlow_Router"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[0], lambda x: x.protocol.split(' ')[2]])
    plotGroupedVSBoxPlot(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[0], lambda x: x.protocol.split(' ')[2]]),
        vsPlotPath, plotName + "_from_all", "source protocol", lambda x: x.protocol.split(' ')[0],
        lambda x: x.protocol.split(' ')[2], "target protocol")
    plotGroupedVSBoxPlot(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[2], lambda x: x.protocol.split(' ')[0]]),
        vsPlotPath, plotName + "_to_all", "target protocol", lambda x: x.protocol.split(' ')[2],
        lambda x: x.protocol.split(' ')[0], "source protocol")

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol.split(' ')[2])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=lambda x: x.protocol.split(' ')[0]), vsPlotPath,
                      plotName + "_to_" + protocol, "protocol conversion")

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol.split(' ')[0])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=lambda x: x.protocol.split(' ')[2]), vsPlotPath,
                      plotName + "_from_" + protocol, "protocol conversion")


def plotAppToApp_ElephantFlow_3b(zipHandle, plotFolder):
    plotName = "3b_ElephantFlow_AppToApp"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    sortedData = ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn])
    reliableProtoData = [x for x in sortedData if
                         "TCP" in x.protocol or "SCTP" in x.protocol or "UDP-Plus" in x.protocol]
    unreliableProtoData = [x for x in sortedData if not x in reliableProtoData]

    plotGroupedVSBoxPlot(reliableProtoData, vsPlotPath, plotName + "_reliable")
    plotGroupedVSBoxPlot(unreliableProtoData, vsPlotPath, plotName + "_unreliable")
    plotGroupedVSBoxPlot(unreliableProtoData, vsPlotPath, plotName + "_unreliable_log", log=True)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [Mbps]", yValFn=lambda x: x.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(1500),
                   Metadata.bandwidthNameFn, yValName="throughput [PPS]", yValFn=lambda x: x.PPS)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                      "bandwidth [Mbps]", Metadata.bandwidthNameFn)


def plotRouter_ElephantFlow_3b(zipHandle, plotFolder):  # TODO: Pro Bandbreite Plot wie bei Mouseflow-Router
    plotName = "3b_ElephantFlow_Router"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    relSource = lambda x: "TCP to" in x.protocol or "SCTP to" in x.protocol or "UDP-Plus to" in x.protocol
    relDest = lambda x: "to TCP" in x.protocol or "to SCTP" in x.protocol or "to UDP-Plus" in x.protocol
    unrelSource = lambda x: not relSource(x)
    unrelDest = lambda x: not relDest(x)

    reliableProtoData = [x for x in metadata if relSource(x) and relDest(x)]
    relToUnrelProtoData = [x for x in metadata if relSource(x) and unrelDest(x)]
    unreliableProtoData = [x for x in metadata if unrelSource(x) and unrelDest(x)]
    unrelToRelProtoData = [x for x in metadata if unrelSource(x) and relDest(x)]

    relSortedFrom = ListUtils.sortbymultiple(reliableProtoData,
                                             [lambda x: x.protocol.split(' ')[0], Metadata.bandwidthKeyFn])
    unrelSortedFrom = ListUtils.sortbymultiple(unreliableProtoData,
                                               [lambda x: x.protocol.split(' ')[0], Metadata.bandwidthKeyFn])
    relUnrelSortedFrom = ListUtils.sortbymultiple(relToUnrelProtoData,
                                                  [lambda x: x.protocol.split(' ')[0], Metadata.bandwidthKeyFn])
    unrelRelSortedFrom = ListUtils.sortbymultiple(unrelToRelProtoData,
                                                  [lambda x: x.protocol.split(' ')[0], Metadata.bandwidthKeyFn])

    plotGroupedVSBoxPlot(relSortedFrom, vsPlotPath, plotName + "_reliable_to_reliable", "protocol conversion")
    plotGroupedVSBoxPlot(relSortedFrom, vsPlotPath, plotName + "_reliable_to_reliable_log", "protocol conversion",
                         log=True)
    plotGroupedVSBoxPlot(unrelSortedFrom, vsPlotPath, plotName + "_unreliable_to_unreliable", "protocol conversion")
    plotGroupedVSBoxPlot(unrelSortedFrom, vsPlotPath, plotName + "_unreliable_to_unreliable_log", "protocol conversion",
                         log=True)
    plotGroupedVSBoxPlot(relUnrelSortedFrom, vsPlotPath, plotName + "_reliable_to_unreliable", "protocol conversion")
    plotGroupedVSBoxPlot(relUnrelSortedFrom, vsPlotPath, plotName + "_reliable_to_unreliable_log",
                         "protocol conversion", log=True)
    plotGroupedVSBoxPlot(unrelRelSortedFrom, vsPlotPath, plotName + "_unreliable_to_reliable", "protocol conversion")

    groupedByProtocol = ListUtils.groupby(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[0], Metadata.bandwidthKeyFn]),
        lambda meta: meta.protocol.split(' ')[2])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotGroupedVSBoxPlot(metaList, vsPlotPath, plotName + "_to_" + protocol, "protocol conversion")

    groupedByProtocol = ListUtils.groupby(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[2], Metadata.bandwidthKeyFn]),
        lambda meta: meta.protocol.split(' ')[0])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotGroupedVSBoxPlot(metaList, vsPlotPath, plotName + "_from_" + protocol, "protocol conversion")

    groupedBybandwidth = ListUtils.groupby(metadata, lambda meta: meta.bandwidth)
    for (bandwidth, metaList) in groupedBybandwidth.iteritems():
        data = [('from', 'to', 'throughput [Mbps]', 'median latency [ns]')] + map(
            lambda x: (x.protocol.split(' ')[0], x.protocol.split(' ')[2], x.throughput, x.latMedian), metaList)

        dataFrame = pd.DataFrame(data[1:], columns=data[0])
        dataFrame = dataFrame.pivot(index=data[0][0], columns=data[0][1], values=data[0][2])
        test = HeatMapPlotter("Throughput - " + bandwidth)
        test.plot(vsPlotPath, dataFrame, data[0][2])

        dataFrame = pd.DataFrame(data[1:], columns=data[0])
        dataFrame = dataFrame.pivot(index=data[0][0], columns=data[0][1], values=data[0][3])
        test = HeatMapPlotter("latency - " + bandwidth)
        test.plot(vsPlotPath, dataFrame, data[0][3])


def plotAppToAppRisingPayloadSize_4(zipHandle, plotFolder):  # TODO: Y-Achse stauchen, damit Daten enger liegen
    plotName = "4_AppToAppRisingPayloadSize"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    plotVSBoxPlot(sorted(metadata, key=lambda x: x.payloadSize), vsPlotPath, plotName, "payload size [B]",
                  boxNameFn=lambda x: x.payloadSize)
    plotVSLinePlot(sorted(metadata, key=lambda x: x.payloadSize), vsPlotPath, plotName, "payload size [B]",
                   lambda x: x.payloadSize, lambda x: str(x.payloadSize) + "B", yValName="throughput [Mbps]",
                   yValFn=lambda meta: meta.throughput)
    plotVSLinePlot(sorted(metadata, key=lambda x: x.payloadSize), vsPlotPath, plotName, "payload size [B]",
                   lambda x: x.payloadSize, lambda x: str(x.payloadSize) + "B", yValName="throughput [PPS]",
                   yValFn=lambda meta: meta.PPS, yLim=[1800000, 2400000])


def plotRisingSendStackSize_5_PC(zipHandle, plotFolder):
    plotName = "5_RisingSendStackSize"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    a2aTcp = ("_TCP", sorted(filter(lambda x: x.simName == "AppToApp" and x.protocol == "TCP", metadata),
                             key=lambda x: x.sendStackBufferSize))
    a2aLoop = ("_Loop", sorted(filter(lambda x: x.simName == "AppToApp" and x.protocol != "TCP", metadata),
                               key=lambda x: x.sendStackBufferSize))
    routerTcp = (
    "_Router", sorted(filter(lambda x: x.simName == "Router", metadata), key=lambda x: x.sendStackBufferSize))
    for run in [a2aTcp, a2aLoop, routerTcp]:
        plotVSBoxPlot(run[1], vsPlotPath, plotName + run[0], "send stack buffer size [packets]",
                      boxNameFn=lambda x: x.sendStackBufferSize)
        plotVSLinePlot(run[1], vsPlotPath, plotName + run[0], "send stack buffer size [packets]",
                       lambda x: x.sendStackBufferSize, lambda x: x.sendStackBufferSize, yValName="throughput [PPS]",
                       yValFn=lambda meta: meta.PPS)


def plotRisingSendStackSize_5(zipHandle, plotFolder):
    plotName = "5_RisingSendStackSize_Server"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    a2aTcp = ("_TCP", sorted(filter(lambda x: x.simName == "AppToApp" and x.protocol == "TCP", metadata),
                             key=lambda x: x.sendStackBufferSize))
    a2aLoop = ("_Loop", sorted(filter(lambda x: x.simName == "AppToApp" and x.protocol != "TCP", metadata),
                               key=lambda x: x.sendStackBufferSize))
    routerTcp = (
    "_Router", sorted(filter(lambda x: x.simName == "Router", metadata), key=lambda x: x.sendStackBufferSize))
    for run in [a2aTcp, a2aLoop, routerTcp]:
        plotVSBoxPlot(run[1], vsPlotPath, plotName + run[0], "send stack buffer size [packets]",
                      boxNameFn=lambda x: x.sendStackBufferSize)
        plotVSLinePlot(run[1], vsPlotPath, plotName + run[0], "send stack buffer size [packets]",
                       lambda x: x.sendStackBufferSize, lambda x: x.sendStackBufferSize, yValName="throughput [PPS]",
                       yValFn=lambda meta: meta.PPS)


def plotCPULimit_AppToApp_Router_TCP_6(zipHandle, plotFolder):
    plotName = "6_CPULimit"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    routerTcp = ("_TCP_Router", sorted(filter(lambda x: x.simName == "Router" and x.bandwidth == "0.00Mbps", metadata),
                                       key=Metadata.cpuLimitKeyFn))
    plotVSLinePlot(routerTcp[1], vsPlotPath, plotName + routerTcp[0], "cpu limit [%]", Metadata.getCpuLimitKeyFn(700),
                   Metadata.cpuLimitNameFn, yValName="throughput [PPS]", yValFn=lambda meta: meta.PPS)
    metadata = filter(lambda x: x.simName != "Router", metadata)
    groupedByBandwidth = ListUtils.groupby(metadata, lambda meta: meta.bandwidth)
    for (bandwidth, metaList) in groupedByBandwidth.iteritems():
        groupedByProtocol = ListUtils.groupby(metaList, lambda meta: meta.protocol)
        for (protocol, metametaList) in groupedByProtocol.iteritems():
            sortedData = sorted(metametaList, key=Metadata.cpuLimitKeyFn)
            plotVSBoxPlot(sortedData, vsPlotPath, plotName + "_" + protocol + "_" + bandwidth, "cpu limit [%]",
                          Metadata.cpuLimitNameFn)
            plotVSLinePlot(sortedData, vsPlotPath, plotName + "_" + protocol + "_" + bandwidth, "cpu limit [%]",
                           Metadata.getCpuLimitKeyFn(Metadata.cpuLimitKeyFn(sortedData[-2]) + 200),
                           Metadata.cpuLimitNameFn, yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
            plotVSLinePlot(sortedData, vsPlotPath, plotName + "_" + protocol + "_" + bandwidth, "cpu limit [%]",
                           Metadata.getCpuLimitKeyFn(Metadata.cpuLimitKeyFn(sortedData[-2]) + 200),
                           Metadata.cpuLimitNameFn, yValName="throughput [PPS]", yValFn=lambda meta: meta.PPS)


def plotNS3Raw_7a(zipHandle, plotFolder):
    plotName = "7a_NS3Raw"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotGroupedVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                             "bandwidth [Mbps]", Metadata.bandwidthNameFn, lambda x: x.payloadSize, "payload size [B]")
        plotGroupedVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath,
                             plotName + "_" + protocol + "_log", "bandwidth [Mbps]", Metadata.bandwidthNameFn,
                             lambda x: x.payloadSize, "payload size [B]", log=True)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [Mbps]", lambda x: x.throughput)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [PPS]", lambda x: x.PPS)


def plotNS3RawMouseFlow_7b(zipHandle, plotFolder):
    plotName = "7b_RawMouseFlow"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    plotVSBoxPlot(metadata, vsPlotPath, plotName, "xaxis", Metadata.bandwidthNameFn)


def plotNS3RawElephantFlow_7b(zipHandle, plotFolder):
    plotName = "7b_RawElephantFlow"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    plotVSBoxPlot(sorted(metadata, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName, "bandwidth [Mbps]",
                  Metadata.bandwidthNameFn)
    plotVSLinePlot(ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn]), vsPlotPath,
                   plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120), Metadata.bandwidthNameFn,
                   yValName="throughput [Mbps]", yValFn=lambda x: x.throughput)
    plotVSLinePlot(ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn]), vsPlotPath,
                   plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120), Metadata.bandwidthNameFn,
                   yValName="throughput [PPS]", yValFn=lambda x: x.PPS)


def plotNS3VS_7c(zipHandle, plotFolder):
    plotName = "7c_VS"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotGroupedVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                             "bandwidth [Mbps]", Metadata.bandwidthNameFn, lambda x: x.payloadSize, "payload size [B]")
        plotGroupedVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath,
                             plotName + "_" + protocol + "_log", "bandwidth [Mbps]", Metadata.bandwidthNameFn,
                             lambda x: x.payloadSize, "payload size [B]", log=True)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [Mbps]", lambda x: x.throughput)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [PPS]", lambda x: x.PPS)


def plotNS3VSMouseFlow_7d(zipHandle, plotFolder):
    plotName = "7d_VSMouseFlow"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    plotVSBoxPlot(metadata, vsPlotPath, plotName, "xaxis", Metadata.bandwidthNameFn)


def plotNS3VSElephantFlow_7d(zipHandle, plotFolder):
    plotName = "7d_VSElephantFlow"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=True)

    plotGroupedVSBoxPlot(metadata, vsPlotPath, plotName)
    plotGroupedVSBoxPlot(metadata, vsPlotPath, plotName + "_logarithmic", log=True)
    plotVSLinePlot(ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn]), vsPlotPath,
                   plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(200.0), Metadata.bandwidthNameFn,
                   yValName="throughput [Mbps]", yValFn=lambda x: x.throughput)


def plotNS3Simulated_8a(zipHandle, plotFolder):
    plotName = "8a_NS3Simulated"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotGroupedVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                             "bandwidth [Mbps]", Metadata.bandwidthNameFn, lambda x: x.payloadSize, "payload size [B]")
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [Mbps]", lambda x: x.throughput)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(120.0), Metadata.bandwidthNameFn,
                       "payload size [B]", lambda x: int(x.payloadSize), "throughput [PPS]", lambda x: x.PPS)


def plotNS3LatencyCoefficient_999(zipHandle, plotFolder):
    plotName = "999_NS3LTELatencyCoefficient"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.lteBandwidth)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        sortedData = ListUtils.sortbymultiple(metaList, [lambda x: x.lteCoefficient, Metadata.bandwidthKeyFn])
        plotGroupedVSBoxPlot(sortedData, vsPlotPath, plotName + "_" + protocol, "application bandwidth [Mbps]",
                             Metadata.bandwidthNameFn, lambda x: x.lteCoefficient, "latency coefficient")
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "application bandwidth [Mbps]", Metadata.getBandwidthKeyFn(125), Metadata.bandwidthNameFn,
                       "LTE coefficient", lambda x: x.lteCoefficient, "throughput [Mbps]", lambda x: x.throughput)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "application bandwidth [Mbps]", Metadata.getBandwidthKeyFn(125), Metadata.bandwidthNameFn,
                       "LTE coefficient", lambda x: x.lteCoefficient, "throughput [PPS]", lambda x: x.PPS)


def plotNS3LatencyCoefficient_998(zipHandle, plotFolder):
    plotName = "998_NS3LTELatencyCoefficient"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.lteBandwidth)
    for (protocol, metaList) in groupedByProtocol.iteritems():
        sortedData = ListUtils.sortbymultiple(metaList, [lambda x: x.lteCoefficient, Metadata.bandwidthKeyFn])
        plotGroupedVSBoxPlot(filter(lambda x: float(x.lteCoefficient) < 1, sortedData), vsPlotPath,
                             plotName + "_" + protocol, "application bandwidth [Mbps]", Metadata.bandwidthNameFn,
                             lambda x: x.lteCoefficient, "latency coefficient")
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "application bandwidth [Mbps]", Metadata.getBandwidthKeyFn(125), Metadata.bandwidthNameFn,
                       "LTE coefficient", lambda x: x.lteCoefficient, "throughput [Mbps]", lambda x: x.throughput)
        plotVSLinePlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName + "_" + protocol,
                       "application bandwidth [Mbps]", Metadata.getBandwidthKeyFn(125), Metadata.bandwidthNameFn,
                       "LTE coefficient", lambda x: x.lteCoefficient, "throughput [PPS]", lambda x: x.PPS)


def plotNS3FinalNetworkProtocolEvaluation_10(zipHandle, plotFolder):
    plotName = "10_NS3FinalNetworkProtocolEvaluation"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    metadata = sorted(metadata, key=lambda x: x.protocol)
    # metadata = filter(lambda x: x.iteration == "0", metadata)

    iteration = "0"
    plotGroupedVSBoxPlot(metadata, vsPlotPath, plotName + "_iteration_" + iteration, "lte/dsl protocols",
                         boxNameFn=lambda meta: " ".join(meta.protocol.split(" ")[:2]),
                         boxGroupName="backbone protocol", boxGroupFn=lambda meta: meta.protocol.split(" ")[2])
    plotVSScatterPlot(metadata, vsPlotPath, plotName + "_iteration_" + iteration, "lte/dsl protocol",
                      lambda x: " ".join(x.protocol.split(" ")[:2]).replace("INVALID", "NONE"),
                      lambda meta: meta.protocol.split(" ")[2], "backbone protocol", "throughput [Mbps]",
                      lambda meta: meta.throughput, scaleValues=False)
    plotVSScatterPlot(metadata, vsPlotPath, plotName + "_" + iteration, "lte/dsl protocols",
                      xValFn=lambda meta: " ".join(meta.protocol.split(" ")[:2]).replace("INVALID", "NONE"),
                      boxGroupName="backbone protocol", boxGroupFn=lambda meta: meta.protocol.split(" ")[2],
                      plotValName="connection establishment time [ns]",
                      plotValFn=lambda meta: meta.connectionEstablishmentTime)


def deAccumulate(lst):
    result = [lst[0]]
    sum = lst[0]
    for elem in lst[1:]:
        result += [elem - sum]
        sum = elem
    return result


def plotNS3FinalNetworkStackedGraph_11(zipHandle, plotFolder):
    plotName = "11_NS3FinalNetworkStacked"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    metadata = sorted(metadata, key=lambda x: x.protocol)

    excluded = [1, 2, 5, 7, 8]

    metadata = filter(lambda meta: meta.controlPoint not in excluded, metadata)

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    data = []
    for (protocol, metaList) in groupedByProtocol.iteritems():
        metaList = sorted(metaList, key=lambda meta: meta.controlPoint)
        maxBranches = max(metaList, key=lambda meta: meta.branch).branch
        maxPoints = max(metaList, key=lambda meta: meta.controlPoint).controlPoint
        for branch in range(0, maxBranches + 1):
            curBranch = ()
            for cp in range(1, maxPoints + 1):
                if cp in excluded:
                    continue
                filtered = filter(lambda meta: meta.controlPoint == cp and meta.branch <= branch, metaList)
                curBranch += (max(filtered, key=lambda meta: meta.branch),)
            data += [curBranch]
    data = sorted(data, key=lambda x: x[-1].latMedian)

    groupedByProtocol = ListUtils.groupby(data, lambda meta: meta[0].protocol.split(" ")[-1])
    for (bbProtocol, metaList) in groupedByProtocol.iteritems():
        values = map(lambda x: tuple(map(lambda meta: meta.latMedian / 1000000.0, x)), metaList)
        values = map(lambda x: tuple(deAccumulate(x)), values)
        labels = map(lambda metaTup: metaTup[0].protocol.split(" ")[:2], metaList)

        legend = [  # zusammenlegen: 1-3, 4, 5-6, 7-9
            "send over LTE/DSL",
            "merge flows",
            "move merged flow through INVS",
            "send over backbone",
        ]

        colors = sns.color_palette("Set3", 20)
        toRemove = colors[1]
        colors = filter(lambda x: x != toRemove, colors)
        plotter = StackedBarPlotter("title" + "_" + bbProtocol, "xAxis", "yAxis")
        plotter.plot(vsPlotPath, values, colors, legend, labels)

    plotName = "11_NS3FinalNetworkProtocolEvaluation"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    metadata = sorted(metadata, key=lambda x: x.protocol)
    metadata = filter(lambda x: x.controlPoint == 9, metadata)

    plotGroupedVSBoxPlot(metadata, vsPlotPath, plotName, "lte/dsl protocols",
                         boxNameFn=lambda meta: " ".join(meta.protocol.split(" ")[:2]),
                         boxGroupName="backbone protocol", boxGroupFn=lambda meta: meta.protocol.split(" ")[2])
    plotVSScatterPlot(metadata, vsPlotPath, plotName, "lte/dsl protocol",
                      lambda x: " ".join(x.protocol.split(" ")[:2]).replace("INVALID", "NONE"),
                      lambda meta: meta.protocol.split(" ")[2], "backbone protocol", "throughput [Mbps]",
                      lambda meta: meta.throughput, scaleValues=False)
    plotVSScatterPlot(metadata, vsPlotPath, plotName, "lte/dsl protocols",
                      xValFn=lambda meta: " ".join(meta.protocol.split(" ")[:2]).replace("INVALID", "NONE"),
                      boxGroupName="backbone protocol", boxGroupFn=lambda meta: meta.protocol.split(" ")[2],
                      plotValName="connection establishment time [ns]",
                      plotValFn=lambda meta: meta.connectionEstablishmentTime)


def plotStateOfTheInternetReport(zipHandle, plotFolder):
    plotName = "_SotiReport"
    data = [
        (2009, 1.7),
        (2010, 1.8),
        (2011, 2.1),
        (2012, 2.6),
        (2013, 3.1),
        (2014, 3.9),
        (2015, 5.0),
        (2016, 6.3),
        (2017, 7.2)]
    plotVSLinePlot(data, plotFolder, plotName, "year", lambda x: x[0], lambda x: x[0],
                   "global average throughput [Mbps]", lambda x: "", "global average throughput [Mbps]", lambda x: x[1])


def statisticalMagick(zipHandle, plotFolder):
    plotName = "0_Magick"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, False)

    metadata = sorted(metadata, key=Metadata.bandwidthKeyFn)
    grouped = ListUtils.groupby(metadata, lambda meta: meta.bandwidth)

    print "Standard deviation"
    for (bandwidth, metaList) in grouped.iteritems():
        print bandwidth + ": " + str(statistics.stdev(*map(lambda x: x.getRawData(), metaList)))

    rawData = map(lambda x: x.getQuantileData(), metadata)
    print stats.f_oneway(*rawData[:-1])
    print stats.ttest_ind(rawData[0], rawData[-1], equal_var=False)


#####################
# POST-MA
#####################

def parseLegacyFilename(filename):
    proto, simName, options, bandwidth, delay, loss = filename.split('-')
    if "DiscretSmallPacket" in filename:
        packetSize = 210
    else:
        packetSize = 1400

    return simName, proto + options, "60s", bandwidth, packetSize, "8", "0", "", "", None, loss, delay


def parseLegacyFile(path, zipHandle):
    print os.path.basename(path)
    import csv
    with zipHandle.open(path) as csvfile:
        tmp = parseLegacyFilename(os.path.splitext(os.path.basename(path))[0])
        metadata = Metadata(*tuple(tmp[:-2]), loss=tmp[-2], delay=tmp[-1])
        metadata.testName = "Legacy-ns-3"
        reader = csv.reader(csvfile)
        data = list(reader)
        header = data[0]  # packetSize	seqNum	sendTime	recvTime
        data = map(lambda tup: tuple(map(lambda item: int(item), tup)), data[1:])
        connPacket = data[0]
        data = filter(lambda x: x[3] > 0, data[1:])
        if (len(data) == 0):
            print "skipping"
            return None
        metadata.connectionEstablishmentTime = connPacket[3] - connPacket[2]
        metadata.packetsSent = len(data)
        metadata.simTime = (data[-1][3] - connPacket[2]) / 1e9
        metadata.PPS = float(metadata.packetsSent) / metadata.simTime
        metadata.throughput = metadata.PPS * metadata.payloadSize
        delays = map(lambda tup: tup[3] - tup[2], data)
        metadata.latAvg = statistics.mean(delays)
        metadata.latMedian = statistics.median(delays)
        metadata.latJitter = statistics.pstdev(delays)
        return metadata


def legacyPlotLatency(metadata, plotFolder):
    plotName = "Legacy_Latency"
    vsPlotPath = prepareLegacyPlot(plotName, plotFolder)
    metadata = sorted(metadata, key=lambda x: x.loss)

    throughputData = filter(lambda x: x.simName == "Throughput", metadata)
    mouseData = filter(lambda x: x.simName != "Throughput", metadata)

    groupedByLoss = ListUtils.groupby(throughputData, lambda meta: meta.loss)
    for (loss, metaList) in groupedByLoss.iteritems():

        plotVSLinePlot(sorted(metaList, key=lambda x: int(x.delay[:-2])), vsPlotPath,
                       plotName + "_throughput_" + str(loss * 100)  + "percent", xValName="propagation delay",
                       xValFn=lambda x: math.log(int(x.delay[:-2])) if x.delay != "0ms" else -1, #hack to create log-esque x-axis, also puts mouse flow to the left
                       xValNameFn=lambda x: x.delay,
                       yValName="latency [ms]", yValFn=lambda meta: meta.latMedian / 1e6, #convert latMedian to ms
                       xLabelSortHack=True)

    groupedByLoss = ListUtils.groupby(mouseData, lambda meta: meta.loss)
    for (loss, metaList) in groupedByLoss.iteritems():
        plotVSLinePlot(sorted(metaList, key=lambda x: int(x.delay[:-2])), vsPlotPath,
                       plotName + "_mouse_" + str(loss * 100)  + "percent",   xValName="propagation delay",
                       xValFn=lambda x: math.log(int(x.delay[:-2])) if x.delay != "0ms" else -1, #hack to create logesque x-axis
                       xValNameFn=lambda x: x.delay,
                       yValName="latency [ms]", yValFn=lambda meta: meta.latMedian / 1e6, #convert latMedian to ms
                       xLabelSortHack=True)


def plotAppToApp_Combined_100(zipHandle, plotFolder):
    plotName = "100_AppToApp_Combined"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit=False)

    sortedData = ListUtils.sortbymultiple(metadata, [lambda x: x.protocol, Metadata.bandwidthKeyFn])
    reliableProtoData = [x for x in sortedData if
                         "TCP" in x.protocol or "SCTP" in x.protocol or "UDP-Plus" in x.protocol]
    unreliableProtoData = [x for x in sortedData if not x in reliableProtoData]

    mouseNameFn = lambda x: Metadata.bandwidthNameFn(x, maxValueName="mouse flow")

    plotGroupedVSBoxPlot(sortedData, vsPlotPath, plotName, boxGroupFn=mouseNameFn)
    plotGroupedVSBoxPlot(reliableProtoData, vsPlotPath, plotName + "_reliable", boxGroupFn=mouseNameFn)
    plotGroupedVSBoxPlot(unreliableProtoData, vsPlotPath, plotName + "_unreliable", boxGroupFn=mouseNameFn)
    plotGroupedVSBoxPlot(unreliableProtoData, vsPlotPath, plotName + "_unreliable_log", log=True,
                         boxGroupFn=mouseNameFn)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(0.0002),
                   mouseNameFn, yValName="throughput [Mbps]", yValFn=lambda x: x.throughput)
    plotVSLinePlot(sortedData, vsPlotPath, plotName, "bandwidth [Mbps]", Metadata.getBandwidthKeyFn(0.0002),
                   mouseNameFn, yValName="throughput [PPS]", yValFn=lambda x: x.PPS)

    # groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol)
    # for (protocol, metaList) in groupedByProtocol.iteritems():
    #    plotVSBoxPlot(sorted(metaList, key=Metadata.bandwidthKeyFn), vsPlotPath, plotName+"_"+protocol, "bandwidth [Mbps]", lambda x: Metadata.bandwidthNameFn(x, maxValueName="mouse flow"))


def plotAppToApp_AllLatency_101(zipHandle, plotFolder):
    plotName = "101_Router_100Mbps"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)

    ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[0], lambda x: x.protocol.split(' ')[2]])
    plotGroupedVSBoxPlot(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[0], lambda x: x.protocol.split(' ')[2]]),
        vsPlotPath, plotName + "_from_all", "source protocol", lambda x: x.protocol.split(' ')[0],
        lambda x: x.protocol.split(' ')[2], "target protocol")
    plotGroupedVSBoxPlot(
        ListUtils.sortbymultiple(metadata, [lambda x: x.protocol.split(' ')[2], lambda x: x.protocol.split(' ')[0]]),
        vsPlotPath, plotName + "_to_all", "target protocol", lambda x: x.protocol.split(' ')[2],
        lambda x: x.protocol.split(' ')[0], "source protocol")

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol.split(' ')[2])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=lambda x: x.protocol.split(' ')[0]), vsPlotPath,
                      plotName + "_to_" + protocol, "protocol conversion")

    groupedByProtocol = ListUtils.groupby(metadata, lambda meta: meta.protocol.split(' ')[0])
    for (protocol, metaList) in groupedByProtocol.iteritems():
        plotVSBoxPlot(sorted(metaList, key=lambda x: x.protocol.split(' ')[2]), vsPlotPath,
                      plotName + "_from_" + protocol, "protocol conversion")


def plotServerSingleHop_100(zipHandle, plotFolder):
    plotName = "100_ServerSingleHop"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    groupedByVariant = ListUtils.groupby(metadata, lambda meta: meta.vsVariant)
    for (variant, metaList) in groupedByVariant.iteritems():
        #groupedByHopName = ListUtils.groupby(metaList, lambda meta: meta.hopName)
        #for (hopName, metaList) in groupedByHopName.iteritems():
            groupedByBandwidth = ListUtils.groupby(metaList, lambda meta: meta.bandwidth)
            for (bandwidth, metaList) in groupedByBandwidth.iteritems():
                sortedData=sorted(metaList, key=lambda x: x.protocol)
                plotGroupedVSBoxPlot(sortedData, vsPlotPath,
                              variant+" "+bandwidth, "protocol", boxGroupFn=lambda x: x.hopName, boxGroupName="hop")
                plotVSScatterPlot(sortedData, vsPlotPath, variant+" "+bandwidth,boxGroupFn=lambda x: x.hopName, boxGroupName="hop",
                                  plotValName="throughput [Mbps]", plotValFn=lambda meta: meta.throughput, scaleValues=False)

def plotServerOnePath_101(zipHandle, plotFolder):
    plotName = "101_ServerOnePath"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    groupedByVariant = ListUtils.groupby(metadata, lambda meta: meta.vsVariant)
    for (variant, metaList) in groupedByVariant.iteritems():
        groupedByHopName = ListUtils.groupby(metaList, lambda meta: meta.hopName)
        for (hopName, metaList) in groupedByHopName.iteritems():
            groupedByBandwidth = ListUtils.groupby(metaList, lambda meta: meta.bandwidth)
            for (bandwidth, metaList) in groupedByBandwidth.iteritems():
                sortedData=sorted(metaList, key=lambda x: x.protocol)
                plotGroupedVSBoxPlot(sortedData, vsPlotPath,
                              variant+" "+hopName+" "+bandwidth, "protocol", boxGroupFn=lambda meta: meta.scheduler, boxGroupName="stack scheduler")
                plotVSScatterPlot(sortedData, vsPlotPath, variant+" "+hopName+" "+bandwidth, boxGroupFn=lambda meta: meta.scheduler, boxGroupName="stack scheduler",
                                  plotValName="throughput [Mbps]", plotValFn=lambda meta: meta.throughput, scaleValues=False)

                excluded = [1, 2, 5, 7, 8]

                metaList = filter(lambda meta: meta.controlPoint not in excluded, metaList)

                groupedByProtocol = ListUtils.groupby(metaList, lambda meta: meta.protocol)
                data = []
                for (protocol, metaList) in groupedByProtocol.iteritems():
                    metaList = sorted(metaList, key=lambda meta: meta.controlPoint)
                    maxBranches = max(metaList, key=lambda meta: meta.branch).branch
                    maxPoints = max(metaList, key=lambda meta: meta.controlPoint).controlPoint
                    for branch in range(0, maxBranches + 1):
                        curBranch = ()
                        for cp in range(1, maxPoints + 1):
                            if cp in excluded:
                                continue
                            filtered = filter(lambda meta: meta.controlPoint == cp and meta.branch <= branch, metaList)
                            curBranch += (max(filtered, key=lambda meta: meta.branch),)
                        data += [curBranch]
                data = sorted(data, key=lambda x: x[-1].latMedian)

                #groupedByProtocol = ListUtils.groupby(data, lambda meta: meta[0].protocol.split("-")[-1])
                #for (bbProtocol, metaList) in groupedByProtocol.iteritems():
                metaList=data
                values = map(lambda x: tuple(map(lambda meta: meta.latMedian / 1000000.0, x)), metaList)
                values = map(lambda x: tuple(deAccumulate(x)), values)
                #labels = map(lambda metaTup: metaTup[0].protocol.split(" ")[:2], metaList)
                labels = map(lambda metaTup: metaTup[0].protocol, metaList)

                legend = [  # zusammenlegen: 1-3, 4, 5-6, 7-9
                    "send over LTE/DSL",
                    "merge flows",
                    "move merged flow through INVS",
                    "send over backbone",
                ]

                colors = sns.color_palette("Set3", 20)
                toRemove = colors[1]
                colors = filter(lambda x: x != toRemove, colors)
                #plotter = StackedBarPlotter("latency composition" + "_" + bbProtocol, "xAxis", "yAxis")
                plotter = StackedBarPlotter("latency composition_"+hopName, "xAxis", "yAxis")
                plotter.plot(vsPlotPath, values, colors, legend, labels)


def plotServerFullNetwork_102(zipHandle, plotFolder):
    plotName = "102_ServerFullNetwork"
    vsPlotPath, metadata = prepareMAPlot(zipHandle, plotName, plotFolder)
    groupedByVariant = ListUtils.groupby(metadata, lambda meta: meta.vsVariant)
    for (variant, metaList) in groupedByVariant.iteritems():
        groupedByBandwidth = ListUtils.groupby(metaList, lambda meta: meta.bandwidth)
        for (bandwidth, metaList) in groupedByBandwidth.iteritems():
            sortedData=sorted(metaList, key=lambda x: x.protocol)
            plotGroupedVSBoxPlot(sortedData, vsPlotPath,
                                 variant+" "+bandwidth, "protocol", boxGroupFn=lambda meta: meta.scheduler, boxGroupName="stack scheduler")
            #plotVSLinePlot(sortedData, vsPlotPath, variant+" "+bandwidth, "protocol",lambda meta: meta.protocol, lambda meta: meta.protocol,
            #               groupByFn=lambda meta: meta.scheduler, groupName="stack scheduler",
            #               yValName="throughput [Mbps]", yValFn=lambda meta: meta.throughput)
            plotVSScatterPlot(sortedData, vsPlotPath, variant+" "+bandwidth, boxGroupFn=lambda meta: meta.scheduler, boxGroupName="stack scheduler",
                              plotValName="throughput [Mbps]", plotValFn=lambda meta: meta.throughput, scaleValues=False)

            excluded = [1, 2, 5, 7, 8]

            metaList = filter(lambda meta: meta.controlPoint not in excluded, metaList)

            groupedByProtocol = ListUtils.groupby(metaList, lambda meta: meta.protocol)
            data = []
            for (protocol, metaList) in groupedByProtocol.iteritems():
                metaList = sorted(metaList, key=lambda meta: meta.controlPoint)
                maxBranches = max(metaList, key=lambda meta: meta.branch).branch
                maxPoints = max(metaList, key=lambda meta: meta.controlPoint).controlPoint
                for branch in range(0, maxBranches + 1):
                    curBranch = ()
                    for cp in range(1, maxPoints + 1):
                        if cp in excluded:
                            continue
                        filtered = filter(lambda meta: meta.controlPoint == cp and meta.branch <= branch, metaList)
                        curBranch += (max(filtered, key=lambda meta: meta.branch),)
                    data += [curBranch]
            data = sorted(data, key=lambda x: x[-1].latMedian)

            #groupedByProtocol = ListUtils.groupby(data, lambda meta: meta[0].protocol.split("-")[-1])
            #for (bbProtocol, metaList) in groupedByProtocol.iteritems():
            metaList=data
            values = map(lambda x: tuple(map(lambda meta: meta.latMedian / 1000000.0, x)), metaList)
            values = map(lambda x: tuple(deAccumulate(x)), values)
            #labels = map(lambda metaTup: metaTup[0].protocol.split(" ")[:2], metaList)
            labels = map(lambda metaTup: metaTup[0].protocol, metaList)

            legend = [  # zusammenlegen: 1-3, 4, 5-6, 7-9
                "send over LTE/DSL",
                "merge flows",
                "move merged flow through INVS",
                "send over backbone",
            ]

            colors = sns.color_palette("Set3", 20)
            toRemove = colors[1]
            colors = filter(lambda x: x != toRemove, colors)
            #plotter = StackedBarPlotter("latency composition" + "_" + bbProtocol, "xAxis", "yAxis")
            plotter = StackedBarPlotter("latency composition", "xAxis", "yAxis")
            plotter.plot(vsPlotPath, values, colors, legend, labels)


def plotPostMABarGraph(zipHandle, plotFolder):
    plotName = "102_PostMABarGraph"
    vsPlotPath = prepareLegacyPlot(plotName, plotFolder)

    ##### DATA

    ## LATENCY
    configured = [20, 30, 1] #LTE, DSL, backbone
    rawDelay = sum(configured)

    tcpMouseDelay = rawDelay - 2

    vsMouseDelays = [tcpMouseDelay - 5, tcpMouseDelay - 10, tcpMouseDelay - 12]
    vsMouseVSDelays = [10,8,15] #vs...VS = delay created by VS

    invsMouseDelay = tcpMouseDelay - 15
    invsMouseVSDelay = 5

    tcpElephantDelay = rawDelay + 10

    vsElephantDelays = [tcpElephantDelay - 5, tcpElephantDelay - 10, tcpElephantDelay - 12]
    vsElephantVSDelays = [10,8,15] #vs...VS = delay created by VS
    invsElephantDelay = tcpElephantDelay - 15
    invsElephantVSDelay = 5

    ## CON. EST. TIME
    tcpCET  = 100
    vsCET = [2*tcpCET, 2*tcpCET * 1.2, 2*tcpCET * 1.5]
    invsCET = 2*vsCET[0]

    ## THROUGHPUT
    rawThroughput = 25
    tcpThroughput = 20
    vsThroughput = [18,22,10]
    invsThroughput = 23

    ##### COLORS

    rawColor = "#222222"
    vsColors = ["#00AAAA", "#AA00AA", "#AAAA00"] #tcp, sctp, udp+
    tcpColor = vsColors[0]
    vsVSColor = "#FF0000"
    invsColor = "#CCCCCC"

    ##### DRAW LATENCY GRAPH

    import numpy as np

    ind = np.arange(9)  # the x locations for the groups
    width = 1./3.  # the width of the bars

    fig, ax = createPlot()

    raw = ax.bar(0, rawDelay, width, color=rawColor, label='RAW')

    tcpMouse = ax.bar(2, tcpMouseDelay, width, color=tcpColor, label='TCP')

    vsIdx = 3
    vsMouseTCP = ax.bar(vsIdx - width, vsMouseDelays[0], width, color=vsColors[0]) #label already created by tcp mouse
    vsMouseSCTP = ax.bar(vsIdx, vsMouseDelays[1], width, color=vsColors[1], label="SCTP")
    vsMouseUDPP = ax.bar(vsIdx + width, vsMouseDelays[2], width, color=vsColors[2], label="UDP+")
    vsMouseVS = ax.bar([vsIdx - width,vsIdx, vsIdx + width], vsMouseVSDelays, width, color=vsVSColor, label='VS overhead')


    invsMouse = ax.bar(4, invsMouseDelay, width, color=invsColor, label='INVS')
    invsMouseVS = ax.bar(4, invsMouseVSDelay, width, color=vsVSColor)

    tcpElephant = ax.bar(6, tcpElephantDelay, width, color=tcpColor)

    vsIdx = 7
    vsElephantTCP = ax.bar(vsIdx - width, vsElephantDelays[0], width, color=vsColors[0])
    vsElephantSCTP = ax.bar(vsIdx, vsElephantDelays[1], width, color=vsColors[1])
    vsElephantUDPP = ax.bar(vsIdx + width, vsElephantDelays[2], width, color=vsColors[2])
    vsElephantVS = ax.bar([vsIdx - width,vsIdx, vsIdx + width], vsElephantVSDelays, width, color=vsVSColor)

    invsElephant = ax.bar(8, invsElephantDelay, width, color=invsColor)
    invsElephantVS = ax.bar(8, invsElephantVSDelay, width, color=vsVSColor)

    ax.set_ylabel('latency [ms]')
    #ax.set_title('Title')
    ax.set_xticks([0,2,3,4,6,7,8])
    ax.set_xticklabels(('raw', 'tcp', 'vs', 'invs', 'tcp', 'vs', 'invs'))
    ax.legend()

    savePlot(ax, fig, vsPlotPath, "latency", "Legend title", aspectRatio=1.25, sortLegend=False)


    ##### DRAW CON. EST. TIME. GRAPH

    fig, ax = createPlot()

    tcpCETG = ax.bar(0, tcpCET, width, color=tcpColor)

    vsIdx = 1
    vsCETG = ax.bar([vsIdx - width,vsIdx, vsIdx + width], vsCET, width)
    map(lambda (idx, item): item.set_color(vsColors[idx]), enumerate(vsCETG))

    invsCETG = ax.bar(2, invsCET, width, color=invsColor)

    ax.set_ylabel('con. est. time [ms]')
    #ax.set_title('Title')
    ax.set_xticks([0,1,2])
    ax.set_xticklabels(('tcp', 'vs', 'invs'))
    #ax.legend()

    savePlot(ax, fig, vsPlotPath, "con_est_time", "Legend title", aspectRatio=0.625)


    ##### DRAW THROUGHPUT

    fig, ax = createPlot()

    rawThroughputG = ax.bar(0, rawThroughput, width, color=rawColor)
    tcpThroughputG = ax.bar(1, tcpThroughput, width, color=tcpColor)

    vsIdx = 2
    vsThroughputG = ax.bar([vsIdx - width,vsIdx, vsIdx + width], vsThroughput, width)
    map(lambda (idx, item): item.set_color(vsColors[idx]), enumerate(vsThroughputG))

    invsThroughputG = ax.bar(3, invsThroughput, width, color=invsColor)

    ax.set_ylabel('throughput [Mbps]')
    #ax.set_title('Title')
    ax.set_xticks([0,1,2,3])
    ax.set_xticklabels(('raw','tcp', 'vs', 'invs'))
    #ax.legend()

    savePlot(ax, fig, vsPlotPath, "throughput", "Legend title", aspectRatio=0.625)

#    exit(0) #TODO: REMOVE ME

def main():
    mainStart = time.time()
    if len(sys.argv) != 3:
        print "main.py [simulation-zip] [plot-folder]"
        return
    plotFolder = sys.argv[2]

    basePath = sys.argv[1]
    zips = sorted(filter(lambda x: x.endswith(".zip"), os.listdir(basePath)), key=lambda x: x[:3])

    zipMap = [
        # ('1_', plotStateOfTheInternetReport),
        # ('1_', statisticalMagick),
        # ('server_1_', plotAppToAppLoopback_1),
        # ('1_', plotAppToAppLoopback_1_PC),
        # ('2_', plotRawVariableBW_2),
        # ('3a_AppToApp', plotAppToApp_MouseFlow_3a),
        # ('3a_Router', plotRouter_MouseFlow_3a),
        # ('3b_AppToApp', plotAppToApp_ElephantFlow_3b),
        # ('3b_Router', plotRouter_ElephantFlow_3b),
        # ('4_', plotAppToAppRisingPayloadSize_4),
        # ('5_', plotRisingSendStackSize_5_PC),
        # ('server_5_', plotRisingSendStackSize_5),
        # ('6_', plotCPULimit_AppToApp_Router_TCP_6),
        # ('7a_', plotNS3Raw_7a),
        # ('7b_NS3-RawMouseFlow', plotNS3RawMouseFlow_7b),
        # ('7b_NS3-RawElephantFlow', plotNS3RawElephantFlow_7b),
        # ('7c_', plotNS3VS_7c),
        # ('7d_NS3-VSMouseFlow', plotNS3VSMouseFlow_7d),
        # ('7d_NS3-VSElephantFlow', plotNS3VSElephantFlow_7d),
        # ('8a_', plotNS3Simulated_8a),
        # ('10_', plotNS3FinalNetworkProtocolEvaluation_10),
        # ('11_', plotNS3FinalNetworkStackedGraph_11),
        # ('998_', plotNS3LatencyCoefficient_998),
        # ('999_', plotNS3LatencyCoefficient_999),
        #('100_', plotAppToApp_Combined_100),
        #('101_', plotAppToApp_AllLatency_101)
        #('101_', plotPostMABarGraph) #TODO: fix file prefix
        ('100_', plotServerSingleHop_100),
        ('101_', plotServerOnePath_101),
        ('102_', plotServerFullNetwork_102),
    ]

    legacyZipMap = [
        ('Legacy_LTE_', legacyPlotLatency)
    ]

    vsPlotPath = os.path.join(plotFolder, "vs")

    if not os.path.exists(vsPlotPath):
        os.mkdir(vsPlotPath)

    for name in zips:
        for fn in (x[1] for x in zipMap if name.startswith(x[0])):
            if fn is None:
                continue

            sys.stdout.write("Plotting " + name)
            sys.stdout.flush()
            start = time.time()

            with zipfile.ZipFile(os.path.join(basePath, name)) as zipHandle:
                fn(zipHandle, vsPlotPath)

            print ' ' * (50 - len(name)) + "Finished in " + str(time.time() - start) + " seconds"

    # print "Plotting legacy files"
    # path = basePath + "lte"
    # for name in zips:
    #     print "Reading from " + name
    #     with zipfile.ZipFile(os.path.join(basePath, name)) as zipHandle:
    #         metadata = map(lambda x: parseLegacyFile(x, zipHandle), filter(lambda x: x.endswith(".csv"), zipHandle.namelist()))
    #         metadata = filter(lambda x: x is not None, metadata)
    #     for fn in (x[1] for x in legacyZipMap if name.startswith(x[0])):
    #         if fn is None:
    #             continue
    #
    #         sys.stdout.write("Plotting " + name)
    #         sys.stdout.flush()
    #         start = time.time()
    #         fn(metadata, vsPlotPath)
    #         print ' ' * (50 - len(name)) + "Finished in " + str(time.time() - start) + " seconds"

    print "Cropping pdfs"
    cropStart = time.time()
    filesToCrop = []
    for path in filter(os.path.isdir, map(lambda x: os.path.join(vsPlotPath, x), os.listdir(vsPlotPath))):
        filesToCrop += map(lambda filename: os.path.join(vsPlotPath, path, filename), os.listdir(path))
    toCropCount = len(filesToCrop)
    with open(os.devnull, 'w') as devNull:
        for idx, file in enumerate(filesToCrop):
            subprocess.check_call(["pdfcrop", file, file], stdout=devNull, stderr=subprocess.STDOUT)
            sys.stdout.write("\rCropped %d/%d" % (idx + 1, toCropCount))
            sys.stdout.flush()
    print " in " + str(time.time() - cropStart) + " seconds"
    print "Finished in " + str(time.time() - mainStart) + " seconds"


if __name__ == "__main__":
    main()
