def A(k, n)
  ary = [k]
  (1..n).each{|i|
    ary += ary.uniq.sort.map{|i| ary.count(i)}
    ary.flatten!
  }
  ary
end

n = 100
# sortがないとa(100)くらいで微妙に異なる値のものがでてくる
ary = A(1, n)
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}