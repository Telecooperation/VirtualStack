import os

import matplotlib

import plotHelper
from plotter.stackedBarGraph import StackedBarGrapher


class StackedBarPlotter:
    def __init__(self, title, xAxisText, yAxisText, fileName=None):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText
        self.fileName = fileName or self.title

    def plot(self, path, data, colors, legendNames, xLabels):
        fig, ax = plotHelper.createPlot()
        grapher = StackedBarGrapher()
        grapher.stackedBarPlot(ax, data, colors, xLabels, ylabel="latency [ms]", gap=0.4, yTicks=5)


        handles = map(lambda (color,label): matplotlib.patches.Patch(color=color, label=label),zip(colors, legendNames))


        ax.yaxis.grid(True)
        ax.legend(handles=handles, frameon = 1,bbox_to_anchor=(0., 1., 1., 0),
                  ncol=2, borderaxespad=0)
        fig.patch.set_visible(True)


        plotHelper.savePlot(ax, fig, path, self.title, "legend title")
