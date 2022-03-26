def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << k * (0..(i - 1) / 2).inject(0){|s, j| s + (-1) ** j * (2 * j + 1) * ncr(i - 1, 2 * j) * ary[-2 * j - 1]}
  }
  ary
end

n = 600
ary = A(1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}