def A(n)
  a = []
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << i * j
    }
    ary.uniq!
    a << ary.size
  }
  a
end

n = 10000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
