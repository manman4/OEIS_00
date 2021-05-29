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

def A(k, n)
  a = bernoulli(2 * n)
  ary = []
  (0..n).each{|i|
    j = a[2 * i].numerator
    ary << i if j % k == 0
  }
  ary
end

n = 1300
ary = A(37, n)
(1..100).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
