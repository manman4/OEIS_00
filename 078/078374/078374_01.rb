require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end



def B(ary, n)
  s = 0
  (1..n).each{|i|
    if n % i == 0
      s += A008683(i) * ary[n / i]
    end
  }
  s
end

m = 56
n = m + 100
a = Array.new(n + 1, -1)
ary = A(a, a, n)
(1..m).each{|i|
  j = B(ary, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
