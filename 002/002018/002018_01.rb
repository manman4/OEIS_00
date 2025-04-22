# a(n) = n^2*a(n-1) - (1/2)*n*(n-1)^2*a(n-2).
def A000681(n)
  a = [1, 1]
  (2..n).each{|i|
    a[i] = i ** 2 * a[i - 1] - i * (i - 1) ** 2 / 2 * a[i - 2]
  }
  a
end

# a(n) = n*(2*n-1)*b(n) - n*(n-1)^2*b(n-1), b(n) = A000681(n).
def A(n)
  b = A000681(n)
  a = [1, 1]
  (2..n).each{|i|
    a[i] = i * (2 * i - 1) * b[i - 1] - i * (i - 1) ** 2 * b[i - 2]
  }
  a
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}