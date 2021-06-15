# 10^d - 1‚Ü‚Å
def A(n, d)
  ary = Array.new(10 ** d, -1)
  cnt = 0
  num = 0
  while cnt < 10 ** d - 1
    f = num ** n
    str = f.to_s
    (0..d - 1).each{|i|
      j = str[0..i].to_i
      if ary[j] == -1
        ary[j] = num
        cnt += 1
      end
    }
    num += 1
  end
  ary
end

d = 5
ary = A(8, d)
(0..10 ** (d - 1)).each{|i|
  j = ary[i] ** 8
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
