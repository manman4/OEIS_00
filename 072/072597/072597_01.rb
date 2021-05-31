def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def f(n)
  return 1 if n < 1
  (1..n).inject(:*)
end
def A072597(n)
  f(n) * (0..n).inject(0){|s, i| s + (n - i + 1r) ** i / f(i)}
end

n = 30
(0..n).each{|i|
  j = A072597(i).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
