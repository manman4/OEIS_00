def A(k, n)
  ary = (1..k).to_a
  ps = Array.new(n + 1, 0)
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end
def A058398(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i - 1).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end
n = 140
ary = A058398(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
