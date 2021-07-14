def s(n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i * i * (i + 1) if n % i == 0}
  s
end

def A(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i|
    ary << (1..i).inject(0){|s, j| s + s_ary[j] * ary[-j]} / (2 * i)
  }
  ary
end

n = 100
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
