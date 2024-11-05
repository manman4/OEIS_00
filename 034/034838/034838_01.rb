def A(n)
  ary = n.to_s.split('').map(&:to_i).uniq
  return false if ary.include?(0)
  ary.all?{|i| n % i == 0}
end


p (1..500).select{|i| A(i)}


