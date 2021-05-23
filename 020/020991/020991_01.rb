n = 2 ** 27
@ary = [1]
s = 1
s_ary = [[-1], [0]]
(1..n).each{|i|
  j = @ary[i / 2]
  if i % 2 == 0
    @ary << j
    s += j
  else
    k = (-1) ** (i / 2) * j
    @ary << k
    s += k
  end
  if s_ary[s] == nil
    s_ary[s] = [i]
  else
    s_ary[s] << i
  end
}
m = s_ary[1..-1].size
ary = []
(1..m - 1).each{|i|
  j = s_ary[i][i - 1]
  ary << j if j != nil
}
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
