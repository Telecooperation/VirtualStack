import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

import plotHelper
import seaborn as sns

class MultiLinePlotter:
    def __init__(self, title, xAxisText, yAxisText):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText

    def plot(self, path, dataArray, legendTitle, xLables=None, xLog = False, yLim=None):
        #print xLables
        #print "!!!!!"
        fig, ax = plotHelper.createPlot()
        ax.set_ylabel(self.yAxisText)
        ax.set_xlabel(self.xAxisText)
        if xLog:
            ax.set_xscale('log')

        filledMarkers = list(Line2D.filled_markers)
        xValues = []
        xValueSet = set()
        colors = sns.color_palette("Set3", 20)
        toRemove = colors[1]
        colors = filter(lambda x: x != toRemove, colors)
        for i, el in enumerate(dataArray):
            label = el[0]
            data = el[1]

            xValues = [x[0] for x in data]
            xValueSet |= set(xValues)
            yValues = [x[1] for x in data]

            ax.plot(xValues, yValues, marker=filledMarkers[i], label=label, color=colors[i])
        if xLables is not None:
            plt.xticks(sorted(list(xValueSet)), xLables)
        if yLim is not None:
            #lim = ax.get_ylim()
            #ax.set_ylim([lim[0] * yLim[0], lim[1] * yLim[1]])
            ax.set_ylim(yLim)
        plotHelper.savePlot(ax, fig, path, self.title, legendTitle)

