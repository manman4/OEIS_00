def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << 2 * (0..(i - 1) / 4).inject(0){|s, j| s + ncr(i - 1, 4 * j) * ary[-4 * j - 1]}
  }
  ary
end

n = 24
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}