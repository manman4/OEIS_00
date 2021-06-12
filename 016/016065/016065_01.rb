def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 500
s = 0
(0..n).each{|i|
  s += f(i) * f(i + 1)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
