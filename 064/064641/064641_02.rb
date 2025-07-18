# a(n) = (-1)^n * (n+1) + Sum_{k=0..n-1} (a(k) + (-1)^k) * (a(n-1-k) + (-1)^(n-1-k)).
def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << (0..i-1).inject((-1)**i * (i + 1)){|s, k| s + (ary[k] + (-1)**k) * (ary[i - 1 - k] + (-1)**(i - 1 - k))}
  }
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}