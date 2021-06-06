def A(k, n)
  a = Array.new(k, 1)
  ary = [1]
  while ary.size < n
    j = (1..k - 2).inject(1){|s, i| s + a[- i] * a[- i - 1]}
    break if j % a[0] > 0
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

n = 17
ary = A(4, n)
(1..n).each{|i|
   j = ary[i - 1]
   break if j.to_s.size > 1000
   print i
   print ' '
   puts j
}
