import os

import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.backends.backend_pdf import PdfPages
from numpy import linspace


class DiscreteDoubleHeatMapPlotter:
    def __init__(self, title, fileName=None):
        self.title = title
        self.fileName = fileName or self.title

    def plot(self, path, title_left, title_right, data_left, data_right, label, nameMap, captions_left=None, captions_right=None, format=".1f"):
        sns.set(font_scale=0.55)
        sns.set_style("ticks")
        fig, (ax_left, ax_right) = plt.subplots(1,2,sharex=True, sharey=True)
        #ax_left.set_title(title_left)
        #ax_right.set_title(title_right)
        #fig.suptitle(label, fontsize=16)

        lspace = linspace(0, len(nameMap)-1, len(nameMap))
        lspace2 = linspace(0, len(nameMap)-1, 2*len(nameMap)+1)[1::2]

        im = sns.heatmap(data_left, ax=ax_left, annot=captions_left, fmt=format, cmap = sns.color_palette("cubehelix", n_colors=len(nameMap)), linewidths=.5, linecolor='lightgray', vmin=0, vmax=len(nameMap)-1, cbar=False)
        ax_left.invert_yaxis()
        ax_left.grid()

        sns.heatmap(data_right, ax=ax_right, annot=captions_right, fmt=format, cmap = sns.color_palette("cubehelix", n_colors=len(nameMap)), linewidths=.5, linecolor='lightgray', vmin=0, vmax=len(nameMap)-1, cbar=False)
        ax_right.invert_yaxis()
        ax_right.get_yaxis().set_visible(False)
        ax_right.grid()

        mappable = im.get_children()[0]
        colorbar = plt.colorbar(mappable, ax = [ax_left,ax_right],location='top')

        #colorbar = ax_left.collections[0].colorbar
        colorbar.set_ticks(lspace2)
        colorbar.set_ticklabels(map(lambda x: nameMap[x], lspace))

        # X - Y axis labels
        #ax.set_ylabel('FROM')
        #ax.set_xlabel('TO')

        #plt.tight_layout()
        savePath = os.path.join(path, self.fileName + ".pdf")
        with PdfPages(savePath) as pdf:
            pdf.savefig(fig)

        plt.close(fig)
