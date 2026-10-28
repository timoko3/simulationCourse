module Benchmarks
    module ThresholdReport
        TABLE_FORMAT = '%12s %24s %30s'.freeze
        HEADERS = ['Threshold', 'Geometric mean speedup', '95% CI'].freeze

        def self.analyze(report)
            thresholds = report.fetch('thresholds')
            results = report.fetch('results').map do |result|
                samples = result.fetch('measurement').fetch('samples')
                statistics = thresholds.to_h do |threshold|
                    paired = samples.map do |sample|
                        { 'naive_ns' => sample.fetch('naive_ns'), 'dbt_ns' => sample.fetch('dbt_ns').fetch(threshold.to_s) }
                    end
                    summary = Report.statistics(paired)

                    [threshold.to_s, { 'dbt_ns' => summary.fetch('dbt_ns'), 'speedup' => summary.fetch('dbt_speedup') }]
                end

                result.merge('statistics' => {
                    'naive_ns' => Report.statistics(samples.map { |sample| { 'naive_ns' => sample.fetch('naive_ns') } }).fetch('naive_ns'),
                    'thresholds' => statistics
                })
            end

            summary = results.empty? ? [] : thresholds.map { |threshold| aggregate(results, threshold) }

            report.merge(
                'results' => results,
                'threshold_summary' => summary,
                'statistics_method' => {
                    'estimator' => 'geometric mean of per-workload median paired naive/DBT ratios',
                    'interval' => 'percentile-bootstrap; paired rounds resampled independently within each workload',
                    'confidence_level' => Report::CONFIDENCE_LEVEL,
                    'resamples' => Report::BOOTSTRAP_RESAMPLES,
                    'seed' => Report::BOOTSTRAP_SEED
                }
            )
        end

        def self.print(report, output: $stdout)
            report = analyze(report) unless report.key?('threshold_summary')
            output.puts format(TABLE_FORMAT, *HEADERS)

            rows(report).each do |row|
                output.puts format(TABLE_FORMAT, *row)
            end
        end

        def self.markdown(report)
            report = analyze(report) unless report.key?('threshold_summary')
            lines = [
                '# DBT threshold sweep', '', "Created: #{report.fetch('created_at')}", '',
                "| #{HEADERS.join(' | ')} |", '| ---: | ---: | ---: |'
            ]

            rows(report).each do |row|
                lines << "| #{row.join(' | ')} |"
            end

            lines.concat([
                '', 'Baseline: naive switch without decoded-block caching. Higher speedup is better.',
                'For each threshold, the aggregate is the geometric mean of per-workload median paired speedups.',
                'Each execution starts with cold caches. DBT time includes JIT setup and translation.',
                'Confidence intervals resample paired rounds independently within each workload, keeping the workload set fixed.',
                'With fewer than two samples in any workload, the aggregate confidence interval is unavailable.'
            ])
            lines.concat(['', "Run failed: #{report.fetch('error')}. Summary covers completed workloads only."]) if report.key?('error')

            lines.join("\n") + "\n"
        end

        def self.aggregate(results, threshold)
            ratios = results.map do |result|
                result.fetch('measurement').fetch('samples').map do |sample|
                    sample.fetch('naive_ns') / sample.fetch('dbt_ns').fetch(threshold.to_s).to_f
                end
            end
            center = geometric_mean(ratios.map { |values| Report.median(values) })
            interval = nil

            if ratios.all? { |values| values.length >= Report::MIN_INTERVAL_SAMPLES }
                random = Random.new(Report::BOOTSTRAP_SEED)
                estimates = Array.new(Report::BOOTSTRAP_RESAMPLES) do
                    medians = ratios.map do |values|
                        Report.median(Array.new(values.length) { values.fetch(random.rand(values.length)) })
                    end

                    geometric_mean(medians)
                end.sort
                tail = (1.0 - Report::CONFIDENCE_LEVEL) / 2.0
                interval = { 'low' => quantile(estimates, tail), 'high' => quantile(estimates, 1.0 - tail) }
            end

            { 'threshold' => threshold, 'geometric_mean' => center, 'ci95' => interval, 'workload_count' => results.length }
        end

        def self.geometric_mean(values)
            Math.exp(values.sum { |value| Math.log(value) } / values.length)
        end

        def self.quantile(sorted, probability)
            position = (sorted.length - 1) * probability
            lower = position.floor
            upper = position.ceil

            sorted.fetch(lower) + (sorted.fetch(upper) - sorted.fetch(lower)) * (position - lower)
        end

        def self.rows(report)
            report.fetch('threshold_summary').map do |summary|
                interval = summary.fetch('ci95')
                bounds = interval ? format('[%.3f, %.3f]', interval.fetch('low'), interval.fetch('high')) : '[n/a]'

                [summary.fetch('threshold'), format('%.3fx', summary.fetch('geometric_mean')), bounds]
            end
        end

        private_class_method :aggregate, :geometric_mean, :quantile, :rows
    end
end
