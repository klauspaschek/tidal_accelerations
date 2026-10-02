"""Reproduce Figures 1-10 and Table 1 of Paschek et al. (2021), Appl. Sci. 11, 8624, doi:10.3390/app11188624.

Needs the results of the simulations ('make run'). All plotted values are calculated from these results; the
files in 'paper_data/' only provide the classification of the objects based on the scientific literature (what is
known about volcanism and subsurface oceans, see 'paper_data/README.md'), their radii, and the selection and order of
objects shown in Figures 5-7. The values of
{A_total * Delta A_total / Delta t}_max published in the paper are also contained in these files and are compared
to the reproduced ones in 'figures/reproduction_check.csv'.

usage: python3 plotting/paper_figures.py    (run from the main directory of the repository)
"""
import csv
import math
import os
import re
import sys

import matplotlib as mpl
mpl.use("Agg")
import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from plot_tidal_acc import plot_file, step_width_from_filename

####
results_full       = "solar_system/results/full"
results_simplified = "solar_system/results/simplified"
results_trappist1  = "trappist-1/results"
data_solar_system  = "solar_system/data"
data_trappist1     = "trappist-1/data"
paper_data         = os.path.join(os.path.dirname(os.path.abspath(__file__)), "paper_data")
output_directory   = "figures"
####


# formater function for logarithmic axes
def mylogfmt(x, pos):
    if x != 0 and not np.isnan(x):
        logx = int(np.log10(x)) # to get the exponent
    else:
        logx = 0

    return u"${:.0f}$".format(logx)

myLogFormatterExponent = mpl.ticker.FuncFormatter(mylogfmt)


def read_csv(path):
    with open(path, newline='', encoding='utf-8') as csv_file:
        rows = [[entry.strip() for entry in row] for row in csv.reader(csv_file) if row]
    return rows[0], rows[1:]


#### maximal value of A_total * Delta A_total / Delta t for each object

def max_a_da_dt(path):
    """{A_total * Delta A_total / Delta t}_max for the mean mass, skipping the first time step (no change defined)"""
    delta_t = step_width_from_filename(os.path.basename(path))
    header, rows = read_csv(path)
    index_total, index_total_change = header.index("total"), header.index("total_change")
    return max(float(row[index_total]) * float(row[index_total_change]) / delta_t for row in rows[1:])


def normalize(name):
    return re.sub(r'[^a-z0-9]', '', name.lower())


# designations used in the result filenames, e.g. 'J_1_Io' (moon), '5_Jupiter' (planet), 'TNO_136108_Haumea',
# 'A_4_Vesta' (asteroid), 'C__1995_O1__Hale-Bopp' (comet)
designation_patterns = [r'^(e|m|j|s|u|n|p|eris|haumea)\d+$', r'^\d+$', r'^tno\d+$', r'^a$', r'^c?$']

def object_values(results_directories):
    """maximal values for all result files, the maximum over all simulation runs (step widths) is used"""
    values = {}
    for directory in results_directories:
        for filename in os.listdir(directory):
            if filename.endswith(".csv"):
                key = normalize(filename[filename.index("_tidal_acc_") + 11 : -4])
                values[key] = max(values.get(key, 0.), max_a_da_dt(os.path.join(directory, filename)))
    return values

def find_value(name, values):
    name_trappist = re.match(r'^TRAPPIST-1([b-h])$', name)
    if name_trappist:
        target = normalize(name_trappist.group(1))
        matches = [key for key in values if re.match(r'^\d+trap' + target + '$', key)]
    else:
        target = normalize(name.replace("(J1) ", "").replace("’", "_"))
        matches = [key for key in values if key.endswith(target) and
                   any(re.match(pattern, key[: len(key) - len(target)]) for pattern in designation_patterns)]
    if not matches:
        sys.exit("Error: no simulation result found for object '" + name + "'")
    return max(values[key] for key in matches)


#### Figures 1-4 and 8-10: bar charts of the tidal accelerations acting on single objects

def bar_charts():
    # (figure, result file, number of parts the time span is split into, part shown in the paper)
    figures = [(1,  results_full,       "delta_t_0_25d_tidal_acc_J_3_Ganymede.csv",       15, 10),
               (2,  results_full,       "delta_t_90d_tidal_acc_A_4_Vesta.csv",            15, 10),
               (3,  results_full,       "delta_t_0_25d_tidal_acc_TNO_136108_Haumea.csv",  15,  1),
               (4,  results_full,       "delta_t_90d_tidal_acc_C__1995_O1__Hale-Bopp.csv", 15, 13),
               (8,  results_simplified, "delta_t_0_25d_tidal_acc_TNO_136108_Haumea.csv",  15,  1),
               (9,  results_full,       "delta_t_0_25d_tidal_acc_U_2_Umbriel.csv",        15,  1),
               (10, results_trappist1,  "delta_t_0_05d_tidal_acc_1_trapB.csv",             9,  1)]
    for number, directory, filename, split, part in figures:
        plot_file(os.path.join(directory, filename), output_directory, split, parts=[part],
                  output_filenames={part: "figure_%02d.png" % number})
        print("created figure %d" % number)


#### Figure 5: ranking of the solar system objects with the largest tidal accelerations

def figure_5(values_full, check):
    header, rows = read_csv(os.path.join(paper_data, "fig5_volcanism_ranking.csv"))
    number_defining = 27    # objects with known volcanism/subsurface ocean, the others are new candidates
    number_new      = len(rows) - number_defining

    reproduced = []
    for row in rows:
        reproduced.append(find_value(row[0], values_full))
        check.append(["Figure 5", row[0], float(row[1]), reproduced[-1]])

    plt.figure(figsize=(15.4, 20), dpi=300)
    ax = plt.subplot2grid((1, 1), (0, 0))

    # plot defining objects
    for index in range(number_defining):
        if rows[index][2] == 'Y':
            bar_color = "red"   # siliceous volcanism
        elif rows[index][3] == 'Y':
            bar_color = "orange"# at least former siliceous volcanism
        elif rows[index][4] == 'Y':
            bar_color = "C0"    # cryovolcanism/subsurface ocean
        elif rows[index][5] == 'Y':
            bar_color = "C9"    # at least former cryovolcanism/subsurface ocean
        elif rows[index][6] == 'Y':
            bar_color = "C4"    # candidates for reevaluation
        ax.barh(number_defining + number_new - 1 - index, reproduced[index], 0.5, color=bar_color, edgecolor='k')

    # plot new objects
    ax.barh(np.arange(number_new)[::-1], reproduced[number_defining:], 0.5, color="C7", edgecolor='k')

    # format axes
    plt.xscale("log")
    plt.xlabel("log" + r"$(\{A_{total} * \frac{\Delta A_{total}}{\Delta t}\}_{max})\ \ [\frac{m^2}{s^4\ d}]$", fontsize=20)
    plt.tick_params(axis='x', which='both', direction='in', labeltop=True, top=True)
    plt.xticks(fontsize=16, ha='center')
    ax.get_xaxis().set_major_locator(mpl.ticker.LogLocator(base=10,numticks=50))
    ax.get_xaxis().set_major_formatter(myLogFormatterExponent)
    ax.get_xaxis().set_minor_formatter(mpl.ticker.NullFormatter())
    ax.get_xaxis().set_tick_params(which='both', direction='in', width=1.5)
    ax.get_xaxis().set_ticks_position('both')
    ax.get_xaxis().set_minor_locator(mpl.ticker.LogLocator(base=10.0,subs=(0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9),numticks=50))
    plt.yticks(np.arange(number_defining + number_new)[::-1], [row[0] for row in rows], fontsize=16)

    # create legend
    patches = [mpatches.Patch(facecolor=color, edgecolor=edge) for color, edge in
               [("red", 'k'), ("orange", 'k'), ("C0", 'k'), ("none", 'none'), ("C9", 'k'), ("none", 'none'), ("C4", 'k'), ("C7", 'k')]]
    plt.legend(patches, ["siliceous volcanism known", "at least former siliceous volcanism known",
                         "non-siliceous cryovolcanism/subsurface ocean", "known/strongly assumed",
                         "at least former non-siliceous cryovolcanism/subsurface", "ocean known/strongly assumed",
                         "potential candidates for reevaluation of tidal based volcanism",
                         "new candidates for tidal based volcanism"], prop={"size": 13.7}, bbox_to_anchor=(0.433, 0.16))

    plt.savefig(os.path.join(output_directory, "figure_05.png"), bbox_inches="tight")
    plt.close("all")
    print("created figure 5")


#### Figures 6 and 7: maximal tidal accelerations against radius

def scatter_radius(number, filename, values, values_trappist1, ylim, xlim, check):
    from adjustText import adjust_text

    header, rows = read_csv(os.path.join(paper_data, filename))

    plt.figure(figsize=(16, 9), dpi=300)
    ax = plt.subplot2grid((1, 1), (0, 0))

    plt.yscale("log")
    plt.xscale("log")

    plt.ylabel("log" + r"$(\{A_{total} * \frac{\Delta A_{total}}{\Delta t}\}_{max})\ \ [\frac{m^2}{s^4\ d}]$", fontsize=18)
    plt.tick_params(axis='both', labelsize=14)
    ax.get_yaxis().set_major_locator(mpl.ticker.LogLocator(base=10,numticks=50))
    ax.get_yaxis().set_major_formatter(myLogFormatterExponent)
    ax.get_yaxis().set_minor_locator(mpl.ticker.LogLocator(base=10.0,subs=(0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9),numticks=50))
    ax.get_yaxis().set_minor_formatter(mpl.ticker.NullFormatter())
    ax.get_yaxis().set_tick_params(which='both', direction='in')
    ax.get_yaxis().set_ticks_position('both')
    plt.xlabel("R " + r"$[km]$", fontsize=18)
    ax.set_ylim(ylim)
    ax.set_xlim(xlim)

    # (marker, color) for: siliceous, former siliceous, cryo, former cryo, uncertain cryo
    styles = [("d", "red"), ("^", "orange"), ("s", "C0"), ("*", "C9"), ("p", "C4")]
    x_positions, y_positions, labels = [], [], []
    for row in rows:
        name = row[0]
        if name.startswith("TRAPPIST-1"):
            value = find_value(name, values_trappist1)
        else:
            value = find_value(name, values)
        check.append(["Figure %d" % number, name, float(row[2]), value])

        marker_type, marker_color, marker_size = ".", "black", 40
        for column, (marker, color) in zip(row[5:10], styles):
            if column == "Y":
                marker_type, marker_color, marker_size = marker, color, 80 if marker == "*" else 50
                break

        radius = float(row[3])
        if not (xlim[0] <= radius <= xlim[1] and ylim[0] <= value <= ylim[1]):
            continue    # outside of the plotted range (e.g., comets)
        ax.scatter(radius, value, marker=marker_type, color=marker_color, s=marker_size, zorder=3)
        x_positions.append(radius)
        y_positions.append(value)
        # labels to the bottom left of the data points, as in the paper
        labels.append(plt.text(radius, value, "(J1) Io" if name == "Io" else name, fontsize=11, ha="right", va="top"))

    # approximate boundary between mainly tidal and mainly radioactive (and stored accretion energy) heating,
    # determined empirically from the volcanism of the objects in the solar system (drawn by hand in the paper)
    boundary_r, boundary_value = [990., 7940.], [10**-15.07, 10**-1.7]
    ax.plot(boundary_r, boundary_value, color="black", linestyle=":", linewidth=2.5, zorder=2)

    # create legend
    descriptions = ["siliceous volcanism known", "at least former siliceous volcanism known",
                    "non-siliceous cryovolcanism/subsurface ocean known/strongly assumed",
                    "at least former non-siliceous cryovolcanism/subsurface ocean known/strongly assumed",
                    "potential candidates for reevaluation of tidal based volcanism"]
    handles = [Line2D([0], [0], marker=marker, color='w', label=description, markerfacecolor=color,
                      markeredgecolor=color, markersize=12 if marker == "*" else 9, linestyle='')
               for (marker, color), description in zip(styles, descriptions)]
    plt.legend(handles=handles, prop={"size": 10}, loc="lower right", bbox_to_anchor=(1., 0.1))

    # labels of the boundary, parallel to the line (angle on screen known only after drawing the final layout)
    plt.gcf().canvas.draw()
    start, end = ax.transData.transform((boundary_r[0], boundary_value[0])), ax.transData.transform((boundary_r[1], boundary_value[1]))
    angle = math.atan2(end[1] - start[1], end[0] - start[0])
    boundary_labels = []
    for text, r_text, side, va in [("mainly tidal heating", 1000., 1, "bottom"), ("mainly radioactive heating", 1010., -1, "top")]:
        value_text = 10**(math.log10(boundary_value[0]) + (math.log10(r_text) - math.log10(boundary_r[0])) *
                          (math.log10(boundary_value[1]) - math.log10(boundary_value[0])) / (math.log10(boundary_r[1]) - math.log10(boundary_r[0])))
        offset = mpl.transforms.offset_copy(ax.transData, fig=plt.gcf(), x=-side * 5. * math.sin(angle), y=side * 5. * math.cos(angle), units="points")
        boundary_labels.append(ax.text(r_text, value_text, text, rotation=math.degrees(angle), rotation_mode="anchor",
                                       va=va, ha="left", fontsize=9, transform=offset))

    # adjust labels to avoid overlaps
    adjust_text(labels, x=x_positions, y=y_positions, objects=boundary_labels, ax=ax, force_text=(0.1, 0.25))

    plt.savefig(os.path.join(output_directory, "figure_%02d.png" % number), bbox_inches="tight")
    plt.close("all")
    print("created figure %d" % number)


#### Table 1: comparison of the planets of TRAPPIST-1 with (J1) Io and Ganymede

def table_1(values_full, values_trappist1, check):
    published = {"TRAPPIST-1b": 9.36e-3, "TRAPPIST-1c": 8.50e-4, "(J1) Io": 2.43e-4, "TRAPPIST-1d": 3.36e-5,
                 "TRAPPIST-1e": 5.70e-6, "TRAPPIST-1f": 9.98e-7, "Ganymede": 5.24e-7, "TRAPPIST-1g": 2.62e-7,
                 "TRAPPIST-1h": 1.50e-8}
    table = []

    header, rows = read_csv(os.path.join(data_trappist1, "trappist-1_planets.csv"))
    column = {name: index for index, name in enumerate(header)}
    for row in rows:
        name = "TRAPPIST-1" + row[column["Planet"]][-1].lower()
        table.append([name, float(row[column["a"]]) * 149597870.7, float(row[column["e"]]), float(row[column["n"]]),
                      float(row[column["radius"]]), float(row[column["mean_mass"]]), find_value(name, values_trappist1)])

    header, rows = read_csv(os.path.join(data_solar_system, "moons_orbital_elements_physical.csv"))
    column = {name: index for index, name in enumerate(header)}
    for row in rows:
        if row[column["Moon"]] in ("Io", "Ganymede"):
            name = "(J1) Io" if row[column["Moon"]] == "Io" else "Ganymede"
            table.append([name, float(row[column["a"]]), float(row[column["e"]]), math.radians(float(row[column["n"]])),
                          float(row[column["Mean_radius"]]), float(row[column["Mass"]]), find_value(name, values_full)])

    table.sort(key=lambda x: x[6], reverse=True)
    with open(os.path.join(output_directory, "table_01.csv"), 'w') as output_file:
        output_file.write("object,a[km],e,n[rad/day],R[km],M[kg],{A_total*(Delta A_total/Delta t)}_max[m^2/(s^4 d)]\n")
        for row in table:
            output_file.write("%s,%.3g,%.4f,%.4f,%.2f,%.3g,%.3g\n" % tuple(row))
            check.append(["Table 1", row[0], published[row[0]], row[6]])
    print("created table 1")


def main():
    for directory in (results_full, results_simplified, results_trappist1):
        if not os.path.isdir(directory) or not os.listdir(directory):
            sys.exit("Error: no simulation results in '" + directory + "', run 'make run' first")
    os.makedirs(output_directory, exist_ok=True)

    values_full       = object_values([results_full])
    values_simplified = object_values([results_simplified])
    values_trappist1  = object_values([results_trappist1])

    check = []
    bar_charts()
    figure_5(values_full, check)
    scatter_radius(6, "fig6_objects_full.csv",       values_full,       values_trappist1, [4e-21, 2e-2], [50, 8000], check)
    scatter_radius(7, "fig7_objects_simplified.csv", values_simplified, values_trappist1, [1e-21, 1e-3], [50, 7000], check)
    table_1(values_full, values_trappist1, check)

    # compare with the values published in the paper (given with three significant digits)
    deviations = 0
    with open(os.path.join(output_directory, "reproduction_check.csv"), 'w') as output_file:
        output_file.write("figure,object,published,reproduced,relative_deviation\n")
        for figure, name, published, reproduced in check:
            deviation = abs(reproduced - published) / published
            output_file.write("%s,%s,%.3g,%.4g,%.2g\n" % (figure, name, published, reproduced, deviation))
            if deviation > 0.01:
                deviations += 1
                print("deviation from published value: %s, %s: published %.3g, reproduced %.4g" % (figure, name, published, reproduced))
    print("%d of %d values agree with the published values within their precision" % (len(check) - deviations, len(check)))
    print("*** Done! ***")


if __name__ == "__main__":
    main()
