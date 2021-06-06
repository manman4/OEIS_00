def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
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

def A057867(n)
  ary = bernoulli(4 * n)
  (1..n).inject([]){|s, i|
    j = 4 * i - 1
    s << ((0..2 * i).inject(0){|t, k| t + (-1) ** (k + 1) * ary[2 * k] * ary[j + 1 - 2 * k] / (f(2 * k) * f(j + 1 - 2 * k) + 0r)} * 2 ** (j - 1)).numerator
  }
end

n = 12
ary = A057867(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
