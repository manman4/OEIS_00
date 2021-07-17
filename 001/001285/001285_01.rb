def A(n)
  ary = [1]
  (1..n).each{|i|
    if i % 2 == 0
      ary << ary[i / 2]
    else
      ary << 3 - ary[i / 2]
    end
  }
  ary
end

n = 10010
ary = A(n)
(0..10000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
