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

def A083007(n)
  ary = bernoulli(n - 1)
  ary[1] = -1r / 2
  (0..n).inject([]){|s, i|
    s << (0..i - 1).inject(0){|t, j| t + 3 ** j * ary[j] * c(i, j)}.numerator
  }
end

def A002111(n)
  ary = A083007(2 * n + 1)
  (1..n).map{|i| (-1) ** (i % 2 + 1) * ary[2 * i + 1]}
end

n = 16
ary = A002111(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
