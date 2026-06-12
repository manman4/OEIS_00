# E.g.f. B(x) satisfies B'(x) = 1 + B^l(x),
# where B^l(x) denotes the l-th iterate of B.
#
# a(n, k, l) = n! * [x^n] B^k(x)
# a(n, 0, l) = 0^(n - 1)
# a(n, k, l) = a(n, k - 1, l) + sum(j=1..n-1,
#   binomial(n - 1, j) * a(j, k + l - 1, l) * a(n - j, k - 1, l)
# ) for k > 0.
#
# This script computes a(n, k, l) by dynamic programming.

def binomial_rows(n_max)
  rows = Array.new(n_max + 1) { [] }
  rows[0] = [1]

  n = 1
  while n <= n_max
    row = Array.new(n + 1, 1)
    prev = rows[n - 1]
    j = 1
    while j < n
      row[j] = prev[j - 1] + prev[j]
      j += 1
    end
    rows[n] = row
    n += 1
  end

  rows
end

def build_table(n_max, k_max, l)
  k_limits = Array.new(n_max + 1, 0)
  n = 1
  while n <= n_max
    k_limits[n] = k_max + (n_max - n) * l
    n += 1
  end

  binoms = binomial_rows(n_max)
  a = Array.new(n_max + 1) { |i| Array.new(k_limits[i] + 1, 0) }

  k = 0
  while k <= k_limits[1]
    a[1][k] = 1
    k += 1
  end

  n = 2
  while n <= n_max
    a[n][0] = 0
    row_binoms = binoms[n - 1]
    k = 1
    while k <= k_limits[n]
      total = a[n][k - 1]
      j = 1
      while j < n
        total += row_binoms[j] * a[j][k + l - 1] * a[n - j][k - 1]
        j += 1
      end
      a[n][k] = total
      k += 1
    end
    n += 1
  end

  a
end

def a_number(n, k, l)
  build_table(n, k, l)[n][k]
end

def a_vector(n_max, k = 1, l = 1)
  table = build_table(n_max, k, l)
  Array.new(n_max) { |n| table[n + 1][k] }
end

if __FILE__ == $PROGRAM_NAME
  n_max = 320
  k = 1
  l = 2

  ary = a_vector(n_max, k, l)
  i = 1
  while i <= n_max
    j = ary[i - 1]
    break if j.to_s.size > 1000
    print i
    print ' '
    puts j
    i += 1
  end
end
