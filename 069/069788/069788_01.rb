#!/usr/bin/env ruby
# frozen_string_literal: true

# A069788
# Number of lozenge tilings of the hexagon with side lengths
# n, n+1, n, n+1, n, n+1 and its central unit triangle removed.
#
# The product formulas below are the odd- and even-n cases obtained from
# Theorem 20 of Helfgott and Gessel, "Tilings of Diamonds and Hexagons
# with Defects" (1999).
#
# Usage:
#   ruby 069788_01.rb       # n = 0..12
#   ruby 069788_01.rb 20    # n = 0..20

def exact_quotient(numerator, denominator)
  quotient, remainder = numerator.divmod(denominator)
  raise ArithmeticError, "nonexact division" unless remainder.zero?

  quotient
end

def a069788(n)
  unless n.is_a?(Integer) && n >= 0
    raise ArgumentError, "n must be a nonnegative integer"
  end

  numerator = 1
  denominator = 1

  if n.odd?
    q = n / 2

    j = 0
    q.downto(0){|k|
      numerator *= (n - k)**(4 * j + 1)
      j += 1
    }

    j = q
    1.upto(q + 1){|k|
      numerator *= (n + k)**(4 * j + 1)
      j -= 1
    }

    denominator <<= (2 * q) * (2 * q + 1)
    1.upto(q){|k|
      denominator *= (2 * k + 1)**(8 * (q - k) + 2)
    }
  else
    q = n / 2

    j = 1
    (q - 1).downto(0){|k|
      numerator *= (n - k)**(4 * j - 1)
      j += 1
    }

    j = q
    1.upto(q){|k|
      numerator *= (n + k)**(4 * j - 1)
      j -= 1
    }

    denominator <<= (2 * q - 1) * (2 * q)
    1.upto(q - 1){|k|
      denominator *= (2 * k + 1)**(8 * (q - k) - 2)
    }
  end

  exact_quotient(numerator, denominator)
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 12).to_i
  raise ArgumentError, "limit must be nonnegative" if limit.negative?

  0.upto(limit){|n|
    i = a069788(n)
    break if i.to_s.size > 1000
    puts "#{n} #{i}"
  }
end
