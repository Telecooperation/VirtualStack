import os

import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.backends.backend_pdf import PdfPages
from numpy import linspace


class DiscreteHeatMapPlotter:
    def __init__(self, title, fileName=None):
        self.title = title
        self.fileName = fileName or self.title

    def plot(self, path, data, label, nameMap, captions=None):

        fig, ax = plt.subplots()

        # For only three colors, it's easier to choose them yourself.
        # If you still really want to generate a colormap with cubehelix_palette instead,
        # add a cbar_kws={"boundaries": linspace(-1, 1, 4)} to the heatmap invocation
        # to have it generate a discrete colorbar instead of a continous one.

        #sns.heatmap(data, annot=True, ax=ax, cmap = sns.color_palette("cubehelix", n_colors=len(nameMap)),cbar_kws={"orientation": "horizontal"}, linewidths=.5, linecolor='lightgray', cbar=True, vmin=0, vmax=len(nameMap)-1)
        sns.heatmap(data, ax=ax, annot=captions, fmt=".1f", cmap = sns.color_palette("cubehelix", n_colors=len(nameMap)),cbar_kws={"orientation": "horizontal"}, linewidths=.5, linecolor='lightgray', vmin=0, vmax=len(nameMap)-1, cbar=True)

        # Manually specify colorbar labelling after it's been generated
        lspace = linspace(0, len(nameMap)-1, len(nameMap))
        lspace2 = linspace(0, len(nameMap)-1, 2*len(nameMap)+1)[1::2]

        colorbar = ax.collections[0].colorbar
        colorbar.set_ticks(lspace2)
        colorbar.set_ticklabels(map(lambda x: nameMap[x], lspace))

        # X - Y axis labels
        #ax.set_ylabel('FROM')
        #ax.set_xlabel('TO')

        ax.invert_yaxis()

        # Only y-axis labels need their rotation set, x-axis labels already have a rotation of 0
        _, labels = plt.yticks()
        plt.setp(labels, rotation=0)

        #plt.show()

        #plt.tight_layout()
        savePath = os.path.join(path, self.fileName + ".pdf")
        with PdfPages(savePath) as pdf:
            pdf.savefig(fig)

        plt.close(fig)
