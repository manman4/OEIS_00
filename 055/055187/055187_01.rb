def A(n)
  ary = [1]
  (1..n).each{|i|
    ary += ary.uniq.sort.map{|i| [ary.count(i), i]}
    ary.flatten!
  }
  ary
end

n = 100
ary = A(n)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}