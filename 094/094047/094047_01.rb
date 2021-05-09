def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = [1, 0, 0, 2]
  i = 3
  while i < n
    i += 1
    ary << (i - 1) * (i * (ary[-1] + ary[-2]) - 4 * (-1) ** i * f(i - 3))
  end
  ary[0..n]
end

n = 500
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
