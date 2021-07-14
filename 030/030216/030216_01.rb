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

def A(k, n)
  ary = A010815(n)
  aryk = Array.new(n + 1, 0)
  (0..n / k).each{|i| aryk[k * i] = ary[i]}
  l = 24 - k
  aryl = Array.new(n + 1, 0)
  (0..n / l).each{|i| aryl[l * i] = ary[i]}
  [0] + mul(aryk, aryl, n)[0..-2]
end

n = 100
ary = A(10, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
