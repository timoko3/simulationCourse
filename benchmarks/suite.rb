require 'digest'
require 'fileutils'
require 'json'
require 'rbconfig'
require 'time'
require_relative 'report'

module Benchmarks
    class Suite
        MODES = %w[threaded switch comparison].freeze
        WORKLOADS = File.join(__dir__, 'workloads').freeze
        DEFAULT_TIMEOUT = 60
        ERROR_TAIL_BYTES = 8192

        def self.names
            Dir.glob(File.join(WORKLOADS, '*.rb')).sort.map { |path| File.basename(path, '.rb') }
        end

        def self.validate_request(mode:, samples: nil, warmups: nil, timeout: DEFAULT_TIMEOUT)
            raise ArgumentError, "Unknown mode: #{mode}" unless MODES.include?(mode)

            if samples && !samples.positive?
                raise ArgumentError, 'Samples must be a positive integer'
            end

            if warmups && warmups.negative?
                raise ArgumentError, 'Warmups must be a nonnegative integer'
            end

            raise ArgumentError, 'Timeout must be positive and finite' unless timeout.finite? && timeout.positive?
        end

        def self.select_cases(cases)
            selected = cases.empty? ? names : cases.uniq
            raise ArgumentError, 'No workloads found' if selected.empty?

            unknown = selected - names
            raise ArgumentError, "Unknown workloads: #{unknown.join(', ')}" unless unknown.empty?

            selected
        end

        def initialize(binary:, mode:, samples: nil, warmups: nil, timeout: DEFAULT_TIMEOUT)
            self.class.validate_request(mode: mode, samples: samples, warmups: warmups, timeout: timeout)

            @binary = File.expand_path(binary)
            @mode = mode
            @samples = samples
            @warmups = warmups
            @timeout = timeout

            unless File.file?(@binary) && File.executable?(@binary)
                raise ArgumentError, "Benchmark executable not found: #{@binary}"
            end
        end

        def run(cases:, output:)
            names = self.class.select_cases(cases)

            report = {
                'created_at' => Time.now.utc.iso8601,
                'binary' => @binary,
                'binary_sha256' => Digest::SHA256.file(@binary).hexdigest,
                'results' => []
            }

            directory = File.expand_path(output)
            FileUtils.mkdir_p(File.dirname(directory))
            Dir.mkdir(directory)

            failure = nil

            begin
                names.each do |name|
                    report['results'] << run_case(name, directory)
                end
            rescue StandardError => error
                report['error'] = error.message
                failure = error
                raise
            ensure
                report = save_report(report, directory, failure)
            end

            report
        end

        private

        def run_case(name, directory)
            source = File.join(WORKLOADS, "#{name}.rb")
            program = File.join(directory, "#{name}.elf")

            run_command([RbConfig.ruby, source, program], File.join(directory, "#{name}.generate"))
            raise 'Generator did not produce an ELF' unless File.file?(program)

            command = benchmark_command(program)
            log = File.join(directory, "#{name}.measure")
            run_command(command, log)
            measurement = JSON.parse(File.read("#{log}.stdout", encoding: 'UTF-8'))
            validate_measurement(measurement)

            {
                'name' => name,
                'source_sha256' => Digest::SHA256.file(source).hexdigest,
                'elf_sha256' => Digest::SHA256.file(program).hexdigest,
                'command' => command,
                'measurement' => measurement
            }
        rescue StandardError => error
            raise "#{name}: #{error.message}"
        end

        def benchmark_command(program)
            command = [@binary, '--mode', @mode, '--program', program]
            command.concat(['--samples', @samples.to_s]) if @samples
            command.concat(['--warmups', @warmups.to_s]) if @warmups

            command
        end

        def run_command(command, log_path)
            status, timed_out = execute_command(command, log_path)

            raise "command exceeded #{@timeout} seconds: #{command.first}" if timed_out

            unless status.success?
                reason = status.signaled? ? "signal #{status.termsig}" : status.exitstatus.to_s
                raise "command failed (#{reason}): #{error_tail("#{log_path}.stderr")}"
            end
        end

        def execute_command(command, log_path)
            File.open("#{log_path}.stdout", 'wb') do |output|
                File.open("#{log_path}.stderr", 'wb') do |errors|
                    process_options = { in: File::NULL, out: output, err: errors }
                    process_options[:pgroup] = true unless Gem.win_platform?
                    pid = Process.spawn(*command, **process_options)
                    waiter = Thread.new { Process.waitpid2(pid).last }

                    begin
                        timed_out = waiter.join(@timeout).nil?
                        terminate(pid) if timed_out

                        [waiter.value, timed_out]
                    ensure
                        # Also stop descendants when the process leader has already exited.
                        terminate(pid) unless Gem.win_platform? && !waiter.alive?
                        waiter.join
                    end
                end
            end
        end

        def terminate(pid)
            target = Gem.win_platform? ? pid : -pid
            Process.kill('KILL', target)
        rescue Errno::ESRCH
            # The process or its group has already exited.
        end

        def error_tail(path)
            File.open(path, 'rb') do |file|
                file.seek([file.size - ERROR_TAIL_BYTES, 0].max)
                file.read.force_encoding('UTF-8').scrub.strip
            end
        end

        def save_report(report, directory, failure)
            Report.write(report, directory)
        rescue StandardError => error
            raise unless failure

            warn "Could not save benchmark report in #{directory}: #{error.message}"
            report
        end

        def validate_measurement(measurement)
            expected_keys = {
                'switch' => ['switch_ns'],
                'threaded' => ['threaded_ns'],
                'comparison' => %w[switch_ns threaded_ns]
            }.fetch(@mode)

            valid = measurement.is_a?(Hash) && measurement['mode'] == @mode &&
                measurement['cache'] == 'cold' && measurement['samples'].is_a?(Array) &&
                !measurement['samples'].empty?

            raise 'Invalid benchmark response' unless valid
            raise 'Unexpected sample count' if @samples && measurement['samples'].length != @samples
            raise 'Unexpected warmup count' if @warmups && measurement['warmups'] != @warmups

            measurement['samples'].each do |sample|
                valid = sample.is_a?(Hash) && sample.keys.sort == expected_keys.sort &&
                    sample.values.all? { |value| value.is_a?(Numeric) && value.finite? && value.positive? }

                raise 'Invalid benchmark sample' unless valid
            end
        end
    end
end
