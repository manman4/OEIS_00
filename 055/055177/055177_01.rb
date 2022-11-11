def A(k, n)
  ary = [k]
  (1..n).each{|i|
    ary += ary.uniq.map{|i| [i, ary.count(i)]}
    ary.flatten!
  }
  ary
end

n = 100
ary = A(3, n)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}