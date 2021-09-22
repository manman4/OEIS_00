def f(n)
  return 1 if n <= 1
  (1..n).inject(:*)
end

def A(n)
  s = 0
  h_ary = [0r]
  ary = [1]
  (1..n).each{|i|
    s += 1r / i
    h_ary << s
    ary << f(i - 1) * (0..i - 1).inject(0){|s, j| s + (i - j) * h_ary[i - j] * ary[j] / f(j)}
  }
  ary.map{|i| i.to_i}
end

n = 450
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}