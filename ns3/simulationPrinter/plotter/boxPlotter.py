import seaborn as sns

import plotHelper


class BoxPlotter:
    def __init__(self, title, xAxisText, yAxisText, fileName=None):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText
        self.fileName = fileName or self.title

    def plot(self, path, data, xName=None, yName=None, hue=None, log=False):

        fig, ax = plotHelper.createPlot()
        ax.set_ylabel(self.yAxisText)
        if log:
            ax.set_yscale("log")
        #huePalette = sns.color_palette("viridis", desat=.5)
        colors = sns.color_palette("Set3", 20)
        toRemove = colors[1]
        colors = filter(lambda x: x != toRemove, colors)
        sns.boxplot(ax=ax,x=xName, y=yName, data=data, showfliers=False,linewidth=0.7, hue=hue, palette=colors)#palette="binary" if hue is None else huePalette)
        ax.set_xlabel(self.xAxisText)

        if hue is None:
            for box in ax.artists:
                box.set_facecolor("white")

        plotHelper.savePlot(ax, fig, path, self.title, hue, sortLegend=False, log=log)
