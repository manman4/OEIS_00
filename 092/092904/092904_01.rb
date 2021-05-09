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
  ary = bernoulli(2 * n)
  (0..n).map{|i| ary[2 * i].denominator.to_s.size}
end

n = 1000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
