def A(k, n)
  ary = [1]
  i = 0
  while i < n
    ary << (k..i - 1).inject(ary[-1]){|s, j| s + ary[j] * ary[i - 1 - j]}
    i += 1
  end
  ary
end

def B(n)
  a = (0..n).inject([]){|s, i| s << A(i, n - i)}
  (0..n).map{|i| (0..i).map{|j| a[j][i - j]}}.flatten!
end

n = 13
ary = B(n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
