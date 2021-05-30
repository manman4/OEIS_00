def A025243(n)
  i = 3
  ary = [1, 2, 1]
  while i < n
    i += 1
    ary << (1..i - 3).inject(0){|s, i| s + ary[i - 1] * ary[-i]}
  end
  ary[0..n - 1]
end

n = 25
ary = A025243(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
