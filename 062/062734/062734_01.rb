def ncr(n, r)
  return 0 if n < r
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# mŽŸˆÈ‰º‚ðŽæ‚èo‚·
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

# mŽŸˆÈ‰º‚ðŽæ‚èo‚·
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A(n)
  ary = [[1]]
  (2..n).each{|i|
    j = ncr(i, 2)
    a = Array.new(j + 1, 0)
    (1..i - 1).each{|k|
      b = mul(ary[k - 1], power([1, 1] + [0] * (j - 1), ncr(i - k, 2), j), j)
      (0..j).each{|m|
        a[m] += k * ncr(i, k) * b[m] if !b[m].nil?
      }
    }
    ary0 = []
    (0..j).each{|k|
      m = ncr(j, k)
      m -= a[k] / i if !a[k].nil?
      ary0 << m
    }
    ary << ary0
  }
  ary
end

n = 39
ary = A(n).flatten
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
