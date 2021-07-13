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
def power(ary, n, m)
  if n == 0
    a = Array.new(m + 1, 0)
    a[0] = 1
    return a
  end
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end
def A(k, n)
  ary = Array.new(n + 1, 0)
  (0..Math.sqrt(n).to_i).each{|i| ary[i * i] = 1}
  power(ary, k, n)
end
p A(12, 100)
