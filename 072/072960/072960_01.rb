n = 1000
i = 0
cnt = 1
puts '1 0'
while cnt < n
  i += 1
  if j = i.to_s.split('').map(&:to_i).uniq - [0, 3, 6, 8, 9] == []
    cnt += 1
    print cnt
    print ' '
    puts i
  end
end
