def step_states(states, active_masks, valid_bits, max_degree, state_count)
  next_states = Array.new(state_count)
  next_active_masks = []
  seen = Array.new(state_count, false)

  active_masks.each{|mask|
    poly = states[mask]

    shifted = mask >> 1
    target = next_states[shifted]
    if target.nil?
      target = next_states[shifted] = Array.new(max_degree + 1, 0)
      unless seen[shifted]
        seen[shifted] = true
        next_active_masks << shifted
      end
    end
    0.upto(max_degree){|r|
      target[r] += poly[r]
    }

    valid_bits.each{|bit|
      next if ((mask >> bit) & 1) != 0

      shifted_with_rook = (mask | (1 << bit)) >> 1
      target_with_rook = next_states[shifted_with_rook]
      if target_with_rook.nil?
        target_with_rook = next_states[shifted_with_rook] = Array.new(max_degree + 1, 0)
        unless seen[shifted_with_rook]
          seen[shifted_with_rook] = true
          next_active_masks << shifted_with_rook
        end
      end
      0.upto(max_degree - 1){|r|
        target_with_rook[r + 1] += poly[r]
      }
    }
  }

  [next_states, next_active_masks]
end

def evaluate_states(states, n, factorials)
  total = 0
  0.upto(n) do |r|
    rook_count = 0
    states.each do |poly|
      rook_count += poly[r] unless poly.nil?
    end
    term = rook_count * factorials[n - r]
    total += r.even? ? term : -term
  end
  total
end

def direct_count(n, k, factorials)
  return 0 if k <= 0
  width = 2 * k - 1
  state_count = 1 << width
  states = Array.new(state_count)
  states[0] = Array.new(n + 1, 0)
  states[0][0] = 1
  active_masks = [0]

  1.upto(n){|row|
    lo = [-(k - 1), 1 - row].max
    hi = [k - 1, n - row].min
    valid_bits = (lo..hi).map{|offset| offset + k - 1}
    states, active_masks = step_states(states, active_masks, valid_bits, n, state_count)
  }

  evaluate_states(states, n, factorials)
end

def derangements(limit)
  ary = Array.new(limit + 1, 0)
  ary[0] = 1
  return ary if limit == 0

  ary[1] = 0
  2.upto(limit){|n|
    ary[n] = (n - 1) * (ary[n - 1] + ary[n - 2])
  }
  ary
end

def sequence(limit, k)
  raise ArgumentError, "limit must be nonnegative" if limit < 0
  raise ArgumentError, "k must be positive" if k <= 0
  return derangements(limit) if k == 1

  factorials = Array.new(limit + 1, 1)
  1.upto(limit){|n|
    factorials[n] = factorials[n - 1] * n
  }

  result = []
  cutoff = [limit, 2 * k - 3].min
  0.upto(cutoff){|n|
    result << direct_count(n, k, factorials)
  }
  return result if limit <= 2 * k - 3

  width = 2 * k - 1
  state_count = 1 << width
  max_degree = limit

  states = Array.new(state_count)
  states[0] = Array.new(max_degree + 1, 0)
  states[0][0] = 1
  active_masks = [0]

  1.upto(k - 1){|row|
    valid_bits = ((1 - row)..(k - 1)).map{|offset| offset + k - 1}
    states, active_masks = step_states(states, active_masks, valid_bits, max_degree, state_count)
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
      tail_states, tail_active_masks = step_states(tail_states, tail_active_masks, valid_bits, max_degree, state_count)
    }
    result << evaluate_states(tail_states, n, factorials)

    n += 1
    break if n > limit

    states, active_masks = step_states(states, active_masks, full_valid_bits, max_degree, state_count)
  end

  result
end

def count(n, k)
  sequence(n, k)[n]
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 30).to_i
  k = (ARGV[1] || 4).to_i
  p ary = sequence(limit, k)
  (0..limit).each{|n|
    i = ary[n]
    break if i.to_s.size > 1000
    print n
    print " "
    puts i
  }
end
