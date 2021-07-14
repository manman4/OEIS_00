def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 24 / (k - 1) * a[j] * ary[-j]} / i}
  ary
end

def B(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 24 / (k - 1) * a[j] * ary[-j]} / i}
  ary
end

n = 23
a_ary = A(7, n + 1)
b_ary = [0, 0] + B(7, n - 1)
-1.upto(n){|i|
  print i
  print ' '
  puts a_ary[i + 1] + 49 * b_ary[i + 1] if i != 0
  puts a_ary[i + 1] + 49 * b_ary[i + 1] + 14 if i == 0
}
