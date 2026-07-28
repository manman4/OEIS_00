#!/usr/bin/env ruby
# frozen_string_literal: true

# A071096
# Number of lozenge tilings of the hexagon with side lengths
# n, n+1, n+2, n, n+1, n+2.
#
# MacMahon's formula gives
#
#   Product_{i=0..n-1} Product_{j=0..n} Product_{k=0..n+1}
#     (i+j+k+2)/(i+j+k+1).
#
# For fixed i and j, the product over k telescopes to
# (i+j+n+3)/(i+j+1), which is used below.
#
# Usage:
#   ruby 071096_01.rb       # n = 0..12
#   ruby 071096_01.rb 30    # n = 0..30

def exact_quotient(numerator, denominator)
  quotient, remainder = numerator.divmod(denominator)
  raise ArithmeticError, "nonexact division" unless remainder.zero?

  quotient
end

def a071096(n)
  unless n.is_a?(Integer) && n >= 0
    raise ArgumentError, "n must be a nonnegative integer"
  end

  numerator = 1
  denominator = 1

  0.upto(n - 1){|i|
    0.upto(n){|j|
      numerator *= i + j + n + 3
      denominator *= i + j + 1
    }
  }

  exact_quotient(numerator, denominator)
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 12).to_i
  raise ArgumentError, "limit must be nonnegative" if limit.negative?

  0.upto(limit){|n|
    value = a071096(n)
    break if value.to_s.size > 1000
    puts "#{n} #{value}"
  }
end
