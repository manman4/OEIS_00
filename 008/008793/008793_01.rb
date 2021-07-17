def A(n)
  (0..n - 1).inject(1){|s, i| s *= (i + n + 1..i + 2 * n).inject(:*) / (i + 1..i + n).inject(:*).to_r}.to_i
end

n = 70
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
