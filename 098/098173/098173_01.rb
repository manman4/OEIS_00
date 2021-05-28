def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << ncr(i, k * (i - j))
    }
  }
  ary
end

n = 13
p ary = A(4, n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
