def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  ary = [0]
  s = 0
  (1..n).each{|i|
    s += 1r / i ** k
    ary << (s * f(i) ** k).to_i
  }
  ary
end

n = 500
ary = A(1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
