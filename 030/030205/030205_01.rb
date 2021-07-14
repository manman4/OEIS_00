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

def A010815(n)
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary = mul(ary, b_ary, n)
  }
  ary
end

def A030205(n)
  ary = A010815(n)
  ary5 = Array.new(n + 1, 0)
  (0..n / 5).each{|i| ary5[5 * i] = ary[i]}
  b = mul(ary, ary5, n)
  mul(b, b, n)
end

n = 1000
ary = A030205(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
