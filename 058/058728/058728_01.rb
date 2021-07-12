def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, i) + s(6, i) + s(10, i) + s(30, i) - s(3, i) - s(4, i) - s(5, i) - s(60, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 84
ary = A(n + 1)
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
