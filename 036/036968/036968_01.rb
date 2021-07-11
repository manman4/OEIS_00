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

def A036968(n)
  ary = bernoulli(n)
  [1] + (2..n).map{|i| (2 * (1 - 2 ** i) * ary[i]).to_i}
end

n = 600
ary = A036968(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
