def A(n)
  s = 0
  ary = []
  (1..n).each{|i|
    i.times{
      ary << s
      s += 1
    }
    s += i - 1
  }
  ary
end

ary = A(150)
(0..10000).each{|i|
  print i
  print ' '
  puts ary[i]
}
