def A(k, n)
  a = Array.new(k, 1)
  s = 0
  ary = [1]
  while s < n
    s += 1
    a = *a[1..-1], a[-1] + (s..s + k - 2).inject(:*) * a[0]
    ary << a[0]
  end
  ary
end

n = 1000
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
