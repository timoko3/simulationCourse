require 'json'

module Benchmarks
    module Report
        NANOSECONDS_PER_MILLISECOND = 1_000_000.0
        RESULT_PRECISION = 3
        TABLE_FORMAT = '%-12s %28s %28s %28s %28s %26s %26s %26s'.freeze
        HEADERS = ['Workload', 'Naive ms [95% CI]', 'Cached switch ms [95% CI]',
            'Threaded ms [95% CI]', 'DBT ms [95% CI]', 'Cached speedup [95% CI]', 'Threaded speedup [95% CI]', 'DBT speedup [95% CI]'].freeze
        CONFIDENCE_LEVEL = 0.95
        BOOTSTRAP_RESAMPLES = 10_000
        BOOTSTRAP_SEED = 0
        MIN_INTERVAL_SAMPLES = 2

        def self.analyze(report)
            results = report.fetch('results').map do |result|
                result.merge('statistics' => statistics(result.fetch('measurement').fetch('samples')))
            end

            report.merge(
                'results' => results,
                'statistics_method' => {
                    'estimator' => 'median',
                    'interval' => 'percentile-bootstrap',
                    'confidence_level' => CONFIDENCE_LEVEL,
                    'resamples' => BOOTSTRAP_RESAMPLES,
                    'seed' => BOOTSTRAP_SEED
                }
            )
        end

        def self.write(report, directory)
            analyzed = analyze(report)

            File.write(File.join(directory, 'results.json'), JSON.pretty_generate(analyzed) + "\n")
            File.write(File.join(directory, 'report.md'), markdown(analyzed))

            analyzed
        end

        def self.print(report, output: $stdout)
            output.puts format(TABLE_FORMAT, *HEADERS)

            rows(report).each do |row|
                output.puts format(TABLE_FORMAT, *row)
            end
        end

        def self.markdown(report)
            lines = [
                '# Dispatch benchmarks', '', "Created: #{report.fetch('created_at')}", '',
                "| #{HEADERS.join(' | ')} |",
                "| #{(['---'] + Array.new(HEADERS.length - 1, '---:')).join(' | ')} |"
            ]

            rows(report).each do |row|
                lines << "| #{row.join(' | ')} |"
            end

            lines.concat(['', 'Times are medians. Speedups are medians of naive/mode ratios per measured round.',
                'Naive switch fetches and decodes every instruction; cached switch and threaded reuse decoded blocks. DBT includes JIT setup and translation.'])
            lines.concat([
                '', 'Brackets show approximate 95% bootstrap confidence intervals for the medians.',
                'Intervals assume independent representative samples; they do not include systematic errors.',
                'With one sample the interval is unavailable. Small samples give coarse uncertainty estimates.'
            ])
            lines.concat(['', "Run failed: #{report.fetch('error')}"]) if report.key?('error')

            lines.join("\n") + "\n"
        end

        def self.rows(report)
            report.fetch('results').map do |result|
                summary = result['statistics'] || statistics(result.fetch('measurement').fetch('samples'))

                [
                    result.fetch('name'),
                    format_estimate(summary['naive_ns'], scale: NANOSECONDS_PER_MILLISECOND),
                    format_estimate(summary['switch_ns'], scale: NANOSECONDS_PER_MILLISECOND),
                    format_estimate(summary['threaded_ns'], scale: NANOSECONDS_PER_MILLISECOND),
                    format_estimate(summary['dbt_ns'], scale: NANOSECONDS_PER_MILLISECOND),
                    format_estimate(summary['switch_speedup'], suffix: 'x'),
                    format_estimate(summary['threaded_speedup'], suffix: 'x'),
                    format_estimate(summary['dbt_speedup'], suffix: 'x')
                ]
            end
        end

        def self.median(values)
            raise ArgumentError, 'Cannot calculate a median of an empty sample' if values.empty?

            sorted = values.sort
            middle = sorted.length / 2

            return sorted[middle] if sorted.length.odd?

            lower = sorted[middle - 1]
            lower + (sorted[middle] - lower) / 2.0
        end

        def self.statistics(samples)
            summary = {}

            %w[naive_ns switch_ns threaded_ns dbt_ns].each do |key|
                values = samples.filter_map { |sample| sample[key] }
                summary[key] = estimate(values) unless values.empty?
            end

            if summary.key?('switch_ns') && summary.key?('threaded_ns')
                ratios = samples.map { |sample| sample.fetch('switch_ns') / sample.fetch('threaded_ns').to_f }
                summary['speedup'] = estimate(ratios)
            end

            if summary.key?('naive_ns')
                %w[switch threaded dbt].each do |mode|
                    key = "#{mode}_ns"
                    next unless summary.key?(key)

                    ratios = samples.map { |sample| sample.fetch('naive_ns') / sample.fetch(key).to_f }
                    summary["#{mode}_speedup"] = estimate(ratios)
                end
            end

            summary
        end

        def self.estimate(values)
            center = median(values)
            interval = bootstrap_interval(values) if values.length >= MIN_INTERVAL_SAMPLES
            half_width = (interval.fetch('high') - interval.fetch('low')) / 2.0 if interval

            {
                'sample_count' => values.length,
                'median' => center,
                'ci95' => interval,
                'relative_half_width_percent' => half_width ? (half_width / center) * 100.0 : nil
            }
        end

        def self.bootstrap_interval(values)
            if values.all? { |value| value == values.first }
                return { 'low' => values.first, 'high' => values.first }
            end

            random = Random.new(BOOTSTRAP_SEED)
            medians = Array.new(BOOTSTRAP_RESAMPLES) do
                resample = Array.new(values.length) { values.fetch(random.rand(values.length)) }
                median(resample)
            end.sort

            tail = (1.0 - CONFIDENCE_LEVEL) / 2.0

            { 'low' => quantile(medians, tail), 'high' => quantile(medians, 1.0 - tail) }
        end

        def self.quantile(sorted, probability)
            position = (sorted.length - 1) * probability
            lower = position.floor
            upper = position.ceil

            sorted.fetch(lower) + (sorted.fetch(upper) - sorted.fetch(lower)) * (position - lower)
        end

        def self.format_estimate(estimate, scale: 1.0, suffix: '')
            return '-' unless estimate

            center = format('%.*f%s', RESULT_PRECISION, estimate.fetch('median') / scale, suffix)
            interval = estimate.fetch('ci95')
            return "#{center} [n/a]" unless interval

            bounds = %w[low high].map { |key| format('%.*f', RESULT_PRECISION, interval.fetch(key) / scale) }

            "#{center} [#{bounds.join(', ')}]"
        end

        private_class_method :rows, :estimate, :bootstrap_interval, :quantile, :format_estimate
    end
end
