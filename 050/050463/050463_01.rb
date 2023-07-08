def A(k, n)
  ary = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      if i % j == 0
        s += j ** k if (i / j) % 4 == 1
      end
    }
    ary << s
  }
  ary
end

n = 10000
ary = A(4, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
