#!/usr/bin/env ruby
# frozen_string_literal: true

# OEIS A010030 -- direct implementation of the definition
#
# T(n,k), n >= 1 and 0 <= k <= floor(n/2), is half the number of
# permutations of [n] having floor(n/2)-k runs of consecutive pairs,
# in either the upward or downward direction.
#
# A consecutive pair is an adjacent pair of entries whose absolute
# difference is 1.  Such a pair is represented by its lower endpoint.
# Consecutive lower endpoints belong to the same run.  For example, in
#
#   1 4 5 6 2 3
#
# the pairs 45 and 56 form one run, while 23 forms a second run.
#
# This program follows that definition literally:
#
#   1. enumerate every permutation of [n];
#   2. record every adjacent pair whose entries differ by 1;
#   3. count the runs among those pairs;
#   4. collect permutations by their number of runs;
#   5. reverse the run-count order and divide every count by 2.
#
# It uses no generating function, recurrence, inclusion-exclusion, or
# symmetry pruning.  Its running time is therefore factorial in n.
#
# Usage:
#   ruby 010030_02.rb             # flattened rows n=1..8
#   ruby 010030_02.rb 9           # flattened rows n=1..9
#   ruby 010030_02.rb --rows 8    # one labeled row per line
#   ruby 010030_02.rb --term 8 2  # T(8,2)
#   ruby 010030_02.rb --check

module A010030Direct
  DEFAULT_MAX_N = 8
  MAX_SUPPORTED_N = 10

  KNOWN_ROWS = [
    [1],
    [1, 0],
    [3, 0],
    [3, 8, 1],
    [25, 28, 7],
    [17, 155, 143, 45],
    [259, 1005, 933, 323],
    [131, 2770, 7488, 7150, 2621]
  ].freeze

  class InputError < StandardError; end
  class CalculationError < StandardError; end

  module_function

  def parse_integer(text, label, minimum, maximum)
    value = Integer(text, 10)
    unless value.between?(minimum, maximum)
      raise InputError, "#{label} must be in #{minimum}..#{maximum}: #{text}"
    end

    value
  rescue ArgumentError
    raise InputError, "#{label} must be an integer: #{text}"
  end

  def validate_n(n)
    return if n.is_a?(Integer) && n.between?(1, MAX_SUPPORTED_N)

    raise InputError, "N must be in 1..#{MAX_SUPPORTED_N}: #{n.inspect}"
  end

  # Count the runs of consecutive pairs in one permutation.
  def run_count(permutation)
    # consecutive[lower] says that lower and lower+1 occur next to each
    # other in the permutation, in either order.  Index 0 is a sentinel.
    consecutive = Array.new(permutation.length, false)

    permutation.each_cons(2) do |left, right|
      next unless (left - right).abs == 1

      consecutive[[left, right].min] = true
    end

    runs = 0
    1.upto(permutation.length - 1) do |lower|
      runs += 1 if consecutive[lower] && !consecutive[lower - 1]
    end
    runs
  end

  # Return the undivided permutation counts indexed by number of runs.
  def run_distribution(n)
    validate_n(n)
    counts = Array.new(n / 2 + 1, 0)

    (1..n).to_a.permutation do |permutation|
      counts[run_count(permutation)] += 1
    end

    counts
  end

  def row(n)
    validate_n(n)
    return [1] if n == 1

    distribution = run_distribution(n)
    maximum_runs = n / 2

    0.upto(maximum_runs).map do |k|
      count = distribution[maximum_runs - k]
      if count.odd?
        raise CalculationError, "odd undivided count for n=#{n}, k=#{k}"
      end

      count / 2
    end
  end

  def value(n, k)
    validate_n(n)
    unless k.is_a?(Integer) && k.between?(0, n / 2)
      raise InputError, "K must be in 0..floor(N/2): #{k.inspect}"
    end

    row(n)[k]
  end

  def rows(maximum_n)
    validate_n(maximum_n)
    1.upto(maximum_n).map { |n| row(n) }
  end

  def check
    actual = rows(KNOWN_ROWS.length)
    unless actual == KNOWN_ROWS
      raise CalculationError,
            "check failed:\nexpected #{KNOWN_ROWS.inspect}\n" \
            "     got #{actual.inspect}"
    end

    warn 'ok: direct permutation enumeration agrees with A010030 for n=1..8'
  end

  def usage(program)
    <<~USAGE
      usage: #{program} [MAX_N]
             #{program} --upto MAX_N
             #{program} --rows MAX_N
             #{program} --term N K
             #{program} --check

      With no arguments, print the flattened rows n=1..#{DEFAULT_MAX_N}.
      N and MAX_N may be 1..#{MAX_SUPPORTED_N}.
      The calculation enumerates all n! permutations.
    USAGE
  end

  def run(arguments, program)
    if arguments == ['--help'] || arguments == ['-h']
      puts usage(program)
      return
    end

    if arguments == ['--check']
      check
      return
    end

    if arguments.first == '--term'
      raise InputError, usage(program) unless arguments.length == 3

      n = parse_integer(arguments[1], 'N', 1, MAX_SUPPORTED_N)
      k = parse_integer(arguments[2], 'K', 0, n / 2)
      puts value(n, k)
      return
    end

    mode, maximum_n =
      case arguments.length
      when 0
        [:flat, DEFAULT_MAX_N]
      when 1
        [:flat, parse_integer(arguments[0], 'MAX_N', 1, MAX_SUPPORTED_N)]
      when 2
        unless %w[--upto --rows].include?(arguments[0])
          raise InputError, usage(program)
        end

        selected_mode = arguments[0] == '--rows' ? :rows : :flat
        selected_n = parse_integer(arguments[1], 'MAX_N', 1, MAX_SUPPORTED_N)
        [selected_mode, selected_n]
      else
        raise InputError, usage(program)
      end

    triangle = rows(maximum_n)
    if mode == :rows
      triangle.each_with_index do |values, index|
        puts "n=#{index + 1}: #{values.join(', ')}"
      end
    else
      puts triangle.flatten.join(', ')
    end
  end
end

if __FILE__ == $PROGRAM_NAME
  begin
    A010030Direct.run(ARGV, $PROGRAM_NAME)
  rescue A010030Direct::InputError, A010030Direct::CalculationError => error
    warn "error: #{error.message}"
    exit 1
  end
end
