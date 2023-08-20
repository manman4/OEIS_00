def isok(n, k)
  a = n.to_s(k)
  b = (n * n).to_s(k)
  # おそらくb.size = a.size * 2 となりそうだが定義通りに計算
  (a.split('') * b.size).sort.join == (b.split('') * a.size).sort.join
end

cnt = 1
ary = []
(2..3483208).each{|i|
  if isok(i, 9)
    print cnt
    print ' '
    puts i
    ary << i
    cnt += 1
  end
}
p ary
