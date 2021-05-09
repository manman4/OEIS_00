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

def A089802(n)
  ary = A010815(n)
  ary6 = Array.new(n + 1, 0)
  (0..n / 6).each{|i| ary6[6 * i] = ary[i]}
  ary5 = [1]
  5.step(n, 6){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary5 = mul(ary5, b_ary, n)
  }
  ary1 = [1]
  1.step(n, 6){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary1 = mul(ary1, b_ary, n)
  }
  ary6 = mul(ary6, ary5, n)
  mul(ary6, ary1, n)
end

def A089801(n)
  ary = A089802(n)
  (0..n).map{|i| (-1) ** i * ary[i]}
end

n = 1000
ary = A089801(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
