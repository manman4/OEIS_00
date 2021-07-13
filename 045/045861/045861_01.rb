def A(k, n)
  str = k.to_s
  ary = []
  i = 0
  while ary.size < n
    i += 1
    ary << i if (i * i).to_s.split('').first == str
  end
  ary
end

n = 10000
ary = A(7, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
