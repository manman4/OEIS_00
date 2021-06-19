def A070191(n)
  ary = []
  s = 1
  i = 1
  while ary.size < n
    s *= 8
    ary << i if (3 * i).gcd(s + 1) == 3
    i += 1
  end
  ary
end

n = 10000
ary = A070191(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
