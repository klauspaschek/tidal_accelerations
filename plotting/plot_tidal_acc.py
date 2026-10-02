"""Bar charts of the tidal accelerations acting on an object over time (as in Figures 1-4 and 8-10 of the paper).

For each result file (one object), the time span is split into `split` parts, each plotted as one png file with
four panels: the individual tidal accelerations A caused by the other objects (logarithmic), their vector sum
A_total, the change Delta A_total between consecutive time steps, and A_total * Delta A_total / Delta t.
Every bar consists of three sub-bars for the minimal, mean, and maximal mass guess (from left to right).

usage: python3 plot_tidal_acc.py <input_directory_or_file> <output_directory> [--split N] [--jobs N]
"""
import argparse
import multiprocessing
import os
import sys
from decimal import Decimal

import matplotlib as mpl
mpl.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def mylogfmt(x, pos):
    logx = int(np.log10(x)) # to get the exponent

    return u"${:.0f}$".format(logx)

myLogFormatterExponent = mpl.ticker.FuncFormatter(mylogfmt)


def find_lower_pow(num):
    num_str = '%.10e' % Decimal(str(num))
    num_str = num_str[0] + num_str[num_str.index("e"):]
    return float(num_str)

def find_upper_pow(num):
    num_str = '%.10e' % Decimal(str(num))
    num_str = str(int(num_str[0]) + 1) + num_str[num_str.index("e"):]
    return float(num_str)


# get step width (in days) from filename, e.g. 'delta_t_0_25d_tidal_acc_...' -> 0.25
def step_width_from_filename(filename):
    step_width = filename[filename.index("delta_t_") + 8 : filename.index("d_tidal_acc")]
    return float(step_width.replace("_", "."))


def plot_file(path, output_directory, split=15, parts=None, output_filenames=None, yaxis_font_size=11):
    """Plot one result file. `parts` selects the plotted parts (1-based), `output_filenames` maps part -> filename."""
    filename = os.path.basename(path)
    delta_t = step_width_from_filename(filename)

    # prepare data as np arrays
    tidal_acc_str = np.genfromtxt(path, delimiter=',', dtype=str, unpack = True)

    time = tidal_acc_str[0][1:].astype(np.float64)

    tidal_acc = []
    for i in range(1, len(tidal_acc_str), 3):
        tidal_acc.append([tidal_acc_str[i][0], tidal_acc_str[i][1:].astype(np.float64), tidal_acc_str[i + 1][1:].astype(np.float64), tidal_acc_str[i + 2][1:].astype(np.float64)])

    tidal_acc.sort(key=lambda x: x[1][0], reverse=True)

    # plot
    width_bars = (time[1] - time[0]) / 6
    number_plot = 1
    created = []

    index_total_change = [i for i, element in enumerate(tidal_acc) if element[0] == "total_change"][0]
    tidal_acc[index_total_change][1][0] = 0
    tidal_acc[index_total_change][2][0] = 0
    tidal_acc[index_total_change][3][0] = 0
    index_total = [i for i, element in enumerate(tidal_acc) if element[0] == "total"][0]

    n_sub = int(len(time) / split)

    for start_sub in range(0, len(time), n_sub):
        if parts is not None and number_plot not in parts:
            number_plot += 1
            continue
        sub = slice(start_sub, start_sub + n_sub)
        sub_wo_first = slice(start_sub + 1, start_sub + n_sub)

        plt.figure(figsize=(15, 10), dpi=300)

        # plot all components
        ax1 = plt.subplot2grid((7, 1), (0, 0), rowspan = 4)

        color_index = 0

        for i in range(len(tidal_acc)):
            if tidal_acc[i][0] != "total" and tidal_acc[i][0] != "total_change":
                current_color = plt.rcParams['axes.prop_cycle'].by_key()['color'][color_index]
                ax1.bar(time[sub], tidal_acc[i][1][sub], width_bars, label=tidal_acc[i][0], color=current_color)
                ax1.bar(time[sub] - width_bars, tidal_acc[i][2][sub], width_bars, color=current_color)
                ax1.bar(time[sub] + width_bars, tidal_acc[i][3][sub], width_bars, color=current_color)

                if color_index < len(plt.rcParams['axes.prop_cycle'].by_key()['color']) - 1:
                    color_index += 1
                else:
                    color_index = 0

        plt.ylabel("log " + r"$A\ \ [\frac{m}{s^2}]$", fontsize=yaxis_font_size)
        plt.yscale("log")
        ax1.get_yaxis().set_major_locator(mpl.ticker.LogLocator(base=10,numticks=50))
        ax1.get_yaxis().set_major_formatter(myLogFormatterExponent)
        ax1.get_yaxis().set_minor_locator(mpl.ticker.LogLocator(base=10.0,subs=(0.25,0.5,0.75),numticks=50))
        ax1.get_yaxis().set_minor_formatter(mpl.ticker.NullFormatter())
        ax1.get_yaxis().set_tick_params(which='both', direction='in')
        ax1.get_yaxis().set_ticks_position('both')

        ax1.get_xaxis().set_major_formatter(mpl.ticker.NullFormatter())

        mpl.rcParams["legend.fontsize"] = 9
        lgd = plt.legend(loc="upper left", bbox_to_anchor=(1, 1))


        # plot total
        ax2 = plt.subplot2grid((7, 1), (4, 0))
        ax2.bar(time[sub], tidal_acc[index_total][1][sub], width_bars, label=tidal_acc[index_total][0])
        ax2.bar(time[sub] - width_bars, tidal_acc[index_total][2][sub], width_bars)
        ax2.bar(time[sub] + width_bars, tidal_acc[index_total][3][sub], width_bars)

        ax2.get_yaxis().set_tick_params(which='both', direction='in')
        ax2.get_yaxis().set_ticks_position('both')
        plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

        ax2.get_xaxis().set_major_formatter(mpl.ticker.NullFormatter())
        ax2.set_ylim([find_lower_pow(np.min(tidal_acc[index_total][1][sub])), find_upper_pow(np.max(tidal_acc[index_total][1][sub]))])

        plt.ylabel(r"$A_{total}\ \ [\frac{m}{s^2}]$", fontsize=yaxis_font_size)


        # plot change of total
        ax3 = plt.subplot2grid((7, 1), (5, 0))
        ax3.bar(time[sub], tidal_acc[index_total_change][1][sub], width_bars, label=tidal_acc[index_total_change][0])
        ax3.bar(time[sub] - width_bars, tidal_acc[index_total_change][2][sub], width_bars)
        ax3.bar(time[sub] + width_bars, tidal_acc[index_total_change][3][sub], width_bars)

        ax3.get_yaxis().set_tick_params(which='both', direction='in')
        ax3.get_yaxis().set_ticks_position('both')

        ax3.get_xaxis().set_major_formatter(mpl.ticker.NullFormatter())

        plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

        ax3.set_ylim([find_lower_pow(np.min(tidal_acc[index_total_change][1][sub_wo_first])), find_upper_pow(np.max(tidal_acc[index_total_change][1][sub_wo_first]))])

        plt.ylabel(r"$\Delta A_{total}\ \ [\frac{m}{s^2}]$", fontsize=yaxis_font_size)


        # plot A_total * Delta A_total / Delta t
        a_da_dt = [tidal_acc[index_total][j] * tidal_acc[index_total_change][j] / delta_t for j in (1, 2, 3)]
        ax4 = plt.subplot2grid((7, 1), (6, 0))
        ax4.bar(time[sub], a_da_dt[0][sub], width_bars)
        ax4.bar(time[sub] - width_bars, a_da_dt[1][sub], width_bars)
        ax4.bar(time[sub] + width_bars, a_da_dt[2][sub], width_bars)

        ax4.get_yaxis().set_tick_params(which='both', direction='in')
        ax4.get_yaxis().set_ticks_position('both')

        plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

        ax4.set_ylim([find_lower_pow(np.min(a_da_dt[0][sub_wo_first])), find_upper_pow(np.max(a_da_dt[0][sub_wo_first]))])

        plt.xlabel("julian date [d]", fontsize = yaxis_font_size)
        plt.ylabel(r"$A_{total} * \frac{\Delta A_{total}}{\Delta t}\ \ [\frac{m^2}{s^4\ d}]$", fontsize = yaxis_font_size)


        # write output
        os.makedirs(output_directory, exist_ok=True)
        if output_filenames is not None and number_plot in output_filenames:
            output_filename = os.path.join(output_directory, output_filenames[number_plot])
        else:
            output_filename = os.path.join(output_directory, "plot_" + filename[:filename.index('.')] + "_" + str(number_plot) + ".png")
        plt.savefig(output_filename, bbox_extra_artists=(lgd,), bbox_inches="tight")
        created.append(output_filename)

        plt.close("all")
        number_plot += 1

    return created


def _plot_file_star(arguments):
    return arguments[0], plot_file(*arguments)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="result file (.csv) or directory containing result files")
    parser.add_argument("output_directory")
    parser.add_argument("--split", type=int, default=15, help="number of images on which the data is temporally distributed (default: 15)")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="number of files plotted in parallel (default: number of CPUs)")
    args = parser.parse_args()

    if os.path.isdir(args.input):
        paths = sorted(os.path.join(args.input, f) for f in os.listdir(args.input) if f.endswith(".csv"))
    else:
        paths = [args.input]

    with multiprocessing.Pool(args.jobs) as pool:
        results = pool.imap_unordered(_plot_file_star, [(path, args.output_directory, args.split) for path in paths])
        for n, (path, created) in enumerate(results, 1):
            sys.stdout.write("\r(%d/%d) created %d plots for '%s'" % (n, len(paths), len(created), os.path.basename(path)))
            sys.stdout.flush()
    print("\n*** Done! ***")


if __name__ == "__main__":
    main()
