"""Plot the three interpreters from the Ruby runner's analyzed results.json."""

import argparse
import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


SERIES = (
    (None, "Naive switch", "#75B39E"),
    ("switch_speedup", "Cached switch", "#E79C78"),
    ("threaded_speedup", "Threaded dispatch", "#648EC4"),
)
BAR_WIDTH = 0.24


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("results", type=Path, help="results.json from comparison mode")
    args = parser.parse_args()

    with args.results.open(encoding="utf-8") as source:
        report = json.load(source)

    figure = plot(report)
    for extension in ("png", "svg"):
        output = args.results.parent / f"dispatch-speedup.{extension}"
        figure.savefig(output, dpi=180, bbox_inches="tight")
        print(output)

    plt.close(figure)


def plot(report):
    results = report["results"]
    figure, axes = plt.subplots(figsize=(12, 6))
    positions = list(range(len(results)))

    for index, (key, label, color) in enumerate(SERIES):
        estimates = [result["statistics"][key] for result in results] if key else []
        values = [estimate["median"] for estimate in estimates] if key else [1.0] * len(results)
        offsets = [position + (index - 1) * BAR_WIDTH for position in positions]

        axes.bar(offsets, values, BAR_WIDTH, label=label, color=color,
                 edgecolor="#404040", linewidth=0.6, zorder=3)
        add_intervals(axes, offsets, values, estimates)
        add_labels(axes, offsets, values, estimates)

        average = geometric_mean(values)
        axes.axhline(average, color=color, linestyle="--", linewidth=1.5,
                    label=f"{label}: geometric mean {average:.2f}×", zorder=2)

    axes.set_title("Interpreter performance relative to naive switch", fontsize=16, pad=18)
    axes.set_ylabel("Speedup (naive time / interpreter time)", fontsize=11)
    axes.set_xticks(positions, [result["name"] for result in results], fontsize=11)
    axes.set_ylim(bottom=0)
    axes.margins(y=0.18)
    axes.grid(axis="y", color="#D5D9DD", linewidth=0.8, zorder=0)
    axes.set_axisbelow(True)
    axes.spines[["top", "right"]].set_visible(False)
    axes.legend(loc="upper left", bbox_to_anchor=(0, -0.12), ncol=2, frameon=False)
    figure.text(0.5, 0.015, "Bars: median paired speedup; whiskers: approximate 95% bootstrap CI. Higher is faster.",
                ha="center", fontsize=10, color="#555555")
    figure.subplots_adjust(bottom=0.26)

    return figure


def add_intervals(axes, positions, values, estimates):
    if not estimates or any(estimate["ci95"] is None for estimate in estimates):
        return

    lower = [max(0, value - estimate["ci95"]["low"]) for value, estimate in zip(values, estimates)]
    upper = [max(0, estimate["ci95"]["high"] - value) for value, estimate in zip(values, estimates)]
    axes.errorbar(positions, values, yerr=[lower, upper], fmt="none", ecolor="#303030",
                  capsize=3, linewidth=1, zorder=4)


def add_labels(axes, positions, values, estimates):
    for position, value, estimate in zip(positions, values, estimates or [None] * len(values)):
        interval = estimate["ci95"] if estimate else None
        top = max(value, interval["high"]) if interval else value
        axes.annotate(f"{value:.2f}×", (position, top), xytext=(0, 6),
                      textcoords="offset points", ha="center", fontsize=10)


def geometric_mean(values):
    return math.exp(sum(math.log(value) for value in values) / len(values))


if __name__ == "__main__":
    main()
