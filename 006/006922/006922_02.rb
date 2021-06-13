def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end
def A006922(n)
  ary = [1]
  a = [0] + (1..n + 1).map{|i| s(i)}
  (1..n + 1).each{|i| ary << (1..i).inject(0){|s, j| s + 24 * a[j] * ary[-j]} / i}
  ary
end

n = 21
ary = A006922(n)
(-1).upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
