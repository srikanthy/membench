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

    # define filenames
    bname = os.path.splitext(infile)[0]
    outfile = bname + ".pdf"

    # load data
    data = pd.read_csv(infile)

    # cleanup data, remove negative values
    data[data < 0] = np.nan

    # create line plot
    fig = plt.figure()
    ax = fig.add_subplot(111)
    linestyles = [ "solid", "dashed", "dashdot" ]

    i = 0
    for key, block in data.groupby("size"):
      plt.semilogx(block['stride'], block['time'], label = bytes_to_string(key), basex = 2, nonposx = 'mask', linewidth = 0.6, linestyle = linestyles[i%3], marker = '.', markersize = 1.8)
      i += 1

    # define xticks
    for key, block in data.groupby("size"):
      max_stride = max(block['stride'])
      min_stride = min(block['stride'])

    nxticks = int(math.log(max_stride,2))
    xticks_labels = []
    xticks_locs = []
    for tid in range(0, nxticks, 3):
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
    plt.xlabel("stride", fontsize = 10, fontweight = 'bold')
    plt.ylabel("time (ns)", fontsize = 10, fontweight = 'bold')

    # plot major gridlines
    plt.grid(b = True, axis= 'x', which = 'major')
    plt.grid(b = True, axis= 'y', which = 'major')
#   plt.grid(b = True, axis= 'x', which = 'minor', linestyle = '--')
#   plt.grid(b = True, axis= 'y', which = 'minor', linestyle = '--')

    # add legend
    legend = plt.legend(loc = 'upper right', fontsize = 4, framealpha = 1.0, frameon = False)

    # define cache values in kb
    l1_cache = 32
    l2_cache = 256
    l3_cache = 6 * 1024

    # add vertical marker lines for cache sizes
    l1_color = "green"
    l2_color = "blue"
    l3_color = "orange"
    offset_pxl = 75

    plt.axvline(l1_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l1_color)
    plt.axvline(l2_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l2_color)
    plt.axvline(l3_cache * 1024, linestyle = "dashed", linewidth = 0.8, color = l3_color)

    # calculate ypos based on number of pixels
    bbox = ax.get_window_extent().transformed(fig.dpi_scale_trans.inverted())
    y_pxl = bbox.height * fig.dpi
    y_hgt = ax.axes.get_ylim()[1]
    ypos = (y_pxl - offset_pxl) * y_hgt / y_pxl

    plt.text(l1_cache * 1024, ypos, 'L1 Cache = ' + str(bytes_to_string(l1_cache * 1024)), rotation = 90, fontsize = 5, color = l1_color)
    plt.text(l2_cache * 1024, ypos, 'L2 Cache = ' + str(bytes_to_string(l2_cache * 1024)), rotation = 90, fontsize = 5, color = l2_color)
    plt.text(l3_cache * 1024, ypos, 'L3 Cache = ' + str(bytes_to_string(l3_cache * 1024)), rotation = 90, fontsize = 5, color = l3_color)

    # add horizontal marker lines for min access time
    min_time = 99999999.0
    for key, block in data.groupby("time"):
      min_time = min(min_time, min(block['time']))

    xlims = ax.axes.get_xlim()
    xpos = xlims[1]
    plt.axhline(min_time, linestyle = "dashed", linewidth = 0.8, color = 'brown')
    plt.text(xpos * 1.2, min_time - 0.1, 'ns = ' + str(round(min_time,3)), rotation = 0, fontsize = 5, color = 'brown')

    # show the plot on screen
#   plt.show()

    # save to pdf
    plt.savefig(outfile)

if __name__ == "__main__":
  main(*sys.argv[1:])
