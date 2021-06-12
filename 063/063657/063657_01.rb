def A(n)
  s = 0
  ary = []
  (1..n).each{|i|
    s += i
    (i - 1).times{
      ary << s
      s += 1
    }
  }
  ary
end

ary = A(150)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
