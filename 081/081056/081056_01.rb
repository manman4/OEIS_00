def A(k, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && i % k != 0
  }
  s
end

def B(k, n)
  (1..n).map{|i| A(k, i)}
end

def C(k, n)
  ary = [1]
  a = [0] + B(k, n)
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 37
ary = C(4, 2 * n + 1)
b=[]
(0..n).each{|i|
  print i
  print ' '
  puts ary[2 * i + 1]
b<<ary[2*i+1]
}
p b
