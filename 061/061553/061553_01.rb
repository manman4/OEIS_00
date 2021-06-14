def A(n)
  ary = [1]
  s_ary = [1]
  (1..n).each{|i|
    a = ary.map{|i| -i} + [0] * i
    (0..(i - 1) * i / 2).each{|j| a[i + j] += ary[j]}
    ary = a
    s_ary << ary.inject(0){|s, j| s + j.abs}
  }
  s_ary
end

n = 46
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
