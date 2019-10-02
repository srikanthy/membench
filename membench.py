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

def main(infile):

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

    # create line plot
    fig = plt.figure()
    ax = fig.add_subplot(111)

    for key, block in data.groupby("size"):
      plt.semilogx(block['stride'], block['time'], label = bytes_to_string(key), basex = 2, nonposx = 'mask', linewidth = 0.6, linestyle = '-', marker = '.', markersize = 1.8)

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

    # show the plot on screen
#   plt.show()

    # save to pdf
    plt.savefig(outfile)

if __name__ == "__main__":
  main(*sys.argv[1:])
