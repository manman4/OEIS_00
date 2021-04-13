def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    s = 1
    (1..i - 1).each{|j|
      s *= ary[j] if (i - 1) % j == 0
    }
    ary << s + 1
  }
  ary
end

n = 100
p ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
