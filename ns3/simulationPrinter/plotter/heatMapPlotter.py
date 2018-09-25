import seaborn as sns
from matplotlib.colors import ListedColormap

import plotHelper


class HeatMapPlotter:
    def __init__(self, title, fileName=None):
        self.title = title
        self.fileName = fileName or self.title

    def plot(self, path, data, label, centerOn=None, comparision=False, captions=None, nonNumeric=False):

        fig, ax = plotHelper.createPlot()
        ax.set_title(self.title)

        if nonNumeric:
            cmap = ListedColormap(sns.cubehelix_palette(start=2.8, rot=.1, light=0.9, n_colors=3))
        else:
            cmap = "viridis"

        if comparision:
            sns.heatmap(data, ax=ax, annot=True, fmt=".1f", cmap=cmap, center=centerOn, vmin=0, vmax=2, cbar=not nonNumeric)
        else:
            if captions is not None:
                sns.heatmap(data, ax=ax, annot=captions, fmt="", cmap=cmap, center=centerOn, cbar=not nonNumeric)
            else:
                sns.heatmap(data, ax=ax, annot=True, fmt=".1f", cmap=cmap, center=centerOn, cbar=not nonNumeric)

        if not nonNumeric:
            cbar = ax.collections[0].colorbar
            cbar.set_label(label)
        ax.invert_yaxis()

        plotHelper.savePlot(ax, fig, path, self.title)
