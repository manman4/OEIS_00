def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    ary << ary[i - 1] + ary[i - 2] * (i - 1) ** 2
  }
  ary
end

def B(n)
  ary = A(n)
  a = [0]
  (1..n).each{|i|
    a << (1..i).inject(0){|s, j| s + ncr(i, j) * ary[j]}
  }
  a
end

n = 500
ary = B(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
