def A(n, k)
  s = 1
  ary = [1]
  (1..n).each{|i|
    s *= k
    ary << s.to_s.split('').map(&:to_i)
  }
  ary.flatten
end

n = 100
ary = A(n, 7)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}