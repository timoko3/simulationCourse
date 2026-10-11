require 'optparse'
require 'json'
require_relative 'suite'
require_relative 'report'

module Benchmarks
    module UI
        PROJECT_ROOT = File.expand_path('..', __dir__).freeze
        BUILD_PRESET = 'benchmark'.freeze

        def self.run(arguments)
            options = read_options(arguments)
            return 0 unless options

            options[:execution][:binary] = prepare_binary(options[:execution][:binary])

            suite = Suite.new(**options.fetch(:execution))
            report = suite.run(cases: options.fetch(:cases), output: options.fetch(:output))

            Report.print(report)
            plot_results(options.fetch(:output)) if options[:plot]
            puts "Results: #{File.expand_path(options.fetch(:output))}"

            0
        rescue OptionParser::ParseError, JSON::ParserError, KeyError, ArgumentError, SystemCallError, RuntimeError => error
            warn "Benchmark failed: #{error.message}"
            1
        end

        def self.read_options(arguments)
            arguments = arguments.dup
            options = { execution: {}, cases: [] }
            parser = command_line(options)

            parser.parse!(arguments)
            return nil if options[:information_only]

            options[:execution][:mode] ||= arguments.shift
            raise OptionParser::InvalidArgument, arguments.join(' ') unless arguments.empty?

            mode = options[:execution][:mode]
            raise OptionParser::MissingArgument, 'MODE' unless mode

            if options[:plot] && mode != 'comparison'
                raise OptionParser::InvalidArgument, '--plot requires comparison mode'
            end

            request = options.fetch(:execution).reject { |key, _value| key == :binary }
            Suite.validate_request(**request)
            Suite.select_cases(options.fetch(:cases))

            timestamp = Time.now.utc.strftime('%Y%m%dT%H%M%S%NZ')
            options[:output] ||= File.expand_path("../benchmark-results/#{timestamp}", __dir__)

            options
        end

        def self.command_line(options)
            OptionParser.new do |cli|
                cli.banner = 'Usage: ruby benchmarks/run.rb MODE [options]'

                cli.on('--mode MODE', Suite::MODES, 'naive, threaded, switch or comparison') do |value|
                    options[:execution][:mode] = value
                end

                cli.on('--binary PATH', 'Use an existing executable instead of building') do |value|
                    options[:execution][:binary] = value
                end

                cli.on('--case NAME', 'Select workload; repeat to select several (default: all)') do |value|
                    options[:cases] << value
                end

                cli.on('--samples N', Integer, 'Override the C++ sample count') do |value|
                    options[:execution][:samples] = value
                end

                cli.on('--warmups N', Integer, 'Override the C++ warmup count') do |value|
                    options[:execution][:warmups] = value
                end

                cli.on('--timeout SECONDS', Float, 'Timeout for each child process') do |value|
                    options[:execution][:timeout] = value
                end

                cli.on('--output DIRECTORY', 'New directory for ELF, logs and results') do |value|
                    options[:output] = value
                end

                cli.on('--plot', 'Save a normalized PNG/SVG chart (requires Python 3 and Matplotlib)') do
                    options[:plot] = true
                end

                cli.on('--list', 'List workloads') do
                    puts Suite.names
                    options[:information_only] = true
                end

                cli.on('-h', '--help', 'Show help') do
                    puts cli
                    options[:information_only] = true
                end
            end
        end

        def self.plot_results(directory)
            command = ['python3', File.join(__dir__, 'plot.py'), File.join(directory, 'results.json')]
            raise 'Plot failed; measurements are saved in results.json' unless system(*command)
        end

        def self.prepare_binary(explicit_path)
            return explicit_path if explicit_path

            directory = build_directory
            build_benchmark(directory)

            executable = Gem.win_platform? ? 'simcourse_bench.exe' : 'simcourse_bench'

            File.join(directory, 'benchmarks', executable)
        end

        def self.build_directory
            presets = JSON.parse(File.read(File.join(PROJECT_ROOT, 'CMakePresets.json')))
            preset = presets.fetch('configurePresets').find { |entry| entry['name'] == BUILD_PRESET }
            raise ArgumentError, "CMake preset not found: #{BUILD_PRESET}" unless preset

            template = preset.fetch('binaryDir')
            directory = template.gsub('${sourceDir}', PROJECT_ROOT).gsub('${presetName}', BUILD_PRESET)
            directory = directory.gsub(/\$env\{([^}]+)\}/) do
                variable = Regexp.last_match(1)
                ENV.fetch(variable) { raise ArgumentError, "Environment variable is not set: #{variable}" }
            end

            raise ArgumentError, 'Unsupported CMake binaryDir macro' if directory.include?('${') || directory.include?('$env{')

            File.expand_path(directory, PROJECT_ROOT)
        end

        def self.build_benchmark(directory)
            cache = File.join(directory, 'CMakeCache.txt')
            build_file = File.join(directory, 'build.ninja')
            presets = File.join(PROJECT_ROOT, 'CMakePresets.json')
            needs_configuration = !File.file?(cache) || !File.file?(build_file) ||
                File.mtime(presets) > File.mtime(cache)

            commands = []
            commands << ['cmake', '--preset', BUILD_PRESET] if needs_configuration
            commands << ['cmake', '--build', '--preset', BUILD_PRESET]

            commands.each do |command|
                success = system(*command, chdir: PROJECT_ROOT, out: $stderr)
                raise "Build failed: #{command.join(' ')}" unless success
            end
        end
    end
end

exit Benchmarks::UI.run(ARGV) if $PROGRAM_NAME == __FILE__
