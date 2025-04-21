filename = './b059751.txt'

ary = []
max= -1
File.readlines(filename).each{|line|
  i = line.strip.split(" ")[1]
  if i != nil
    j = i.to_i
    if j > max
      max = j
      ary << j
    end
  end
}
p ary

# [7, 10, 14, 15, 22, 23, 27, 29, 30, 31, 34, 41, 46, 47, 55, 60, 63, 68, 71, 75, 87, 90, 91, 103, 119, 139, 169, 185, 193, 203, 233, 239, 279, 287, 305, 309]
