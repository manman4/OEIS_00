def A(n)
  ary = []
  i = 0
  while ary.size < n
    ary << i if i.to_s.split('').sort.join.to_i == i
    i += 1
  end
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
