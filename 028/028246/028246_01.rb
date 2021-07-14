def A028246(n)
  a = [1]
  i = 1
  ary = [1]
  while ary.size < n
    a << 0
    b = [1] + (1..i).map{|j| (j + 1) * a[j] - j * a[j - 1]}
    a = b
    i += 1
    ary += a.map{|i| i.abs}
  end
  ary[0..n - 1]
end

n = 11000
ary = A028246(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
