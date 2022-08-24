def A(n)
  ary = [-1]
  (1..n).each{|i|
    if i % 2 == 0
      ary << ary[i / 2] + i / 2
    else
      ary << 2 * ary[(i - 1) / 2]
    end
  }
  ary
end

n = 2 ** 13 - 1
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

