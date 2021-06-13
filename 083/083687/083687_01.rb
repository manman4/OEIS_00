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
def H(n)
  ary = [0]
  s = 0
  n.times{|i|
    s += 1r / (i + 1)
    ary << s
  }
  ary
end

n = 16
b_ary = bernoulli(2 * n)
h_ary = H(2 * n)
(1..n).each{|i|
  j = (b_ary[2 * i] * h_ary[2 * i] / i * (-1) ** ((i + 1) % 2)).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
