def c(n, r)
  return 1 if r == 0
  return c(n, n - r) if r > n - r
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0]
  }
  ary
end

def A(k, n)
  ary = bernoulli(n - 1)
  ary[1] = -1r / 2
  s = []
  (0..n).each{|i|
    s << (0..i - 1).inject(0){|t, j| t + k ** j * ary[j] * c(i, j)}.numerator
  }
  s
end

n = 19
ary = A(10, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
