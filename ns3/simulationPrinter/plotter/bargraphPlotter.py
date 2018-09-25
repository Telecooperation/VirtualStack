import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import ScalarFormatter

import plotHelper


class BargraphPlotter:
    def __init__(self, title, xAxisText, yAxisText, fileName=None):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText
        self.fileName = fileName or self.title

    def plot(self, path, data, figSize=(40,10)):
        #data: [(name, val), (name, val)]

        fig, ax = plt.subplots(figsize=figSize)
        ax.set_title(self.title)
        ax.set_ylabel(self.yAxisText)
        ax.set_xlabel(self.xAxisText)

        N = len(data)

        ind = np.arange(N)  # the x locations for the groups
        width = 0.35  # the width of the bars

        labels = [x[0] for x in data]
        yVal = [x[1] for x in data]
        yError = [x[2] if len(x) > 2 else 0 for x in data]

        newWidth = (width + 0.1)
        xPos = ind * newWidth
        labelPos = xPos

        ax.bar(xPos, yVal, width, yerr=yError)

        ax.set_xticks(labelPos)
        ax.set_xticklabels(labels, rotation=20, ha="right", )
        #ax.xaxis.set_major_formatter(ScalarFormatter())
        plt.minorticks_on()
        ax.yaxis.set_major_formatter(ScalarFormatter())
        plt.grid(True, which='major', linestyle='--',color='gray')
        plt.grid(True, which='minor', linestyle='--',color='lightgray')
        ax.set_axisbelow(True)

        plt.tight_layout()
        plotHelper.savePlot(path, self.fileName, fig)
