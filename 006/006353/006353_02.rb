def A000203(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end
def A006353(n)
  a = [0] + (1..n).map{|i| A000203(i)}
  ary = [1]
  (1..n).each{|i|
    ary[i] = 5 * a[i]
    ary[i] -=  2 * a[i / 2] if i % 2 == 0
    ary[i] +=  3 * a[i / 3] if i % 3 == 0
    ary[i] -= 30 * a[i / 6] if i % 6 == 0
  }
  ary
end
p A006353(100)

n = 53
ary = A006353(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
