def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A000670(n)
  ary = [1]
  (1..n).each{|i|
    ary << (1..i).inject(0){|s, j| s + ncr(i, j) * ary[-j]}
  }
  ary
end

def A052862(n)
  ary = A000670(n)
  [0, 0, 2] + (3..n).map{|i| 2 * i * ary[i - 2]}
end

n = 500
ary = A052862(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}