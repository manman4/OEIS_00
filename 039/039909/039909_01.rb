def mul(f_ary, b_ary)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary
end

def A(n)
  a = [1]
  b = [1]
  ary = []
  (0..n).each{|i|
    a, b = b, mul(b, (0..i + 1).map{|j| (-1) ** (j % 2)})
    ary << a.max
  }
  ary
end

n = 210
m = 200
ary = A(n)
# p ary.size
# p ary[-1]
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}