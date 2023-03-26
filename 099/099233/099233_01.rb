def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  (0..n).inject(0){|s, i| s + ncr(k * (n - i), i)}
end

def B(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << A(j, i - j)
    }
  }
  ary
end

n = 139
ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}