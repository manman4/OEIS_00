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
def A(n)
  a = bernoulli(2 * n)
  [1] + (1..n).map{|i| (-1) ** (i % 2 + 1) * (2 ** (2 * i) - 2) * a[2 * i] / (1..2 * i).inject(:*)}
end

n = 300
ary = A(n).map{|i| i.denominator}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
