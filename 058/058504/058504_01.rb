def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, i) + s(7, i) - s(1, i) - s(14, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 4 * a[j] * ary[-j]} / i}
  ary
end

n = 34
ary = A(n + 1)
ary[1] = 0
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
