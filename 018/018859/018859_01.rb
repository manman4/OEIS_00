# 10^d - 1‚Ü‚Å
def A(n, d)
  ary = Array.new(10 ** d, -1)
  ary[1] = 1
  cnt = 1
  f = 1
  while cnt < 10 ** d - 1
    f *= n
    str = f.to_s
    (0..d - 1).each{|i|
      j = str[0..i].to_i
      if ary[j] == -1
        ary[j] = f
        cnt += 1
      end
    }
  end
  ary
end

d = 3
ary = A(4, d)
(1..10 ** d - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
