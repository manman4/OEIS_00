def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def g(k, m)
  f(k * m) / (f(k) ** m * f(m))
end

def A(k, n)
  (1..n).map{|i| g(k, i)}
end

def A060540(n)
  a = []
  (1..n).each{|i| a << A(i, n - i + 1)}
  ary = []
  (0..n - 1).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 9
ary = A060540(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
