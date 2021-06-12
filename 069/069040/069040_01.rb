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
  a = bernoulli(2 * n)
  (1..n).map{|i| a[2 * i].numerator % i}
end

def B(n)
  a = A(n)
  (1..n).select{|i| a[i - 1] == 0}
end

n = 950
ary = B(n)
(1..300).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
