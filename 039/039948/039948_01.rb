def A(n)
  ary = [0]
  a, b = 0, 1
  (1..n).each{|i|
    a, b = b, a + b
    ary << a
  }
  ary
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def B(n)
  a = A(n + 1)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << f(i) / f(j) * a[i - j + 1]
    }
  }
  ary
end

n = 139
ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
