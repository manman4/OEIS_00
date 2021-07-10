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

# n‚Í-1‚©‚ç
def A(n)
  return [-1] if n == 1
  a = bernoulli(2 * n + 2)
  [1] + (1..n + 1).map{|i| (-1) ** (i % 2) * 2 ** (2 * i) * a[2 * i] / (1..2 * i).inject(:*)}
end

n = 400
ary = A(n).map{|i| i.numerator}
-1.upto(n){|i|
  j = ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
