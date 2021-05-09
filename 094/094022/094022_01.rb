def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, i) + s(30, i) - s(3, i) - s(5, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary = [0] + A(n)[0..-2]
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
