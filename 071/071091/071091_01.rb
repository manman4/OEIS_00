#!/usr/bin/env ruby
# frozen_string_literal: true

# A071091
# Number of lozenge tilings of a hexagon with alternating side lengths
# 2*n and 2*n+3, with the middle unit triangle removed from each of the
# three longer sides.
#
# Formula from Corollary 2 of Theresia Eisenkoelbl,
# "Rhombus Tilings of a Hexagon with Three Fixed Border Tiles" (1999):
#
#   a(n) = (n+1)^3*(3*n+1)*(3*n+2)^2*(n+2)_(2*n)^6
#          *S(2*n)^3*S(6*n+2)/S(4*n+2)^3,
#
# where (x)_m is the rising factorial and S(m) = Product_{k=0..m} k!.
#
# Usage:
#   ruby 071091_01.rb       # n = 0..6
#   ruby 071091_01.rb 20    # n = 0..20

def rising_factorial(start, length)
  0.upto(length - 1).reduce(1){|product, k| product * (start + k)}
end

def superfactorial(n)
  factorial = 1
  1.upto(n).reduce(1){|product, k|
    factorial *= k
    product * factorial
  }
end

def exact_quotient(numerator, denominator)
  quotient, remainder = numerator.divmod(denominator)
  raise ArithmeticError, "nonexact division" unless remainder.zero?

  quotient
end

def a071091(n)
  unless n.is_a?(Integer) && n >= 0
    raise ArgumentError, "n must be a nonnegative integer"
  end

  numerator =
    (n + 1)**3 *
    (3 * n + 1) *
    (3 * n + 2)**2 *
    rising_factorial(n + 2, 2 * n)**6 *
    superfactorial(2 * n)**3 *
    superfactorial(6 * n + 2)
  denominator = superfactorial(4 * n + 2)**3

  exact_quotient(numerator, denominator)
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 6).to_i
  raise ArgumentError, "limit must be nonnegative" if limit.negative?

  0.upto(limit){|n|
    value = a071091(n)
    break if value.to_s.size > 1000
    puts "#{n} #{value}"
  }
end
