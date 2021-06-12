def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** k if n % i == 0}
  s
end
def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 54
ary = A(2, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
