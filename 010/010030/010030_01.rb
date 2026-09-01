#!/usr/bin/env ruby
# frozen_string_literal: true

# OEIS A010030
#
# Irregular triangle read by rows.  T(n,k), n >= 1 and
# 0 <= k <= floor(n/2), is half the number of permutations of [n] with
# floor(n/2)-k runs of consecutive pairs, in either direction.
#
# For example, the consecutive pairs of 145623 are 45, 56, and 23.
# The first two form one run and 23 forms another, so this permutation has
# two runs.
#
# Let q = 1-y.  The generating function quoted in A010030 can be written as
#
#   H(x,y) = sum(m >= 0, m! * B(x,y)^m),
#   B(x,y) = x * (1 - 2*q*x + q*x^2) / (1 - q*x^2).
#
# The coefficient [x^n y^r] H is the undivided number of permutations with
# r runs.  In B^m choose:
#
#   a factors contributing -2*q*x,
#   b factors contributing  q*x^2, and
#   the term (q*x^2)^c from (1-q*x^2)^(-m).
#
# Thus n = m+a+2*b+2*c and the power of q is j = a+b+c.  This gives a
# finite integer sum.  Finally,
#
#   [y^r] q^j = [y^r] (1-y)^j = (-1)^r * binomial(j,r).
#
# Reversing a permutation preserves its runs and pairs permutations without
# a fixed point when n > 1, explaining the division by 2.  The singleton
# row n=1 is the exceptional initial row [1].
#
# Usage:
#   ruby 010030_01.rb             # flattened rows n=1..10
#   ruby 010030_01.rb 20          # flattened rows n=1..20
#   ruby 010030_01.rb --rows 10   # one labeled row per line
#   ruby 010030_01.rb --term 8 2  # T(8,2)
#   ruby 010030_01.rb --check

module A010030
  DEFAULT_MAX_N = 10
  MAX_SUPPORTED_N = 200

  KNOWN_ROWS = [
    [1],
    [1, 0],
    [3, 0],
    [3, 8, 1],
    [25, 28, 7],
    [17, 155, 143, 45],
    [259, 1005, 933, 323],
    [131, 2770, 7488, 7150, 2621],
    [3177, 27_978, 64_164, 62_310, 23_811],
    [1281, 51_433, 294_602, 619_986, 607_445, 239_653]
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

  def binomial_table(n)
    table = Array.new(n + 1) { Array.new(n + 1, 0) }
    table[0][0] = 1

    1.upto(n) do |i|
      table[i][0] = 1
      table[i][i] = 1
      1.upto(i - 1) do |j|
        table[i][j] = table[i - 1][j - 1] + table[i - 1][j]
      end
    end

    table
  end

  # Return the undivided counts indexed by the number of runs.
  def run_distribution(n)
    choose = binomial_table(n)
    maximum_runs = n / 2
    q_coefficients = Array.new(maximum_runs + 1, 0)
    factorial = 1

    1.upto(n) do |m|
      factorial *= m

      0.upto(m) do |a|
        0.upto(m - a) do |b|
          remainder = n - m - a - 2 * b
          next if remainder.negative? || remainder.odd?

          c = remainder / 2
          q_power = a + b + c
          multinomial = choose[m][a] * choose[m - a][b]
          negative_binomial = choose[m + c - 1][c]

          q_coefficients[q_power] +=
            factorial * multinomial * ((-2)**a) * negative_binomial
        end
      end
    end

    counts = Array.new(maximum_runs + 1, 0)
    0.upto(maximum_runs) do |runs|
      coefficient = 0
      runs.upto(maximum_runs) do |q_power|
        coefficient += q_coefficients[q_power] * choose[q_power][runs]
      end
      counts[runs] = runs.odd? ? -coefficient : coefficient
    end

    expected_total = factorial
    unless counts.none?(&:negative?) && counts.sum == expected_total
      raise CalculationError, "invalid run distribution for n=#{n}"
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
      raise CalculationError, "odd undivided count for n=#{n}, k=#{k}" if count.odd?

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

  def direct_run_distribution(n)
    counts = Array.new(n / 2 + 1, 0)

    (1..n).to_a.permutation do |permutation|
      consecutive_edges = {}
      permutation.each_cons(2) do |left, right|
        consecutive_edges[[left, right].min] = true if (left - right).abs == 1
      end

      runs = consecutive_edges.count do |lower_endpoint, _present|
        !consecutive_edges.key?(lower_endpoint - 1)
      end
      counts[runs] += 1
    end

    counts
  end

  def check
    actual_rows = rows(KNOWN_ROWS.length)
    unless actual_rows == KNOWN_ROWS
      raise CalculationError,
            "known-row check failed:\nexpected #{KNOWN_ROWS.inspect}\n" \
            "     got #{actual_rows.inspect}"
    end

    1.upto(8) do |n|
      formula = run_distribution(n)
      direct = direct_run_distribution(n)
      next if formula == direct

      raise CalculationError,
            "direct check failed for n=#{n}: expected #{direct.inspect}, " \
            "got #{formula.inspect}"
    end

    warn 'ok: A010030 known rows n=1..10 and direct counts n=1..8 agree'
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
    cnt = 1
    if mode == :rows
      triangle.each_with_index do |values, index|
        puts "n=#{index + 1}: #{values.join(', ')}"
      end
    else
      triangle.each{|row|
        row.each{|i|
          break if i.to_s.size > 1000
          print cnt
          print ' '
          puts i
          cnt += 1
        }
      }
    end
  end
end

if __FILE__ == $PROGRAM_NAME
  begin
    A010030.run(ARGV, $PROGRAM_NAME)
  rescue A010030::InputError, A010030::CalculationError => error
    warn "error: #{error.message}"
    exit 1
  end
end
