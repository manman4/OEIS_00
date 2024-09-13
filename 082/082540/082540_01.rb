# a(n) = n^k - Sum_{j=2..n} a(floor(n/j)).
def A(n, k)
  ary = [0]
  (1..n).each{|i|
    ary << i ** k - (2..i).inject(0){|s, j| s + ary[i / j]}
  }
  ary
end

n = 10000
ary = A(n, 4)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
