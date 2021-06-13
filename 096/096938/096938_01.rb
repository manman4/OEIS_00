# mŸˆÈ‰º‚ğæ‚èo‚·
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

def f(ary, k, n)
  aryk = Array.new(n + 1, 0)
  (0..n / k).each{|i| aryk[k * i] = ary[i]}
  aryk
end

def A(k, n)
  ary = A010815(n)
  ary2 = f(ary, 2, n)
  aryk = f(ary, k, n)
  ps = Array.new(n + 1, 0)
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps2k = f(ps, 2 * k, n)
  m_ary1 = mul(ary2, aryk, n)
  m_ary1 = mul(m_ary1, ps, n)
  mul(m_ary1, ps2k, n)
end

n = 1000
ary = A(5, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
