#!/usr/bin/env python3

"""
Generate line plots for the result of membench microbenchmark.

Author: Srikanth Yalavarthi
Last Modified : 2019-10-02
URL: https://github.com/srikanthy/membench.git

"""

# module imports
import sys
import os
import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

def bytes_to_string(int_bytes):

    """ Generate string based on size in bytes. """

    if (int_bytes < int(pow(2,10))):
      return "{0} B".format(int(int_bytes/pow(2,0)))
    if (int_bytes >= int(pow(2,10)) and int_bytes < int(pow(2,20))):
      return "{0} KB".format(int(int_bytes/pow(2,10)))
    if (int_bytes >= int(pow(2,20)) and int_bytes < int(pow(2,30))):
      return "{0} MB".format(int(int_bytes/pow(2,20)))
    if (int_bytes >= int(pow(2,30)) and int_bytes < int(pow(2,40))):
      return "{0} GB".format(int(int_bytes/pow(2,30)))

def main(infile = 'membench.csv'):

    """ 
    Program program to plot output of membench.

    Arguments:
        infile: input file (csv format)

    Output:
        Plot in pdf format

    """

    # define ouput filename
    bname = os.path.splitext(infile)[0]
    outfile = bname + ".pdf"

    # open pdf file
    pdf_pages = PdfPages(outfile)

    # load data
    data = pd.read_csv(infile)

    # cleanup data, remove negative values
    data[data < 0] = np.nan

    # loop over all output variables
    for column_name in data.columns[2:]:

      # define filenames
#     bname = os.path.splitext(infile)[0]
#     outfile = bname + "_" + column_name + ".pdf"

      # create line plot
      fig = plt.figure()
      ax = fig.add_subplot(111)
      linestyles = [ "solid", "dashed", "dashdot" ]

      i = 0
      for key, block in data.groupby(data.columns[0]):
        plt.semilogx(block[data.columns[1]], block[column_name], label = bytes_to_string(key), basex = 2, nonposx = 'mask', linewidth = 0.6, linestyle = linestyles[i%3], marker = '.', markersize = 1.8)
        i += 1

      # define xticks
      for key, block in data.groupby(data.columns[0]):
        max_stride = max(block[data.columns[1]])
        min_stride = min(block[data.columns[1]])

      nxticks = int(math.log(max_stride,2))

      ratio = nxticks/12;
      jump = math.floor(ratio)
      if ((ratio-math.floor(ratio)) > 0.5):
        jump += 1

      xticks_labels = []
      xticks_locs = []
      for tid in range(0, nxticks, jump):
        xticks_locs.append(int(pow(2,tid)))
        if (tid < 10):
            xticks_labels.append(str(int(pow(2,tid)/pow(2,0)))  +  " B")
        if (tid >= 10 and tid < 20):
            xticks_labels.append(str(int(pow(2,tid)/pow(2,10))) + " KB")
        if (tid >= 20 and tid < 30):
            xticks_labels.append(str(int(pow(2,tid)/pow(2,20))) + " MB")
        if (tid >= 30 and tid < 40):
            xticks_labels.append(str(int(pow(2,tid)/pow(2,30))) + " GB")

      # set axis limits
      plt.xlim(left = min_stride)
      plt.ylim(bottom = 0)

      # set custom ticks
      plt.minorticks_on()
      plt.xticks(xticks_locs, xticks_labels, rotation = 0, fontsize = 6, fontweight = 'demibold')
      plt.yticks(rotation = 0, fontsize = 6, fontweight = 'demibold')
      ax.tick_params(axis = "x", direction = "in")
      ax.tick_params(axis = "y", direction = "in")

      # add labels
      plt.xlabel("Stride", fontsize = 10, fontweight = 'bold')
      plt.ylabel(column_name, fontsize = 10, fontweight = 'bold')

      # plot major gridlines
      plt.grid(b = True, axis= 'x', which = 'major')
      plt.grid(b = True, axis= 'y', which = 'major')
#     plt.grid(b = True, axis= 'x', which = 'minor', linestyle = '--')
#     plt.grid(b = True, axis= 'y', which = 'minor', linestyle = '--')

      # add legend
      legend = plt.legend(loc = 'upper left', fontsize = 4, framealpha = 1.0, frameon = False)

      # define cache values in kb
      l1_cache = 32
      l2_cache = 256
      l3_cache = 6 * 1024

      # add vertical marker lines for cache sizes
      l1_color = "green"
      l2_color = "blue"
      l3_color = "orange"
      offset_pxl = 100
      offset_pts = 54

      if ( (l1_cache * 1024) < max_stride ):
        plt.axvline(l1_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l1_color)
      if ( (l2_cache * 1024) < max_stride ):
        plt.axvline(l2_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l2_color)
      if ( (l3_cache * 1024) < max_stride ):
        plt.axvline(l3_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l3_color)

      # calculate ypos based on number of pixels
      bbox = ax.get_window_extent().transformed(fig.dpi_scale_trans.inverted())
      y_hgt = ax.axes.get_ylim()[1]
      ypos = (1 - offset_pts/(72 * bbox.height)) * y_hgt

      if ( (l1_cache * 1024) < max_stride ):
        plt.text(l1_cache * 1024, ypos, 'L1 Cache = ' + str(bytes_to_string(l1_cache * 1024)), rotation = 90, fontsize = 5, color = l1_color)
      if ( (l2_cache * 1024) < max_stride ):
        plt.text(l2_cache * 1024, ypos, 'L2 Cache = ' + str(bytes_to_string(l2_cache * 1024)), rotation = 90, fontsize = 5, color = l2_color)
      if ( (l3_cache * 1024) < max_stride ):
        plt.text(l3_cache * 1024, ypos, 'L3 Cache = ' + str(bytes_to_string(l3_cache * 1024)), rotation = 90, fontsize = 5, color = l3_color)

      # add horizontal marker lines for min access val
      min_val = 99999999.0
      for key, block in data.groupby(column_name):
        min_val = min(min_val, min(block[column_name]))

      xlims = ax.axes.get_xlim()
      xpos = xlims[1]
      plt.axhline(min_val, linestyle = "dashed", linewidth = 0.8, color = 'brown')
      plt.text(xpos * 1.2, min_val - 0.1, 'ns = ' + str(round(min_val,3)), rotation = 0, fontsize = 5, color = 'brown')

      # show the plot on screen
#     plt.show()

      # save to pdf
#     plt.savefig(outfile)

      # save page
      pdf_pages.savefig(fig)

    # write pdf file to disk
    pdf_pages.close()

if __name__ == "__main__":
  main(*sys.argv[1:])
