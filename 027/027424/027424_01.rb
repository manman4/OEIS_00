def A(n)
  a = []
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << i * j
    }
    ary.uniq!
    a << ary.size
  }
  a
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
