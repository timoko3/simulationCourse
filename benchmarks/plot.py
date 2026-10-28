"""Plot cached switch, threaded and DBT speedups relative to naive switch."""

import argparse
import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, LogLocator, NullLocator


SERIES = (
    ("switch_speedup", "Cached switch", "#B3A1D2"),
    ("threaded_speedup", "Threaded dispatch", "#648EC4"),
    ("dbt_speedup", "DBT", "#E79C78"),
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
    figure, axes = plt.subplots(figsize=(15, 6))
    positions = list(range(len(results)))
    bounds = [1.0]

    axes.axhline(1.0, color="#75B39E", linestyle="--", linewidth=1.5,
                 label="Naive switch: 1×", zorder=2)

    for index, (key, label, color) in enumerate(SERIES):
        estimates = [result["statistics"][key] for result in results]
        values = [estimate["median"] for estimate in estimates]
        offsets = [position + (index - (len(SERIES) - 1) / 2) * BAR_WIDTH for position in positions]

        for value, estimate in zip(values, estimates):
            interval = estimate["ci95"]
            bounds.extend([value, interval["low"], interval["high"]] if interval else [value])

        axes.bar(offsets, values, BAR_WIDTH, label=label, color=color,
                 edgecolor="#404040", linewidth=0.6, zorder=3)
        add_intervals(axes, offsets, values, estimates)
        add_labels(axes, offsets, values, estimates)

        average = geometric_mean(values)
        axes.axhline(average, color=color, linestyle="--", linewidth=1.5,
                    label=f"{label}: geometric mean {average:.2f}×", zorder=2)

    axes.set_title("Execution performance relative to naive switch", fontsize=16, pad=18)
    axes.set_ylabel("Speedup (naive time / mode time), log scale", fontsize=11)
    axes.set_xticks(positions, [result["name"] for result in results], fontsize=11)
    axes.set_yscale("log", base=2)
    axes.set_ylim(min(bounds) / 1.4, max(bounds) * 1.4)
    axes.yaxis.set_major_locator(LogLocator(base=2))
    axes.yaxis.set_major_formatter(FuncFormatter(lambda value, _position: f"{value:g}×"))
    axes.yaxis.set_minor_locator(NullLocator())
    axes.grid(axis="y", color="#D5D9DD", linewidth=0.8, zorder=0)
    axes.set_axisbelow(True)
    axes.spines[["top", "right"]].set_visible(False)
    axes.legend(loc="upper left", bbox_to_anchor=(0, -0.12), ncol=2, frameon=False)
    figure.text(0.5, 0.015, "Bars: median paired speedup; whiskers: approximate 95% bootstrap CI. Higher is faster. DBT includes JIT setup and translation.",
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
