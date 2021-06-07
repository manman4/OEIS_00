def f_ary(n)
  a, b = 0, 1
  ary = [a]
  while ary.size < n + 1
    ary << b
    a, b = b, b + a
  end
  ary
end

def A(n)
  a = f_ary(n + 2)
  [1] + (2..n).map{|i| a[i + 2]}
end

n = 5000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
