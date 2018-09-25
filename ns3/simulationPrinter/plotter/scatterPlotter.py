import matplotlib.pyplot as plt
import seaborn as sns

import plotHelper


class ScatterPlotter:
    def __init__(self, title, xAxisText, yAxisText):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText

    def plot2d(self, path, data):
        fig, ax = plotHelper.createPlot()
        ax.set_ylabel(self.yAxisText)
        ax.set_xlabel(self.xAxisText)

        x = [val[0] for val in data]
        y = [val[1] for val in data]

        plt.scatter(x, y)

        plotHelper.savePlot(ax, fig, path, self.title)

    def plot(self, path, data, xName, yName, hueName):

        fig, ax = plotHelper.createPlot()
        ax.set_ylabel(self.yAxisText)
        ax.set_xlabel(self.xAxisText)

        colors = sns.color_palette("Set3", 20)
        toRemove = colors[1]
        colors = filter(lambda x: x != toRemove, colors)
        sns.barplot(xName, yName, hueName, data, ax=ax, ci=None, palette=colors)
        plotHelper.savePlot(ax, fig, path, self.title, hueName)

