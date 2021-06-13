def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << ncr(i * j, i)
    }
  }
  ary.flatten
end

n = 140
ary = A(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
