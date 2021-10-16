def A(k, n)
  i = (n * n).to_s(k)
  i == i.reverse
end

m = 0
cnt = 0
while cnt < 92
  if A(6, m)
    cnt += 1
    print cnt
    print ' '
    puts m
  end
  m += 1
end
