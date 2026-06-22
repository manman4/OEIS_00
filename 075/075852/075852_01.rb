def forbidden_bits_for_row(row, n, k)
  lo = [-(k - 1), 1 - row].max
  hi = [k - 1, n - row].min
  (lo..hi).map{|offset| offset + k - 1}
end

def rook_step(states, active_masks, valid_bits, max_rooks, state_count)
  next_states = Array.new(state_count)
  next_active_masks = []
  seen = Array.new(state_count, false)

  active_masks.each{|mask|
    poly = states[mask]

    shifted = mask >> 1
    stay = next_states[shifted]
    if stay.nil?
      stay = next_states[shifted] = Array.new(max_rooks + 1, 0)
      unless seen[shifted]
        seen[shifted] = true
        next_active_masks << shifted
      end
    end
    0.upto(max_rooks){|j|
      stay[j] += poly[j]
    }

    valid_bits.each{|bit|
      next if ((mask >> bit) & 1) != 0

      shifted_with_rook = (mask | (1 << bit)) >> 1
      add_rook = next_states[shifted_with_rook]
      if add_rook.nil?
        add_rook = next_states[shifted_with_rook] = Array.new(max_rooks + 1, 0)
        unless seen[shifted_with_rook]
          seen[shifted_with_rook] = true
          next_active_masks << shifted_with_rook
        end
      end
      0.upto(max_rooks - 1){|j|
        add_rook[j + 1] += poly[j]
      }
    }
  }

  [next_states, next_active_masks]
end

def rook_polynomial(n, k)
  return [1] if n == 0
  raise ArgumentError, "k must be positive" if k <= 0

  width = 2 * k - 1
  state_count = 1 << width
  states = Array.new(state_count)
  states[0] = Array.new(n + 1, 0)
  states[0][0] = 1
  active_masks = [0]

  1.upto(n) do |row|
    valid_bits = forbidden_bits_for_row(row, n, k)
    states, active_masks = rook_step(states, active_masks, valid_bits, n, state_count)
  end

  rook = Array.new(n + 1, 0)
  states.each{|poly|
    next if poly.nil?
    0.upto(n){|j|
      rook[j] += poly[j]
    }
  }
  rook
end

def aggregate_rook(states, n)
  rook = Array.new(n + 1, 0)
  states.each{|poly|
    next if poly.nil?
    0.upto(n){|j|
      rook[j] += poly[j]
    }
  }
  rook
end

def factorials(limit)
  ary = Array.new(limit + 1, 1)
  1.upto(limit){|n|
    ary[n] = ary[n - 1] * n
  }
  ary
end

def count_from_rook(rook, facts)
  n = rook.length - 1
  total = 0
  0.upto(n){|j|
    term = rook[j] * facts[n - j]
    total += j.even? ? term : -term
  }
  total
end

def count(n, k, facts = nil)
  facts ||= factorials(n)
  count_from_rook(rook_polynomial(n, k), facts)
end

def sequence(limit, k)
  raise ArgumentError, "limit must be nonnegative" if limit < 0
  raise ArgumentError, "k must be positive" if k <= 0

  facts = factorials(limit)
  return [1] if limit == 0

  width = 2 * k - 1
  state_count = 1 << width
  result = []

  cutoff = [limit, 2 * k - 3].min
  0.upto(cutoff) do |n|
    result << count(n, k, facts)
  end
  return result if limit <= 2 * k - 3

  states = Array.new(state_count)
  states[0] = Array.new(limit + 1, 0)
  states[0][0] = 1
  active_masks = [0]

  1.upto(k - 1){|row|
    valid_bits = ((1 - row)..(k - 1)).map{|offset| offset + k - 1}
    states, active_masks = rook_step(states, active_masks, valid_bits, limit, state_count)
  }

  full_valid_bits = (0...width).to_a
  tail_valid_bits = []
  1.upto(k - 1){|t|
    tail_valid_bits << ((-(k - 1))..(k - 1 - t)).map{|offset| offset + k - 1}
  }

  n = 2 * k - 2
  while n <= limit
    tail_states = states
    tail_active_masks = active_masks
    tail_valid_bits.each{|valid_bits|
      tail_states, tail_active_masks = rook_step(tail_states, tail_active_masks, valid_bits, limit, state_count)
    }
    result << count_from_rook(aggregate_rook(tail_states, n), facts)

    n += 1
    break if n > limit

    states, active_masks = rook_step(states, active_masks, full_valid_bits, limit, state_count)
  end

  result
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 20).to_i
  k = (ARGV[1] || 4).to_i
  ary = sequence(limit, k)
  puts ary.join(", ")
  (0..limit).each{|n|
    i = ary[n]
    break if i.to_s.size > 1000
    print n
    print " "
    puts i
  }
end
