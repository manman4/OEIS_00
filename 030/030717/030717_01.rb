def A(n)
  ary = [1]
  n.times{
    ary += ary.uniq.sort.map{|i| ary.count(i)}
  }
  ary
end

n = 100
# sortがないとa(114)で微妙に異なる値のものがでてくる
ary = A(n)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}