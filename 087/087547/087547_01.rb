def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A087547(n)
  i = 0
  a = 0
  ary = [0]
  while i < n
    i += 1
    a = (2 * i - 1) * a + f(i - 1)
    ary << a 
  end
  ary
end

n = 500
ary = A087547(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
