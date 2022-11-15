def A(n)
  a = []
  ary = [1]
  n.times{
    j = ary.uniq.sort
    a += j
    ary += j.map{|i| ary.count(i)}
  }
  a
end

n = 100
ary = A(n)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}