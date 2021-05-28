ary = []
(1..500).each{|i|
  i = i.to_s.split('').map(&:to_i)
  cnt = 1
  while cnt < i.size
    break if i[0..cnt].join.to_i % (cnt + 1) > 0
    cnt += 1
  end
  if cnt == i.size
    while true
      j = 0
      while (i << j).join.to_i % (cnt + 1) > 0 && j < 10
        i.pop
        j += 1
      end
      if j == 10
        i.pop
        break
      end
      cnt += 1
    end
  end
  ary << i.join.to_i
}
p ary
