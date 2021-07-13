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

def A(k, n)
  ary = [1] + Array.new(n, 0)
  i = 1
  while i <= n
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, i
    ary = mul(ary, b_ary, n)
    i *= k
  end
  ary
end

n = 1000
ary0 = A(2, 2 * n)
ary = (0..n).map{|i| ary0[2 * i]}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
