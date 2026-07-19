#!/usr/bin/env ruby
# frozen_string_literal: true

# A019317
# Place n queens on an n X n board so as to leave as many unattacked vacant
# squares as possible.  Count solutions up to the eight symmetries of a square.
#
# This is deliberately a straightforward exact search.  The published optimum
# Free(n) is used as a branch-and-bound target for n <= 16.  For other n the
# program discovers the optimum, but that means a much larger search.
#
# Output status:
#   OK         Both the computed A001366 target and A019317 agree with the
#              stored reference data (currently n <= 16).
#   expected=  At least one of A001366/A019317 does not agree with its stored
#              reference value.
# No status is printed when this Ruby version has no stored reference value.
# A019317, its orbit-size breakdown, and "all" are still computed in that case.

KNOWN_FREE = {
  1 => 0,  2 => 0,   3 => 0,   4 => 1,
  5 => 3,  6 => 5,   7 => 7,   8 => 11,
  9 => 18, 10 => 22, 11 => 30, 12 => 36,
  13 => 47, 14 => 56, 15 => 72, 16 => 82
}.freeze

KNOWN_A019317 = {
  1 => 1,  2 => 2,  3 => 16, 4 => 25,
  5 => 1,  6 => 3,  7 => 38, 8 => 7,
  9 => 1, 10 => 1, 11 => 2, 12 => 7,
  13 => 1, 14 => 4, 15 => 3, 16 => 1
}.freeze

class NonDominatingQueens
  ORBIT_SIZES = [1, 2, 4, 8].freeze
  ORBIT_INDEX = ORBIT_SIZES.each_with_index.to_h.freeze
  Result = Struct.new(
    :n, :free, :solutions, :all_solutions, :orbit_counts, :nodes,
    keyword_init: true
  )
  POPCOUNT_16 = Array.new(1 << 16, 0).tap do |table|
    1.upto(table.length - 1) { |x| table[x] = table[x >> 1] + (x & 1) }
  end.freeze

  def initialize(n, known_free: KNOWN_FREE[n])
    raise ArgumentError, "n must be positive" unless n.positive?

    @n = n
    @size = n * n
    @queen_count = n
    @attack_masks = build_attack_masks
    @transforms = build_transforms
    @minimum_image = Array.new(@size) do |square|
      @transforms.map { |transform| transform[square] }.min
    end
    @canonical_pair = build_canonical_pairs
    @chosen = Array.new(n)
    @nodes = 0
    @solutions = 0
    @all_solutions = 0
    @orbit_counts = Array.new(ORBIT_SIZES.length, 0)
    @best_attacked = known_free.nil? ? @size : @size - known_free
  end

  def solve
    search(0, 0, 0, 0)
    verify_orbit_counts!
    Result.new(
      n: @n,
      free: @size - @best_attacked,
      solutions: @solutions,
      all_solutions: @all_solutions,
      orbit_counts: @orbit_counts.dup.freeze,
      nodes: @nodes
    )
  end

  private

  def build_attack_masks
    Array.new(@size) do |square|
      qr, qc = square.divmod(@n)
      mask = 0

      @n.times do |r|
        @n.times do |c|
          if r == qr || c == qc || (r - qr).abs == (c - qc).abs
            mask |= 1 << (r * @n + c)
          end
        end
      end
      mask
    end
  end

  def build_transforms
    Array.new(8) { Array.new(@size) }.tap do |maps|
      @size.times do |square|
        r, c = square.divmod(@n)
        coordinates = [
          [r, c], [c, @n - 1 - r],
          [@n - 1 - r, @n - 1 - c], [@n - 1 - c, r],
          [r, @n - 1 - c], [@n - 1 - c, @n - 1 - r],
          [@n - 1 - r, c], [c, r]
        ]
        coordinates.each_with_index { |(rr, cc), i| maps[i][square] = rr * @n + cc }
      end
    end
  end

  def build_canonical_pairs
    # For a sorted prefix P and a symmetry g, sorted(g(P)) < P can never be
    # repaired by appending larger original squares.  Cache this test for the
    # first two queens; the three-queen version below is allocation-free.
    pairs = Array.new(@size * @size, false)
    @size.times do |first|
      (first + 1...@size).each do |second|
        pairs[first * @size + second] = @transforms.all? do |transform|
          a = transform[first]
          b = transform[second]
          a, b = b, a if a > b
          first < a || (first == a && second <= b)
        end
      end
    end
    pairs
  end

  def search(start_square, depth, attacked, attacked_count)
    @nodes += 1

    if depth == @queen_count
      if attacked_count < @best_attacked
        @best_attacked = attacked_count
        @solutions = 0
        @all_solutions = 0
        @orbit_counts.fill(0)
      end
      return unless attacked_count == @best_attacked

      orbit_size = canonical_orbit_size
      if orbit_size
        @solutions += 1
        @all_solutions += orbit_size
        @orbit_counts[ORBIT_INDEX.fetch(orbit_size)] += 1
      end
      return
    end

    remaining = @queen_count - depth
    last = @size - remaining
    first = depth.zero? ? nil : @chosen[0]
    pair_base = depth == 1 ? @chosen[0] * @size : nil
    not_attacked = ~attacked
    square = start_square
    while square <= last
      # In the lexicographically least orientation, the first queen cannot be
      # beaten by any image of any queen.  This rejects only noncanonical
      # orientations; canonical_orbit_size still resolves boundary ties.
      if @minimum_image[square] < (first || square)
        square += 1
        next
      end
      if pair_base && !@canonical_pair[pair_base + square]
        square += 1
        next
      end
      if depth == 2 && !canonical_triple?(@chosen[0], @chosen[1], square)
        square += 1
        next
      end

      new_bits = @attack_masks[square] & not_attacked
      budget = @best_attacked - attacked_count
      if budget.zero?
        unless new_bits.zero?
          square += 1
          next
        end
        added = 0
      else
        added = popcount_up_to(new_bits, budget)
      end
      if added <= budget
        @chosen[depth] = square
        search(square + 1, depth + 1, attacked | new_bits, attacked_count + added)
      end
      square += 1
    end
  end

  def canonical_triple?(first, second, third)
    @transforms.all? do |transform|
      a = transform[first]
      b = transform[second]
      c = transform[third]
      a, b = b, a if a > b
      b, c = c, b if b > c
      a, b = b, a if a > b

      first < a ||
        (first == a && (second < b || (second == b && third <= c)))
    end
  end

  def verify_orbit_counts!
    orbit_total = @orbit_counts.sum
    weighted_total = @orbit_counts.each_with_index.sum do |count, i|
      count * ORBIT_SIZES[i]
    end
    return if orbit_total == @solutions && weighted_total == @all_solutions

    raise "internal orbit-count verification failed"
  end

  # One representative per orbit under D4 (four rotations and four reflections).
  def canonical_orbit_size
    original = @chosen
    stabilizer_size = 0

    @transforms.each do |transform|
      comparison = original <=> original.map { |square| transform[square] }.sort
      return nil if comparison.positive?

      stabilizer_size += 1 if comparison.zero?
    end

    8 / stabilizer_size
  end

  # Returns as soon as it proves that value contains more than limit set bits.
  def popcount_up_to(value, limit)
    count = 0
    until value.zero?
      count += POPCOUNT_16[value & 0xffff]
      return limit + 1 if count > limit

      value >>= 16
    end
    count
  end
end

def usage
  warn "Usage: ruby #{File.basename($PROGRAM_NAME)} N"
  warn "       ruby #{File.basename($PROGRAM_NAME)} --upto N"
  exit 1
end

arguments = ARGV.dup
ns = if arguments.length == 1 && arguments[0].match?(/\A[1-9]\d*\z/)
       [Integer(arguments[0])]
     elsif arguments.length == 2 && arguments[0] == "--upto" && arguments[1].match?(/\A[1-9]\d*\z/)
       1..Integer(arguments[1])
     else
       usage
     end

all_verified = true
ns.each do |n|
  result = NonDominatingQueens.new(n).solve
  expected_free = KNOWN_FREE[n]
  expected = KNOWN_A019317[n]
  free_matches = expected_free.nil? || result.free == expected_free
  a019317_matches = expected.nil? || result.solutions == expected
  matches = free_matches && a019317_matches
  all_verified &&= matches
  status = if expected.nil?
             ""
           elsif matches
             " OK"
           else
             " expected={A001366:#{expected_free},A019317:#{expected}}"
           end
  puts "n=#{n} A001366(n)=#{result.free} A019317(n)=#{result.solutions}" \
       " orbits={1:#{result.orbit_counts[0]},2:#{result.orbit_counts[1]}," \
       "4:#{result.orbit_counts[2]},8:#{result.orbit_counts[3]}}" \
       " all=#{result.all_solutions} nodes=#{result.nodes}#{status}"
end

exit 2 unless all_verified
