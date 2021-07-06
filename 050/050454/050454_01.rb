def A(k, n)
  ary = []
  (1..n).each{|i|
    s = 0
    3.step(i, 4){|j|
      s += j ** k if i % j == 0
    }
    ary << s
  }
  ary
end

n = 10000
ary = A(3, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
