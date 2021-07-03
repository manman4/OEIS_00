def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(n)
  ary = [1, 1]
  (1..n - 1).each{|i| ary << (1..i).inject(0){|s, j| s + i ** (i - j) * ncr(i - 1, j - 1)}}
  ary[0..n]
end
   
n = 19
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
