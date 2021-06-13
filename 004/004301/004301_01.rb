# k > 0
def A(k, n)
  a = [1]
  ary = []
  (1..n).each{|i|
    a += [0]
    b = [1] + (1..i).map{|j| (2 * i - 1 - j) * a[j - 1] + (j + 1) * a[j]}
    a = b
    ary << a[k] if i >= k
  }
  ary
end

k = 2
n = 1000
ary = A(k, n)
(k..n).each{|i|
  j = ary[i - k]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
