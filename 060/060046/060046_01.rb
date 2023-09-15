def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# m次以下を取り出す
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

# m次以下を取り出す
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def I(ary, n)
  a = [1]
  i = 0
  while i < n
    a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}
    i += 1
  end
  a
end

def A(n)
  a = [1] + [0] * n
  (1..n).each{|i|
    j = i * i
    break if j > n
    a[j] = 2 * (-1) ** (j % 2)
  }
  # Sum_{k>=3} (-1)^k * k * ncr(k+2,5) * q^(k^2)
  b = Array.new(n + 1, 0)
  (3..n).each{|i|
    j = i * i
    break if j > n
    b[j] = (-1) ** (i % 2) * i * ncr(i + 2, 5)
  }
  mul(I(a, n), b, n)
end

n = 11000
m = 10000
ary = A(n)
(9..m).each{|i|
  j = -ary[i] / 3
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}