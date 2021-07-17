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

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % 2 == 1}
  s
end

def A(k, n)
  ary = [1] + (1..n).map{|i| 24 * s(i)}
  power(ary, k, n)
end

n = 10000
ary = A(6, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
