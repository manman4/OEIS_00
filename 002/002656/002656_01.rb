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

def p0(n)
  (3 * n * n - n) / 2
end

def p1(n)
  (3 * n * n + n) / 2
end

def A010815(n)
  ary = Array.new(n + 1, 0)
  ary[0] = 1
  i = 1
  j = p0(i)
  while j <= n
    ary[j] = (-1) ** i
    i += 1
    j = p0(i)
  end
  i = 1
  j = p1(i)
  while j <= n
    ary[j] = (-1) ** i
    i += 1
    j = p1(i)
  end
  ary
end

def A002656(n)
  ary = A010815(n)
  ary7 = Array.new(n + 1, 0)
  (0..n / 7).each{|i| ary7[7 * i] = ary[i]}
  b = mul(ary, ary7, n)
  b2 = mul(b, b, n)
  mul(b, b2, n)[0..-2]
end

n = 10000
ary = A002656(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
