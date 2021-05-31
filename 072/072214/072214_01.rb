l = 10 ** 5
i, j = 0, 1
ary = [i, j]
while ary.max < l
  i, j = j, i + j
  ary << j
end
m = 25
n = ary[m]

@part = [1, 1]
def part(i)
  sum = 0
  (0..i).each{|j|
    i1 = (1 - 2 * (j % 2)) * ((j + 2) / 2) # 1, -1, 2, -2, 3, -3, ...
    i1 = i1 * (3 * i1 - 1) / 2             # 1, 2, 5, 7, 12, 15, ...
    break if i1 > i
    if (j / 2).even?
      sum += @part[i - i1]
    else
      sum -= @part[i - i1]
    end
  }
  @part[i] = sum
end
(1..n).each{|i| part(i)}

(1..m).each{|i|
  j = @part[ary[i]]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
