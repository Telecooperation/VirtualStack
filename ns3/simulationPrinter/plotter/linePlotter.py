import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

import plotHelper


class LinePlotter:
    def __init__(self, title, xAxisText, yAxisText):
        self.title = title
        self.xAxisText = xAxisText
        self.yAxisText = yAxisText

    def plot(self, path, data):
        #data = [(x,y),..]
        fig, ax = plt.subplots()
        ax.set_title(self.title)
        ax.set_ylabel(self.yAxisText)
        ax.set_xlabel(self.xAxisText)

        xValues = [x[0] for x in data]
        yValues = [x[1] for x in data]

        ax.plot(xValues, yValues, "-o")
        plt.legend()
        plt.minorticks_on()
        ax.xaxis.set_major_formatter(ScalarFormatter())
        ax.yaxis.set_major_formatter(ScalarFormatter())
        plt.grid(True, which='major', linestyle='--',color='gray')
        plt.grid(True, which='minor', linestyle='--',color='lightgray')
        ax.set_axisbelow(True)

        plotHelper.savePlot(path, self.title, fig)
