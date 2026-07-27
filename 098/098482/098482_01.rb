#!/usr/bin/env ruby
# frozen_string_literal: true

# A387599 is the case s = 4 of the general family below.
#
# Count lattice paths from (0, 0) to (n, n) using the three steps
#
#   (s+1-r, r), (r, s+1-r), (1, 1),
#
# for fixed integers s >= 0 and 0 <= r < (s+1)/2.  This program uses
# dynamic programming on lattice points; it does not use a binomial sum
# or a closed formula.
#
# General proof:
# Fix integers s >= 0 and 0 <= r < (s+1)/2, and consider paths from
# (0, 0) to (n, n) with steps
#
#   A = (s+1-r, r), B = (r, s+1-r), C = (1, 1).
#
# Let u, v, and w be the numbers of A-, B-, and C-steps.  The endpoint
# conditions are
#
#   (s+1-r)*u + r*v + w = n,
#   r*u + (s+1-r)*v + w = n.
#
# Subtraction gives (s+1-2*r)*(u-v) = 0.  The restriction on r makes
# s+1-2*r positive, so u = v = k.  Either endpoint equation then gives
#
#   w = n-(s+1)*k.
#
# Conversely, any ordering of k A-steps, k B-steps, and n-(s+1)*k
# C-steps ends at (n, n).  Thus the number of paths with a fixed k is
#
#   (n-(s-1)*k)! / (k!^2 * (n-(s+1)*k)!)
#     = binomial(n-(s-1)*k, k) * binomial(n-s*k, k).
#
# Summing over k proves that the path count is independent of r.  Writing
# j = n-(s+1)*k and summing the corresponding multinomial coefficients
# gives the ordinary generating function
#
#   1 / sqrt((1-x)^2 - 4*x^(s+1)).
#
# The dynamic program constructs every permitted step ordering exactly
# once by extending paths from each reachable lattice point; it does not
# use the formula in the proof.
#
# Usage:
#   ruby 387599_01.rb n s r
#   ruby 387599_01.rb upto N s r
#   ruby 387599_01.rb --verify s [max_n]
#
# Examples:
#   ruby 387599_01.rb 5 4 0
#   ruby 387599_01.rb 20 4 2
#   ruby 387599_01.rb upto 20 4 1
#   ruby 387599_01.rb --verify 4

def usage!
  program = File.basename($PROGRAM_NAME)
  warn "Usage:"
  warn "  ruby #{program} n s r"
  warn "  ruby #{program} upto N s r"
  warn "  ruby #{program} --verify s [max_n]"
  warn "  n, N: nonnegative integer"
  warn "  s: nonnegative integer"
  warn "  r: integer satisfying 0 <= r < (s+1)/2"
  warn "  upto: print a(0),a(1),...,a(N)"
  warn "  --verify: compare all valid r for n=0..max_n (default: 40)"
  exit 1
end

def valid_r?(s, r)
  r >= 0 && 2 * r < s + 1
end

def count_paths_upto(max_n, s, r)
  raise ArgumentError, "N must be nonnegative" if max_n.negative?
  raise ArgumentError, "s must be nonnegative" if s.negative?
  raise ArgumentError, "r must satisfy 0 <= r < (s+1)/2" unless valid_r?(s, r)

  steps = [
    [s + 1 - r, r],
    [r, s + 1 - r],
    [1, 1]
  ].freeze

  # The rolling buffer has one more row than the largest relevant
  # x-increment.  When a step has dx = 0, increasing y-order ensures that
  # its contribution is processed later in the same row.
  max_dx = [steps.map(&:first).max, max_n].min
  rows = Array.new(max_dx + 1){Array.new(max_n + 1, 0)}
  rows[0][0] = 1
  answers = Array.new(max_n + 1, 0)

  0.upto(max_n){|x|
    row = rows[x % rows.length]

    0.upto(max_n){|y|
      count = row[y]
      next if count.zero?

      steps.each{|dx, dy|
        next_x = x + dx
        next_y = y + dy
        next if next_x > max_n || next_y > max_n

        rows[next_x % rows.length][next_y] += count
      }
    }

    answers[x] = row[x]
    row.fill(0)
  }

  answers
end

def count_paths(n, s, r)
  count_paths_upto(n, s, r)[n]
end

def verify_r_agreement(s, max_n = 40)
  raise ArgumentError, "s must be nonnegative" if s.negative?
  raise ArgumentError, "max_n must be nonnegative" if max_n.negative?

  r_values = 0.upto(s / 2).to_a
  sequences = r_values.map{|r| count_paths_upto(max_n, s, r)}

  0.upto(max_n){|n|
    counts = sequences.map{|sequence| sequence[n]}
    next if counts.uniq.length == 1

    details = r_values.zip(counts).map{|r, count| "r=#{r}: #{count}" }.join(", ")
    raise "path counts differ at s=#{s}, n=#{n} (#{details})"
  }

  true
end

if __FILE__ == $PROGRAM_NAME
  if %w[upto --upto].include?(ARGV.first)
    usage! unless ARGV.length == 4

    begin
      max_n = Integer(ARGV[1], 10)
      s = Integer(ARGV[2], 10)
      r = Integer(ARGV[3], 10)
      puts count_paths_upto(max_n, s, r).join(", ")
    rescue ArgumentError => error
      abort error.message
    end
    exit
  end

  if ARGV.first == "--verify"
    usage! unless ARGV.length.between?(2, 3)

    begin
      s = Integer(ARGV[1], 10)
      max_n = Integer(ARGV[2] || "40", 10)
      verify_r_agreement(s, max_n)
    rescue ArgumentError => error
      abort error.message
    end

    r_values = 0.upto(s / 2).to_a.join(",")
    puts "Verified: r=#{r_values} agree for s=#{s}, n=0..#{max_n}."
    exit
  end

  usage! if ARGV.length != 3 || ARGV.any?{|arg| %w[-h --help].include?(arg)}

  begin
    n = Integer(ARGV[0], 10)
    s = Integer(ARGV[1], 10)
    r = Integer(ARGV[2], 10)
  rescue ArgumentError
    usage!
  end

  abort "n must be nonnegative" if n.negative?
  abort "s must be nonnegative" if s.negative?
  abort "r must satisfy 0 <= r < (s+1)/2" unless valid_r?(s, r)

  puts count_paths(n, s, r)
end
