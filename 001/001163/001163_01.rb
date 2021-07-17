def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(n)
  a = bernoulli(n + 1)
  ary = [1]
  (1..n).each{|i|
    ary << 1r / i * (0..i - 1).inject(0){|s, j| s + a[i - j + 1] * ary[j] / (i - j + 1r)}
  }
  ary
end

n = 16
ary = A(n).map{|i| i.numerator}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
