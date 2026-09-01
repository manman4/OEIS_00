#!/usr/bin/env ruby
# frozen_string_literal: true

# OEIS A398331
#
# a(n) is one-half of the number of permutations p of [n] having exactly
# k runs of adjacent entries differing by 1.  For A398331, k = 5 and
# n >= 2*k = 10.
#
# A run of a permutation p is a maximal contiguous block
# p(i), p(i+1), ..., p(j), where j > i, such that
# |p(h+1) - p(h)| = 1 for every i <= h < j.
#
# Let q = 1-y and
#
#   G(x,q) = Sum_{m>=0} m!*B(x,q)^m,
#   B(x,q) = x*(1-2*q*x+q*x^2)/(1-q*x^2).
#
# If F(z) = Sum_{m>=0} m!*z^m, then
#
#   z^2*F'(z) + (z-1)*F(z) + 1 = 0.
#
# Substitution of z = B gives a finite-width recurrence for the
# polynomials [x^n]G(x,q).  This program uses that recurrence and then
# obtains [y^k] from [y^k]q^h = (-1)^k*binomial(h,k).
#
# Usage:
#   ruby 398330_01.rb                 # k=5, b-file lines n=10..30
#   ruby 398330_01.rb 100             # k=5, b-file lines n=10..100
#   ruby 398330_01.rb --runs 4 100    # k=4, b-file lines n=8..100
#   ruby 398330_01.rb --check

module A398330
  DEFAULT_RUNS = 4
  DEFAULT_MAX_N = 30
  MAX_SUPPORTED_N = 1_000
  MAX_DIGITS = 1_000

  # Coefficients in x of the polynomial multiplying G_x.
  A_COEFFICIENTS = {
    2 => [[0, 1]],
    3 => [[1, -4]],
    4 => [[1, 1], [2, 4]],
    6 => [[2, -1], [3, -4]],
    7 => [[3, 4]],
    8 => [[3, -1]]
  }.freeze

  # Coefficients in x of the polynomial multiplying G.
  C_COEFFICIENTS = {
    1 => [[0, 1], [1, 4]],
    2 => [[1, -9]],
    3 => [[1, 5], [2, 4]],
    4 => [[2, -7]],
    5 => [[2, 3]],
    6 => [[3, 1]],
    7 => [[3, -1]]
  }.freeze

  # Inhomogeneous polynomial in the differential equation.
  R_COEFFICIENTS = {
    0 => [[0, 1]],
    1 => [[1, -4]],
    2 => [[1, 3]],
    3 => [[2, 4]],
    4 => [[2, -5]],
    6 => [[3, 1]]
  }.freeze

  # KNOWN_TERMS = [
  #   1_281,
  #   45_155,
  #   1_024_252,
  #   19_832_856,
  #   364_000_521,
  #   6_640_162_083,
  #   123_218_209_230,
  #   2_353_262_069_902,
  #   46_531_668_504_614
  # ].freeze

  class InputError < StandardError; end
  class CalculationError < StandardError; end

  module_function

  def add_kernel!(target, source, kernel, scale = 1)
    source.each_with_index do |source_coefficient, source_degree|
      next if source_coefficient.zero?

      kernel.each do |shift, kernel_coefficient|
        degree = source_degree + shift
        target.fill(0, target.length...degree + 1) if degree >= target.length
        target[degree] += scale * source_coefficient * kernel_coefficient
      end
    end
  end

  # Yields g[n], where g[n][h] = [x^n q^h]G(x,q).
  def each_q_row(maximum_n)
    return enum_for(__method__, maximum_n) unless block_given?

    rows = Array.new(maximum_n + 1)
    rows[0] = [1]
    yield 0, rows[0]

    1.upto(maximum_n) do |n|
      current = [0]

      kernel = R_COEFFICIENTS[n]
      add_kernel!(current, [1], kernel) if kernel

      A_COEFFICIENTS.each do |x_degree, q_kernel|
        source_n = n - x_degree + 1
        next if source_n < 1

        add_kernel!(current, rows[source_n], q_kernel, source_n)
      end

      C_COEFFICIENTS.each do |x_degree, q_kernel|
        source_n = n - x_degree
        next if source_n.negative?

        add_kernel!(current, rows[source_n], q_kernel)
      end

      current.pop while current.length > 1 && current[-1].zero?
      rows[n] = current
      yield n, current
    end
  end

  def fixed_binomials(maximum_h, k)
    values = Array.new(maximum_h + 1, 0)
    values[k] = 1
    (k + 1).upto(maximum_h) do |h|
      values[h] = values[h - 1] * h / (h - k)
    end
    values
  end

  def each_value(k, maximum_n)
    unless k.is_a?(Integer) && k.positive?
      raise InputError, "K must be a positive integer: #{k.inspect}"
    end
    unless maximum_n.is_a?(Integer) && maximum_n.between?(2 * k, MAX_SUPPORTED_N)
      raise InputError,
            "N must be in #{2 * k}..#{MAX_SUPPORTED_N}: #{maximum_n.inspect}"
    end
    return enum_for(__method__, k, maximum_n) unless block_given?

    choose = fixed_binomials(maximum_n / 2, k)
    sign = k.odd? ? -1 : 1

    each_q_row(maximum_n).each do |n, row|
      next if n < 2 * k

      count = sign * k.upto(row.length - 1).sum do |h|
        row[h] * choose[h]
      end
      unless count.even? && count >= 0
        raise CalculationError, "invalid count for n=#{n}, k=#{k}: #{count}"
      end

      yield n, count / 2
    end
  end

  def values(k, maximum_n)
    each_value(k, maximum_n).map { |_n, value| value }
  end

  def parse_integer(text, label)
    Integer(text, 10)
  rescue ArgumentError
    raise InputError, "#{label} must be an integer: #{text}"
  end

  def check
    actual = values(DEFAULT_RUNS, 18)
    unless actual == KNOWN_TERMS
      raise CalculationError,
            "known-term check failed:\nexpected #{KNOWN_TERMS.inspect}\n" \
            "     got #{actual.inspect}"
    end

    warn 'ok: A398331 terms for k=5 and n=10..18 agree'
  end

  def usage(program)
    <<~USAGE
      usage: #{program} [MAX_N]
             #{program} --runs K MAX_N
             #{program} --check

      The default is K=#{DEFAULT_RUNS}, MAX_N=#{DEFAULT_MAX_N}.
      Terms are printed in b-file format "n a(n)" for n=2*K..MAX_N.
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

    k, maximum_n =
      case arguments.length
      when 0
        [DEFAULT_RUNS, DEFAULT_MAX_N]
      when 1
        [DEFAULT_RUNS, parse_integer(arguments[0], 'N')]
      when 3
        raise InputError, usage(program) unless arguments[0] == '--runs'

        [parse_integer(arguments[1], 'K'), parse_integer(arguments[2], 'N')]
      else
        raise InputError, usage(program)
      end

    each_value(k, maximum_n).each do |n, value|
      break if value.to_s.size > MAX_DIGITS

      puts "#{n} #{value}"
    end
  end
end

if __FILE__ == $PROGRAM_NAME
  begin
    A398330.run(ARGV, $PROGRAM_NAME)
  rescue A398330::InputError, A398330::CalculationError => error
    warn "error: #{error.message}"
    exit 1
  end
end
