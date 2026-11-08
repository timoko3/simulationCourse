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
    parser.add_argument("results", type=Path, help="results.json from comparison or threshold-sweep mode")
    args = parser.parse_args()

    with args.results.open(encoding="utf-8") as source:
        report = json.load(source)

    is_sweep = report.get("mode") == "threshold-sweep"
    figure = plot_threshold_sweep(report) if is_sweep else plot(report)
    basename = "threshold-speedup" if is_sweep else "dispatch-speedup"
    for extension in ("png", "svg"):
        output = args.results.parent / f"{basename}.{extension}"
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


def plot_threshold_sweep(report):
    summary = report["threshold_summary"]
    if not summary:
        raise ValueError("No completed workloads to plot")

    thresholds = [point["threshold"] for point in summary]
    means = [point["geometric_mean"] for point in summary]
    figure, (mean_axes, workload_axes) = plt.subplots(
        2, 1, figsize=(14, 10), sharex=True, gridspec_kw={"height_ratios": [1.2, 1]})
    figure.suptitle("DBT speedup versus translation threshold", fontsize=18, y=0.98)

    mean_axes.plot(thresholds, means, color="#D86B35", marker="o", linewidth=3,
                   label="Geometric mean", zorder=5)
    mean_axes.axhline(means[0], color="#888888", linestyle=":", linewidth=1,
                      label=f"Mean at threshold {thresholds[0]}")
    bounds = list(means)

    if all(point["ci95"] is not None for point in summary):
        lower = [point["ci95"]["low"] for point in summary]
        upper = [point["ci95"]["high"] for point in summary]
        bounds.extend(lower + upper)
        mean_axes.fill_between(thresholds, lower, upper, color="#D86B35", alpha=0.18,
                               label="Approximate 95% CI", zorder=2)

    padding = max((max(bounds) - min(bounds)) * 0.22, max(means) * 0.015)
    mean_axes.set_ylim(max(0, min(bounds) - padding), max(bounds) + padding)
    mean_axes.set_title("Geometric mean across workloads (linear scale)", fontsize=12, pad=12)
    mean_axes.set_xlabel("Translation threshold")
    mean_axes.tick_params(axis="x", labelbottom=True)
    mean_axes.set_ylabel("Geometric mean speedup")
    mean_axes.yaxis.set_major_formatter(FuncFormatter(lambda value, _position: f"{value:.2f}×"))
    mean_axes.legend(loc="best", frameon=False, fontsize=9)

    for index, (threshold, mean) in enumerate(zip(thresholds, means)):
        mean_axes.annotate(f"{mean:.2f}×", (threshold, mean),
                           xytext=(0, 10 if index % 2 == 0 else -18),
                           textcoords="offset points", ha="center", fontsize=9, color="#9A431E")

    relative_axes = mean_axes.secondary_yaxis(
        "right", functions=(lambda value: (value / means[0] - 1) * 100,
                             lambda value: means[0] * (1 + value / 100)))
    relative_axes.set_ylabel(f"Change from threshold {thresholds[0]}")
    relative_axes.yaxis.set_major_formatter(FuncFormatter(lambda value, _position: f"{value:+.0f}%"))

    workload_axes.axhline(1.0, color="#75B39E", linestyle="--", linewidth=1.5, label="Naive switch: 1×")

    for result in report["results"]:
        statistics = result["statistics"]["thresholds"]
        speedups = [statistics[str(threshold)]["speedup"]["median"] for threshold in thresholds]
        workload_axes.plot(thresholds, speedups, marker=".", linewidth=1.5, alpha=0.8, label=result["name"])

    workload_axes.set_title("Individual workloads (log scale)", fontsize=12, pad=12)
    workload_axes.set_xlabel("Interpreted block executions before translation (0: translate immediately)")
    workload_axes.set_ylabel("Speedup relative to naive")
    workload_axes.set_xscale("symlog", linthresh=1)
    workload_axes.set_xticks(thresholds, [format_threshold(value) for value in thresholds])
    workload_axes.xaxis.set_minor_locator(NullLocator())
    workload_axes.set_yscale("log", base=2)
    workload_axes.yaxis.set_major_locator(LogLocator(base=2))
    workload_axes.yaxis.set_major_formatter(FuncFormatter(lambda value, _position: f"{value:g}×"))
    workload_axes.yaxis.set_minor_locator(NullLocator())
    workload_axes.legend(loc="upper left", bbox_to_anchor=(0, -0.20), ncol=4, frameon=False, fontsize=9)

    for axes in (mean_axes, workload_axes):
        axes.grid(axis="both", color="#D5D9DD", linewidth=0.8)
        axes.set_axisbelow(True)
        axes.spines[["top", "right"]].set_visible(False)

    figure.text(0.5, 0.015,
                "Upper Y-axis zooms to the observed mean and CI. Cold caches; DBT includes JIT setup and translation.",
                ha="center", fontsize=10, color="#555555")
    figure.subplots_adjust(left=0.08, right=0.90, top=0.90, bottom=0.20, hspace=0.40)

    return figure


def format_threshold(value):
    if value >= 1_000_000 and value % 1_000_000 == 0:
        return f"{value // 1_000_000}M"
    if value >= 1000 and value % 1000 == 0:
        return f"{value // 1000}k"
    return str(value)


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
