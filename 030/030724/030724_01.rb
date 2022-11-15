def A(n)
  ary = [1]
  n.times{
    ary += ary.uniq.sort.map{|i| ary.count(i)}
  }
  ary
end

n = 200
ary = A(n)
a = (1..ary.size).select{|i| ary[i - 1] == 2}
(1..10000).each{|i|
  print i
  print ' '
  puts a[i - 1]
}