def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, m, n)
  ary = [1]
  x = f(m)
  (1..n).each{|i|
    y = f(i) / x
    ary << (k + 1..i).inject(0){|s, j| s + ary[-j] * y / (f(i - j) * (j - k).to_r)}.to_i
  }
  ary
end

n = 500
ary = A(1, 1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}