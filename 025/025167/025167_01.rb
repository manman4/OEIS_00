def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A025167(n)
  (0..n).inject(0){|s, i| s + ncr(n, i) ** 2 * 2 ** i * f(i)}
end

n = 1000
(0..n).each{|i|
  j = A025167(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}