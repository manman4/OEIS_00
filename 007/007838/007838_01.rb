def s(i)
  s = 0
  (1..i).each{|j| s += (-j) ** (1 - i / j) if i % j == 0}
  s
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i|
    ary << f(i - 1) * (1..i).inject(0){|s, j| s + s_ary[j] * ary[-j] / f(i - j).to_r}
  }
  ary
end

n = 22
ary = A(1, n)
(0..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
