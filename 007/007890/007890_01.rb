def A(n)
  n.to_s.split("").group_by(&:to_s).to_a.sort.reverse.map{|i| [i[1].size, i[0]]}.join.to_i 
end

p (1..100).map{|i| A(i)}

i = 1
b = [1]
100.times{
  i = A(i)
  b << i
}
p b