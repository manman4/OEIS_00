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

def A030184(n)
  ary = A010815(n)
  ary3 = Array.new(n + 1, 0)
  (0..n / 3).each{|i| ary3[3 * i] = ary[i]}
  ary5 = Array.new(n + 1, 0)
  (0..n / 5).each{|i| ary5[5 * i] = ary[i]}
  ary15 = Array.new(n + 1, 0)
  (0..n / 15).each{|i| ary15[15 * i] = ary[i]}
  ary = mul(ary, ary3, n)
  ary = mul(ary, ary5, n)
  mul(ary, ary15, n)[0..-2]
end

n = 10000
ary = A030184(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
