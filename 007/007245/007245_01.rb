def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 8 * a[j] * ary[-j]} / i}
  ary
end

def B(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 16 * a[j] * ary[-j]} / i}
  ary
end

n = 20
a_ary = A(n)
b_ary = [0] + B(n - 1)
(0..n).each{|i|
  print i
  print ' '
  puts a_ary[i] + 256 * b_ary[i]
}
