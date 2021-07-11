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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 300
ary = bernoulli(2 * n)
(1..n).each{|i|
  j = ((2 ** (2 * i - 1) - 1) * ary[2 * i].abs / i.to_r).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
