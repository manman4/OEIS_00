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

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % 2 == 1}
  s
end

def A(n)
  ary = [1] + (1..n).map{|i| 24 * s(i)}
  mul(ary, ary, n)
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
