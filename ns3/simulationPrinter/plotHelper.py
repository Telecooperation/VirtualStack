import os

import matplotlib
import pandas as pd
from matplotlib.backends.backend_pdf import PdfPages

import matplotlib.pyplot as plt

import seaborn as sns

from metadata import Metadata
from plotter.boxPlotter import BoxPlotter
from plotter.multiLinePlotter import MultiLinePlotter
from plotter.scatterPlotter import ScatterPlotter
from utils.listUtils import ListUtils


def tryConvert(x):
    try:
        return float(x)
    except ValueError:
        return x

def createPlot():
    #configure seaborn
    sns.set(font_scale=0.8)
    sns.set_style("ticks", {'legend.frameon': True})
    sns.despine()

    fig, ax = plt.subplots()

    #configure grid
    ax.get_yaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax.grid(b=True, which='major', color='#cccccc', linewidth=0.5, linestyle='-')
    ax.grid(b=True, which='minor', color='#dddddd', linewidth=0.5, linestyle=':')
    ax.xaxis.grid(False)
    ax.set_xticklabels(ax.get_xticklabels(),rotation=20,ha="right")

    return fig, ax


def savePlot(ax, fig, path, title, legendTitle=None, sortLegend=True, log=False, aspectRatio = 2.5):

    if not log: #set aspect ratio; not supported for logarithmic plots
        ax.set_aspect(1.0/ax.get_data_ratio()*(1/aspectRatio))

    xticklabels = map(lambda x: x._text, ax.get_xticklabels())
    if "3.0" in  xticklabels:
        ax.set_xticklabels(ax.get_xticklabels(),rotation=45,ha="right")

    #move legend
    handles, labels = ax.get_legend_handles_labels()
    ldg = None
    if len(labels) > 0:
        if sortLegend:
            labels, handles = zip(*sorted(zip(labels, handles), key=lambda t: tryConvert(t[0])))
        ldg = ax.legend(handles, labels, bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0., title=legendTitle)#loc='best')
        fig.artists.append(ldg)

    savePath = os.path.join(path, title.replace(" ", "_").replace(".00", "").replace("_[ns]","").replace("_[us]","").replace("_[ms]","").replace("_[s]","") + ".pdf")
    with PdfPages(savePath) as pdf:
        pdf.savefig(fig, additional_artists=ldg, bbox_inches="tight")

    plt.close(fig)


def splitVSBoxPlot(metadata, vsPlotPath, resultName, boxNameFn = lambda x: x.protocol):
    fastMetadata = filter(lambda x: x.latMedian <= 100000, metadata)
    slowMetadata = [item for item in metadata if item not in fastMetadata]

    if len(slowMetadata) == 0:
        plotVSBoxPlot(fastMetadata, vsPlotPath, resultName, boxNameFn)
    else:
        plotVSBoxPlot(fastMetadata, vsPlotPath, resultName + "_fast", boxNameFn)
        plotVSBoxPlot(slowMetadata, vsPlotPath, resultName + "_slow", boxNameFn)


def getScalingFactor(lst, plotValName):
    if "[ns]" not in plotValName:
        print "Can only scale values in ns!"
        return (1, None)

    for factor in [(1e6, "[ms]"), (1e3, "[us]")]:#(1e9, "[s]"),
        if sum(map(lambda x: 1 if x > factor[0] else 0, lst)) > 0:#len(lst)/2:
            return factor
    return (1, "[ns]")

def plotGroupedVSBoxPlot(metadata, vsPlotPath, resultName, xAxisText="protocol", boxNameFn = lambda x: x.protocol, boxGroupFn = Metadata.bandwidthNameFn, boxGroupName="bandwidth [Mbps]", plotValName = "latency [ns]", plotValFn = lambda meta: meta.getQuantileData(), log=False):
    if len(metadata) == 0:
        return

    results = []

    medians = []
    for meta in metadata:
        values = plotValFn(meta)
        results += map(lambda x: (boxNameFn(meta), long(x), boxGroupFn(meta)), values)
        medians.append(float(values[len(values) / 2]))

    scalingFactor, unit = getScalingFactor(medians, plotValName)
    plotValName = plotValName[:-4] + unit

    results = [('Protocol', plotValName, boxGroupName)] + map(lambda x: (x[0], x[1] / scalingFactor, x[2]), results[1:])
    #results += filter(lambda x: x[0] == "UDP-Plus" and x[2] == "SCTP", results)

    dataFrame =  pd.DataFrame(results[1:], columns=results[0])
    plotter = BoxPlotter(resultName + " " + plotValName, xAxisText,plotValName)
    plotter.plot(vsPlotPath, dataFrame, results[0][0], results[0][1], hue=results[0][2], log=log)

def plotVSBoxPlot(metadata, vsPlotPath, resultName, xAxisCaption="protocol", boxNameFn = lambda x: x.protocol, plotValName = "latency [ns]", plotValFn = lambda meta: meta.getQuantileData()):
    if len(metadata) == 0:
        return

    results = []

    medians = []
    for meta in metadata:
        values = plotValFn(meta)
        results += map(lambda x: (boxNameFn(meta), long(x)), values)
        medians.append(float(values[len(values) / 2]))

    scalingFactor, unit = getScalingFactor(medians, plotValName)
    plotValName = plotValName[:-4] + unit

    results = [('Protocol', plotValName)] + map(lambda x: (x[0], x[1] / scalingFactor), results[1:])

    dataFrame =  pd.DataFrame(results[1:], columns=results[0])
    plotter = BoxPlotter(resultName + " " + plotValName, xAxisCaption,plotValName)
    plotter.plot(vsPlotPath, dataFrame, results[0][0], results[0][1])

def is_sequence(arg):
    return (not hasattr(arg, "strip") and
            hasattr(arg, "__getitem__") or
            hasattr(arg, "__iter__"))

def plotVSScatterPlot(metadata, vsPlotPath, resultName, xAxisCaption="protocol", xValFn = lambda x: x.protocol, boxGroupFn = Metadata.bandwidthNameFn, boxGroupName="bandwidth [Mbps]", plotValName ="latency [ns]", plotValFn = lambda meta: meta.getQuantileData(), scaleValues=True):
    if len(metadata) == 0:
        return

    results = []

    #medians = []
    for meta in metadata:
        values = plotValFn(meta)
        xVal = xValFn(meta)
        group=boxGroupFn(meta)
        if is_sequence(values):
            results += map(lambda x: (xVal, float(x), group), values)
        else:
            results += [(xVal, float(values), group)]
        #medians.append(float(values[len(values) / 2]))

    if scaleValues:
        scalingFactor, unit = getScalingFactor(results, plotValName)
        if scalingFactor != 1:
            plotValName = plotValName[:-4] + unit
            results = map(lambda x: (x[0], x[1] / scalingFactor, x[2]), results)

    results = [(xAxisCaption, plotValName, boxGroupName)] + results

    dataFrame =  pd.DataFrame(results[1:], columns=results[0])
    plotter = ScatterPlotter(resultName + " " + plotValName, xAxisCaption,plotValName)
    plotter.plot(vsPlotPath, dataFrame, results[0][0], results[0][1], results[0][2])


def plotVSLinePlot(metadata, vsPlotPath, resultName, xValName, xValFn, xValNameFn, groupName = "protocol", groupByFn=lambda meta: meta.protocol, yValName = "latency [ns]", yValFn = lambda meta: meta.getQuantileData(), yLim=None, xLog=False, xLabelSortHack=False):
    if len(metadata) == 0:
        return

    grouped = ListUtils.groupby(metadata, groupByFn)
    multiLineData = []
    xLabels = []
    xLabelSet = set()
    for (group, items) in grouped.iteritems():
        results = []
        results += map(lambda meta: (xValFn(meta), yValFn(meta)), items)

        xLabels = map(xValNameFn, items)
        xLabelSet |= set(xLabels)
        #print xLabelSet
        multiLineData += [(group, results)]

    if xLabelSortHack:
        xLabelSet = sorted(list(xLabelSet), key=lambda x: int(x[:-2]))
    #print xLabelSet
    #print xLabelSortHack
    #print "---"

    plotter = MultiLinePlotter(resultName + " " + yValName, xValName,yValName)
    plotter.plot(vsPlotPath, multiLineData, groupName, xLabels if not xLabelSortHack else xLabelSet, yLim=yLim, xLog=xLog)

def plotVSSurfacePlot(metadata, vsPlotPath, resultName, xValName, xValFn, xValNameFn, groupName = "protocol", groupByFn=lambda meta: meta.protocol, yValName = "latency [ns]", yValFn = lambda meta: meta.getQuantileData(), yLim=None):
    if len(metadata) == 0:
        return

    grouped = ListUtils.groupby(metadata, groupByFn)
    multiLineData = []
    xLabels = []
    for (group, items) in grouped.iteritems():
        results = []
        results += map(lambda meta: (xValFn(meta), yValFn(meta)), items)
        xLabels = map(xValNameFn, items)

        multiLineData += [(group, results)]

    plotter = MultiLinePlotter(resultName + " " + yValName, xValName,yValName)
    plotter.plot(vsPlotPath, multiLineData, groupName, xLabels, yLim=yLim)

def prepareMAPlot(zipHandle, plotName, plotFolder, filterNoLimit = False):
    vsPlotPath = os.path.join(plotFolder, plotName)

    if not os.path.exists(vsPlotPath):
        os.mkdir(vsPlotPath)

    metadata = map(lambda x: Metadata.fromFile(x, zipHandle), filter(lambda x: x.endswith(".vsmeta"), zipHandle.namelist()))
    failedData = filter(lambda x: x.packetsSent == 0, metadata)
    for failed in failedData:
        print "Failed to send one packet for: " + failed.simName
    metadata = filter(lambda x: x.packetsSent != 0, metadata)

    if filterNoLimit:
        metadata = filter(lambda x: x.bandwidth != "0.00Mbps" and x.bandwidth != "0Mbps", metadata)

    return vsPlotPath, metadata

def prepareLegacyPlot(plotName, plotFolder):
    vsPlotPath = os.path.join(plotFolder, plotName)

    if not os.path.exists(vsPlotPath):
        os.mkdir(vsPlotPath)
    return vsPlotPath