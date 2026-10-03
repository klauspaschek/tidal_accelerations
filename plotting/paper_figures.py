"""Figures 1-10 and Table 1 of Paschek et al. (2021), Appl. Sci. 11, 8624, doi:10.3390/app11188624, with the results
of the current version of the code. Version 1.0 of the code reproduces the figures of the paper; since version 2.0 the
errors of the code used for the paper are corrected (see CHANGELOG.md), so the values differ, mostly by a factor of 2.

Needs the results of the simulations ('make run'). All plotted values are taken from these results; the files in
'paper_data/' only provide the classification of the objects based on the scientific literature (what is known about
volcanism and subsurface oceans, see 'paper_data/README.md'), their radii, and the selection of objects shown in
Figures 5-7. The values of {A_total * Delta A_total / Delta t}_max published in the paper are also contained in these
files and are compared to the current ones in 'figures/comparison_with_paper.csv'.

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
results_trappist1  = "exoplanets/results/trappist-1"
data_solar_system  = "solar_system/data"
data_trappist1     = "exoplanets/data"
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

def read_maxima(path):
    """{A_total * Delta A_total / Delta t}_max (mean mass) of all objects of a simulation run, determined from all time steps"""
    header, rows = read_csv(path)
    return {row[0]: float(row[2]) for row in rows}


def normalize(name):
    return re.sub(r'[^a-z0-9]', '', name.lower())


# designations used in the result filenames, e.g. 'J_1_Io' (moon), '5_Jupiter' (planet), 'TNO_136108_Haumea',
# 'A_4_Vesta' (asteroid), 'C__1995_O1__Hale-Bopp' (comet)
designation_patterns = [r'^(e|m|j|s|u|n|p|eris|haumea)\d+$', r'^\d+$', r'^tno\d+$', r'^a$', r'^c?$']

def object_values(results_directories, prefix="delta_t_"):
    """maximal values for all objects, the maximum over all simulation runs (time steps) is used"""
    values = {}
    for directory in results_directories:
        for filename in os.listdir(directory):
            if filename.startswith(prefix) and filename.endswith("_maxima.csv"):
                for name, value in read_maxima(os.path.join(directory, filename)).items():
                    key = normalize(name)
                    values[key] = max(values.get(key, 0.), value)
    return values

def find_value(name, values):
    target = normalize(name.replace("(J1) ", "").replace("’", "_"))
    matches = [key for key in values if key.endswith(target) and
               any(re.match(pattern, key[: len(key) - len(target)]) for pattern in designation_patterns)]
    if not matches:
        sys.exit("Error: no simulation result found for object '" + name + "'")
    return max(values[key] for key in matches)


#### Figures 1-4 and 8-10: bar charts of the tidal accelerations acting on single objects

def bar_charts():
    # (figure, result file, number of parts the time span is split into, part shown in the paper)
    # the time window of Figure 4 shows the encounter of Hale-Bopp with Jupiter in 1996, which is at a different time
    # than in the paper due to the corrected orbits
    figures = [(1,  results_full,       "delta_t_0_0125d_tidal_acc_J_3_Ganymede.csv",      15, 10),
               (2,  results_full,       "delta_t_90d_tidal_acc_A_4_Vesta.csv",             15, 10),
               (3,  results_full,       "delta_t_0_0125d_tidal_acc_TNO_136108_Haumea.csv", 15,  1),
               (4,  results_full,       "delta_t_90d_tidal_acc_C__1995_O1__Hale-Bopp.csv", 15,  2),
               (8,  results_simplified, "delta_t_0_0125d_tidal_acc_TNO_136108_Haumea.csv", 15,  1),
               (9,  results_full,       "delta_t_0_0125d_tidal_acc_U_2_Umbriel.csv",       15,  1),
               (10, results_trappist1,  "delta_t_0_05d_tidal_acc_1_TRAPPIST-1_b.csv",       9,  1)]
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

    # sort both groups (known volcanism/subsurface ocean, new candidates) by the current values, as in the paper
    order = (sorted(range(number_defining), key=lambda k: -reproduced[k]) +
             sorted(range(number_defining, len(rows)), key=lambda k: -reproduced[k]))
    rows, reproduced = [rows[k] for k in order], [reproduced[k] for k in order]

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
    legend = plt.legend(patches, ["siliceous volcanism known", "at least former siliceous volcanism known",
                         "non-siliceous cryovolcanism/subsurface ocean", "known/strongly assumed",
                         "at least former non-siliceous cryovolcanism/subsurface", "ocean known/strongly assumed",
                         "potential candidates for reevaluation of tidal based volcanism",
                         "new candidates for tidal based volcanism"], prop={"size": 13.7}, loc="lower right")

    # space above the bars as in the paper, and space below the bars such that the legend is just below the longest
    # of the bars it would otherwise overlap (the bars of the new candidates are short at the bottom)
    top = number_defining + number_new + 0.1
    ax.set_ylim(-5.0, top)
    plt.gcf().canvas.draw()
    legend_box = legend.get_window_extent().transformed(ax.transAxes.inverted())
    left_of_legend = ax.transData.inverted().transform(ax.transAxes.transform((legend_box.x0, 0.)))[0]
    blocking = [k for k in range(number_new) if reproduced[number_defining + number_new - 1 - k] > left_of_legend]
    legend_top = (min(blocking) if blocking else number_new) - 0.25 - 0.35    # bar half height and gap
    bottom = (legend_top - legend_box.y1 * top) / (1. - legend_box.y1)
    ax.set_ylim(bottom, top)

    plt.savefig(os.path.join(output_directory, "figure_05.png"), bbox_inches="tight")
    plt.close("all")
    print("created figure 5")


#### Figures 6 and 7: maximal tidal accelerations against radius

def place_labels(ax, texts, points, obstacle_circles, obstacle_boxes, divider=None, preferred_angle=225.):
    """places the labels 'texts' of the data 'points' (display coordinates) without overlaps: for each label, positions
    around its data point are tried (16 directions, increasing distances) and the one with the least overlap with the
    other labels, the circles 'obstacle_circles' ((x, y, radius), e.g. markers and lines), the boxes 'obstacle_boxes'
    ((x0, y0, x1, y1), e.g. legend), and the frame of the axes, and without crossing the straight line 'divider'
    ((x0, y0), (x1, y1)), e.g. a boundary between two regimes, is chosen, preferring short distances and the direction
    'preferred_angle' (in degrees, 225 = bottom left as in the paper); repeated until no label changes its position,
    labels away from their data point are connected to it by a line"""
    figure = ax.figure
    renderer = figure.canvas.get_renderer()
    pad = 1.5    # free space around the text [pixel]
    sizes = []
    for text in texts:
        extent = text.get_window_extent(renderer)
        sizes.append((extent.width + 2 * pad, extent.height + 2 * pad))
    frame = ax.get_window_extent(renderer)
    scale = figure.dpi / 72.    # points to pixels

    circles = np.array(obstacle_circles + [(x, y, 4. * scale) for x, y in points])
    angles = np.radians(np.arange(0., 360., 22.5))
    gaps = np.array([1.5, 4., 8., 13., 19., 26., 35., 46., 60.]) * scale

    def overlap_box(a, b):
        return max(0., min(a[2], b[2]) - max(a[0], b[0])) * max(0., min(a[3], b[3]) - max(a[1], b[1]))

    def overlap_circles(box):
        # number of covered circles, from the overlap of the box with the square around each circle
        dx = np.clip(np.minimum(box[2], circles[:, 0] + circles[:, 2]) - np.maximum(box[0], circles[:, 0] - circles[:, 2]), 0., None)
        dy = np.clip(np.minimum(box[3], circles[:, 1] + circles[:, 2]) - np.maximum(box[1], circles[:, 1] - circles[:, 2]), 0., None)
        return np.sum(dx * dy / (2. * circles[:, 2])**2)

    def candidates(index):
        (x, y), (width, height) = points[index], sizes[index]
        for angle in angles:
            ux, uy = math.cos(angle), math.sin(angle)
            for gap in gaps:
                # distance of the center such that the box is 'gap' away from the data point in this direction
                distance = gap + min(abs(ux) * width / 2 + abs(uy) * height / 2, math.hypot(width, height) / 2)
                cx, cy = x + ux * distance, y + uy * distance
                deviation = abs((math.degrees(angle) - preferred_angle + 180.) % 360. - 180.)
                yield (cx - width / 2, cy - height / 2, cx + width / 2, cy + height / 2), gap, deviation

    def distance_box(box, x, y):
        return math.hypot(max(box[0] - x, 0., x - box[2]), max(box[1] - y, 0., y - box[3]))

    def leader(index, box):
        # points along the line from the data point to the closest point of the label
        x, y = points[index]
        nearest = (min(max(x, box[0] + pad), box[2] - pad), min(max(y, box[1] + pad), box[3] - pad))
        return [(x + t * (nearest[0] - x), y + t * (nearest[1] - y)) for t in np.linspace(0.15, 0.9, 8)]

    margin = 4. * scale    # minimal distance from the frame of the axes
    inner = (frame.x0 + margin, frame.y0 + margin, frame.x1 - margin, frame.y1 - margin)
    clear = 8. * scale     # a label has to be this much closer to its own data point than to any other one

    def opposite_side(index, x, y):
        # (x, y) next to the line 'divider' but on the other side than the data point
        if divider is None:
            return False
        (x0, y0), (x1, y1) = divider
        if not (min(x0, x1) <= x <= max(x0, x1) and min(y0, y1) <= y <= max(y0, y1)):
            return False
        side = lambda u, v: (x1 - x0) * (v - y0) - (y1 - y0) * (u - x0)
        return side(x, y) * side(*points[index]) < 0.

    def cost(index, box, gap, deviation, boxes):
        area = (box[2] - box[0]) * (box[3] - box[1])
        outside = area - overlap_box(box, inner)
        overlap = sum(overlap_box(box, other) for other in obstacle_boxes)
        others = [other for k, other in enumerate(boxes) if k != index and other is not None]
        overlap += sum(overlap_box(box, other) for other in others)
        # labels must be unambiguously assigned to their data points
        own = distance_box(box, *points[index])
        ambiguity = sum(max(0., own + clear - distance_box(box, x, y)) / clear
                        for k, (x, y) in enumerate(points) if k != index)
        # the line to the data point should not cross other labels (and the lines of other labels not this one)
        crossing = 0
        if gap > 8. * scale:
            crossing += sum(1 for x, y in leader(index, box) for other in others if distance_box(other, x, y) == 0.)
        for k, other in enumerate(boxes):
            if k != index and other is not None and math.hypot(*np.subtract(points[k], ((other[0] + other[2]) / 2, (other[1] + other[3]) / 2))) > 14. * scale:
                crossing += sum(1 for x, y in leader(k, other) if distance_box(box, x, y) == 0.)
        crossing += 2 * sum(1 for x, y in [(box[0], box[1]), (box[0], box[3]), (box[2], box[1]), (box[2], box[3])] + leader(index, box)
                            if opposite_side(index, x, y))
        return (20. * outside / area + 4. * overlap / area + 1.5 * overlap_circles(box) + 3. * ambiguity + 0.5 * crossing +
                gap / (10. * scale) + deviation / 180.)

    boxes = [None] * len(texts)
    # labels in crowded regions first
    crowding = [sum(1 for other in points if math.hypot(other[0] - x, other[1] - y) < 40. * scale) for x, y in points]
    order = sorted(range(len(texts)), key=lambda k: -crowding[k])
    for iteration in range(20):
        changed = False
        for index in order:
            best = min(candidates(index), key=lambda c: cost(index, c[0], c[1], c[2], boxes))[0]
            if best != boxes[index]:
                boxes[index], changed = best, True
        if not changed:
            break

    inverse = ax.transData.inverted()
    for text, box, (x, y) in zip(texts, boxes, points):
        text.set_position(inverse.transform(((box[0] + box[2]) / 2, (box[1] + box[3]) / 2)))
        text.set_ha("center")
        text.set_va("center")
        # line from the data point to the closest point of the label, if not directly next to it
        nearest = (min(max(x, box[0] + pad), box[2] - pad), min(max(y, box[1] + pad), box[3] - pad))
        if math.hypot(nearest[0] - x, nearest[1] - y) > 9. * scale:
            ax.plot(*zip(inverse.transform((x, y)), inverse.transform(nearest)), color="0.5", lw=0.6, zorder=1)


def scatter_radius(number, filename, values, values_trappist1, xlim, check, notes={}):
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
    # y range from the smallest to the largest value of the objects within the x range, in full decades
    plotted = []
    for row in rows:
        if xlim[0] <= float(row[3]) <= xlim[1]:
            plotted.append(find_value(row[0].replace("TRAPPIST-1", "TRAPPIST-1 "), values_trappist1) if row[0].startswith("TRAPPIST-1")
                           else find_value(row[0], values))
    ylim = [10**math.floor(math.log10(min(plotted)) - 0.3), 10**math.ceil(math.log10(max(plotted)) + 0.1)]
    ax.set_ylim(ylim)
    ax.set_xlim(xlim)

    # (marker, color) for: siliceous, former siliceous, cryo, former cryo, uncertain cryo
    styles = [("d", "red"), ("^", "orange"), ("s", "C0"), ("*", "C9"), ("p", "C4")]
    x_positions, y_positions, labels = [], [], []
    for row in rows:
        name = row[0]
        if name.startswith("TRAPPIST-1"):
            value = find_value(name.replace("TRAPPIST-1", "TRAPPIST-1 "), values_trappist1)
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
        label = ("(J1) Io" if name == "Io" else name.replace("_", "’")) + ("*" if name in notes else "")
        labels.append(plt.text(radius, value, label, fontsize=10, ha="right", va="top"))

    # approximate boundary between mainly tidal and mainly radioactive (and stored accretion energy) heating,
    # determined empirically from the volcanism of the objects in the solar system (drawn by hand in the paper)
    # as all values are about a factor of 2 lower than in the paper (corrected orbits), the boundary is shifted by
    # this factor, it has to be reevaluated with the current results
    boundary_r, boundary_value = [990., 7940.], [0.5 * 10**-15.07, 0.5 * 10**-1.7]
    ax.plot(boundary_r, boundary_value, color="black", linestyle=":", linewidth=2.5, zorder=2)

    # create legend
    descriptions = ["siliceous volcanism known", "at least former siliceous volcanism known",
                    "non-siliceous cryovolcanism/subsurface ocean known/strongly assumed",
                    "at least former non-siliceous cryovolcanism/subsurface ocean known/strongly assumed",
                    "potential candidates for reevaluation of tidal based volcanism"]
    handles = [Line2D([0], [0], marker=marker, color='w', label=description, markerfacecolor=color,
                      markeredgecolor=color, markersize=12 if marker == "*" else 9, linestyle='')
               for (marker, color), description in zip(styles, descriptions)]
    legend = plt.legend(handles=handles, prop={"size": 10}, loc="lower right", bbox_to_anchor=(1., 0.1))

    # notes on objects marked with *
    note_texts = [ax.text(0.995, 0.012, "* " + note, transform=ax.transAxes, ha="right", va="bottom", fontsize=10)
                  for note in notes.values()]

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

    # place the labels without overlaps with each other, the data points, the boundary line, its labels, the legend,
    # and the note; labels away from their data points are connected to them by a line
    plt.gcf().canvas.draw()
    to_display = lambda x, y: tuple(ax.transData.transform((x, y)))
    scale = plt.gcf().dpi / 72.
    boundary_points = np.logspace(np.log10(boundary_r[0]), np.log10(boundary_r[1]), 300)
    boundary_points_value = 10**np.interp(np.log10(boundary_points), np.log10(boundary_r), np.log10(boundary_value))
    circles = [to_display(r, v) + (4. * scale,) for r, v in zip(boundary_points, boundary_points_value)]
    renderer = plt.gcf().canvas.get_renderer()
    for text in boundary_labels:
        # the rotated labels as a chain of circles along their center line
        width = text.get_fontsize() * scale * 0.55 * len(text.get_text())
        start = np.array(text.get_transform().transform(text.get_position()))
        direction = np.array([math.cos(angle), math.sin(angle)])
        normal = np.array([-math.sin(angle), math.cos(angle)]) * (1 if text.get_va() == "bottom" else -1)
        for t in np.linspace(0., width, 40):
            center = start + direction * t + normal * 0.5 * text.get_fontsize() * scale
            circles.append((center[0], center[1], 0.6 * text.get_fontsize() * scale))
    boxes = [tuple(artist.get_window_extent(renderer).extents) for artist in note_texts + [legend]]
    place_labels(ax, labels, [to_display(x, y) for x, y in zip(x_positions, y_positions)], circles, boxes,
                 divider=(to_display(boundary_r[0], boundary_value[0]), to_display(boundary_r[1], boundary_value[1])))

    plt.savefig(os.path.join(output_directory, "figure_%02d.png" % number), bbox_inches="tight")
    plt.close("all")
    print("created figure %d" % number)


#### Table 1: comparison of the planets of TRAPPIST-1 with (J1) Io and Ganymede

def table_1(values_full, values_trappist1, check):
    published = {"TRAPPIST-1b": 9.36e-3, "TRAPPIST-1c": 8.50e-4, "(J1) Io": 2.43e-4, "TRAPPIST-1d": 3.36e-5,
                 "TRAPPIST-1e": 5.70e-6, "TRAPPIST-1f": 9.98e-7, "Ganymede": 5.24e-7, "TRAPPIST-1g": 2.62e-7,
                 "TRAPPIST-1h": 1.50e-8}
    table = []

    header, rows = read_csv(os.path.join(data_trappist1, "trappist-1.csv"))
    column = {name: index for index, name in enumerate(header)}
    for row in rows[1:]:    # skip line of units
        if row[column["type"]] == "planet":
            name = row[column["name"]].replace(" ", "")
            table.append([name, float(row[column["a"]]) * 149597870.7, float(row[column["e"]]), 2. * math.pi / float(row[column["period"]]),
                          float(row[column["radius"]]) * 6378.137, float(row[column["mass"]]) * 5.9722e24,
                          find_value(row[column["name"]], values_trappist1)])

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
    values_trappist1  = object_values([results_trappist1], prefix="delta_t_0_05d")    # setup of the paper

    check = []
    bar_charts()
    figure_5(values_full, check)
    scatter_radius(6, "fig6_objects_full.csv",       values_full,       values_trappist1, [40, 10000], check)
    scatter_radius(7, "fig7_objects_simplified.csv", values_simplified, values_trappist1, [40, 7000], check,
                   {"Janus": "close approach to the co-orbital moon Epimetheus, an artefact of the simplified orbits (all mean anomalies at epoch zero)"})
    table_1(values_full, values_trappist1, check)

    # compare with the values published in the paper
    ratios = []
    with open(os.path.join(output_directory, "comparison_with_paper.csv"), 'w') as output_file:
        output_file.write("figure,object,paper,current,ratio\n")
        for figure, name, published, current in check:
            output_file.write("%s,%s,%.3g,%.4g,%.3g\n" % (figure, name, published, current, current / published))
            ratios.append(current / published)
    ratios.sort()
    print("ratio of current to published values: median %.2f (range %.3g - %.3g), see '%s'" %
          (ratios[len(ratios) // 2], ratios[0], ratios[-1], os.path.join(output_directory, "comparison_with_paper.csv")))
    print("*** Done! ***")


if __name__ == "__main__":
    main()
