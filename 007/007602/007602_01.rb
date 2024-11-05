def A(n)
  ary = n.to_s.split('').map(&:to_i)
  return false if ary.include?(0)
  n % ary.inject(:*) == 0
end

p (1..2000).select{|i| A(i)}


