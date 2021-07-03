def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A000670(n)
  ary = [1]
  (1..n).each{|i|
    a = (0..i).map{|j| ncr(i, j)}
    ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]}
  }
  ary
end

def A000629(n)
  a = A000670(n)
  (0..n).map{|i| 2 * a[i] - 0 ** i}
end

n = 19
ary = A000629(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
